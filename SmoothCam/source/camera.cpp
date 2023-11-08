#include "camera.h"
#include "crosshair.h"
#include "compat.h"
#ifdef DEVELOPER
#include "trackir/trackir.h"
#endif
#include "firstperson.h"
#include "thirdperson.h"
#include "debug/eh.h"

extern Offsets* g_Offsets;

Camera::Camera::Camera() noexcept : config(Config::GetCurrentConfig()) {
	cameraFirst = eastl::make_unique<Firstperson>(this);
	cameraThird = eastl::make_unique<Thirdperson>(this);

	if (Render::HasContext())
		Render::OnPresent(std::bind(&Camera::Camera::Render, this, std::placeholders::_1));
}

Camera::Camera::~Camera() {
	activeCamera = nullptr;
	cameraFirst.reset();
	cameraThird.reset();
}

// Called when the player toggles the POV
void Camera::Camera::OnTogglePOV(RE::ButtonEvent* ev) noexcept {
	povWasPressed = true;
	if (activeCamera)
		activeCamera->OnTogglePOV(ev);
}

void Camera::Camera::OnKeyPress(const RE::ButtonEvent* ev) noexcept {
	auto code = static_cast<int32_t>(ev->idCode);
	if (code <= 0x6 && ev->device == RE::INPUT_DEVICE::kMouse)
		code += 0x100;
	else if (ev->device == RE::INPUT_DEVICE::kGamepad)
		code = Util::GamepadMaskToKeycode(code);

	if (!inMenuMode) {
		// Cycle next preset
		if (config->nextPresetKey >= 0 && code == config->nextPresetKey && ev->heldDownSecs <= 0.000001f) {
			// wrap to 0..5
			if (++currentPresetIndex > (Config::MaxPresetSlots - 1)) currentPresetIndex = 0;

			// Try and load from currentIndex, up to n slots wrapping back to startIndex
			const auto startIndex = currentPresetIndex;
			while (!Config::LoadPreset(currentPresetIndex)) {
				if (++currentPresetIndex > (Config::MaxPresetSlots - 1)) currentPresetIndex = 0;
				if (currentPresetIndex == startIndex) break; // No valid preset to load
			}

			return;
		} else if (config->modToggleKey >= 0 && code == config->modToggleKey && ev->heldDownSecs <= 0.000001f) {
			// Mod enable/disable
			config->modDisabled = !config->modDisabled;
			return;
		}
	}

	if (activeCamera && activeCamera->OnKeyPress(ev)) return;
}

void Camera::Camera::OnMenuOpenClose(MenuID id, const RE::MenuOpenCloseEvent* const ev) noexcept {
	if (activeCamera && activeCamera->OnMenuOpenClose(id, ev)) return;

	switch (id) {
		case MenuID::InventoryMenu: [[fallthrough]];
		case MenuID::DialogMenu: [[fallthrough]];
		case MenuID::MapMenu: break;
		case MenuID::LoadingMenu:
			wasLoading = true;
			[[fallthrough]];
		default: {
			loadScreenDepth = glm::clamp<int8_t>(
				loadScreenDepth + (ev->opening ? 1 : -1),
				0,
				127
			);
			break;
		}
	}
}

void Camera::Camera::OnMenuModeChange(bool isMenuMode) noexcept {
	inMenuMode = isMenuMode;
}

bool Camera::Camera::IsInputLocked(RE::TESCameraState* state) noexcept {
	if (!cameraThird) return false;
	return cameraThird->IsInputLocked(state);
}

// Updates our POV state to the true value the game expects for each state
const bool Camera::Camera::UpdateCameraPOVState(const RE::PlayerCamera* camera) noexcept {
	povIsThird = GameState::IsInAutoVanityCamera(camera) ||
		GameState::IsInCameraTransition(camera) || GameState::IsInUsingObjectCamera(camera) ||
		GameState::IsInKillMove(camera) || GameState::IsInBleedoutCamera(camera) ||
		GameState::IsInFurnitureCamera(camera) || GameState::IsInHorseCamera(camera) ||
		GameState::IsInDragonCamera(camera) || GameState::IsThirdPerson(camera);
	return povIsThird;
}

const GameState::CameraState Camera::Camera::GetCurrentCameraState()  const noexcept {
	return currentState;
}

const Camera::CameraActionState Camera::Camera::GetCurrentCameraActionState() const noexcept {
	return currentActionState;
}

// Returns the current camera state for use in selecting an update method
const GameState::CameraState Camera::Camera::UpdateCurrentCameraState(RE::PlayerCharacter* player,
	const RE::Actor* forRef, RE::PlayerCamera* camera) noexcept
{
	const GameState::CameraState newState = GameState::GetCameraState(forRef, camera);
	if (newState != currentState) {
		if (!OnCameraStateTransition(player, camera, newState, currentState)) {
			lastState = currentState;
			currentState = newState;
		}
	}
	return currentState;
}

// Returns the current camera action state for use in the selected update method
const Camera::CameraActionState Camera::Camera::UpdateCurrentCameraActionState(const RE::PlayerCharacter* player,
	const RE::Actor* forRef, const RE::PlayerCamera* camera) noexcept
{
	CameraActionState newState = CameraActionState::Unknown;

	if (GameState::IsInAutoVanityCamera(camera)) {
		newState = CameraActionState::Vanity;
	} else if (GameState::IsVampireLord(forRef)) {
		newState = CameraActionState::VampireLord;
	} else if (GameState::IsWerewolf(forRef)) {
		newState = CameraActionState::Werewolf;
	} else if (camera->currentState == camera->cameraStates[RE::CameraStates::kMount]) {
		// FPV camera mod compat
		if (GameState::IsFirstPerson(camera)) {
			newState = CameraActionState::FirstPersonHorseback;
		} else if (GameState::IsDisMountingHorse(forRef)) {
			newState = CameraActionState::DisMounting;
		} else {
			newState = CameraActionState::Horseback;
		}
	} else if (GameState::IsInDragonCamera(camera)) {
		// FPV camera mod compat
		if (GameState::IsFirstPerson(camera)) {
			newState = CameraActionState::FirstPersonDragon;
		} else {
			newState = CameraActionState::Dragon;
		}
	} else if (GameState::IsSleeping(forRef)) {
		newState = CameraActionState::Sleeping;
	} else if (GameState::IsInFurnitureCamera(camera)) {
		newState = CameraActionState::SittingTransition;
	} else if (GameState::IsSitting(forRef)) {
		// FPV camera mod compat
		if (GameState::IsFirstPerson(camera)) {
			newState = CameraActionState::FirstPersonSitting;
		} else {
			newState = CameraActionState::Sitting;
		}
	} else if (GameState::IsBowDrawn(forRef)) { // Bow being drawn should have priority here
		newState = CameraActionState::Aiming;
	} else if (GameState::IsSneaking(forRef)) {
		newState = CameraActionState::Sneaking;
	} else if (GameState::IsSwimming(forRef)) {
		newState = CameraActionState::Swimming;
	} else if (GameState::IsSprinting(forRef)) {
		newState = CameraActionState::Sprinting;
	} else if (GameState::IsWalking(forRef)) {
		newState = CameraActionState::Walking;
	} else if (GameState::IsRunning(forRef)) {
		newState = CameraActionState::Running;
	} else {
		newState = CameraActionState::Standing;
	}

	if (newState != currentActionState) {
		lastActionState = currentActionState;
		currentActionState = newState;
		OnCameraActionStateTransition(player, newState, lastActionState);
	}

	return newState;
}

// Triggers when the camera action state changes, for debugging
void Camera::Camera::OnCameraActionStateTransition(const RE::PlayerCharacter* player,
	const CameraActionState newState, const CameraActionState oldState) const noexcept
{
	if (activeCamera)
		activeCamera->OnCameraActionStateTransition(player, newState, oldState);
}

// Triggers when the camera state changes
bool Camera::Camera::OnCameraStateTransition(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
	const GameState::CameraState newState, const GameState::CameraState oldState) noexcept
{
	if (activeCamera)
		return activeCamera->OnCameraStateTransition(player, camera, newState, oldState);
	return false;
}

void Camera::Camera::SetPosition(const glm::vec3& pos, const RE::PlayerCamera* camera, RE::NiCamera* niCamera) noexcept {
#ifdef _DEBUG
	if (!mmath::IsValid(pos)) {
		__debugbreak();
		return;
	}
#endif

	auto& cameraNode = camera->cameraRoot;
	cameraNode->local.translate = cameraNode->world.translate = (niCamera ? niCamera : cameraNi.get())->world.translate =
		{ pos.x, pos.y, pos.z };

	if (currentState == GameState::CameraState::ThirdPerson || currentState == GameState::CameraState::ThirdPersonCombat) {
		auto state = reinterpret_cast<RE::ThirdPersonState*>(camera->currentState.get());
		state->translation = cameraNode->local.translate;
	}
}

void Camera::Camera::UpdateInternalWorldToScreenMatrix(RE::NiCamera* niCamera) noexcept {
	auto camNi = niCamera ? niCamera : cameraNi.get();
	typedef void(*UpdateWorldToScreenMtx)(RE::NiCamera*);
	static auto toScreenFunc = REL::Relocation<UpdateWorldToScreenMtx>(g_Offsets->ComputeToScreenMatrix);
	toScreenFunc(camNi);
	worldToScaleform = *reinterpret_cast<mmath::NiMatrix44*>(camNi->worldToCam);
}

const RE::NiFrustum& Camera::Camera::GetFrustum() const noexcept {
	return frustum;
}

const glm::vec3& Camera::Camera::GetLastRecordedCameraPosition() const noexcept {
	return gameLastActualPosition;
}

RE::NiPointer<RE::Actor> Camera::Camera::GetCurrentCameraTarget(const RE::PlayerCamera* camera) noexcept {
	// Special case - In horse camera, just return the player
	if (GameState::IsInHorseCamera(camera))
		return RE::NiPointer<RE::Actor>(RE::PlayerCharacter::GetSingleton());

	// We should be looking at this first
	if (camera->cameraTarget)
		return camera->cameraTarget.get();

	const auto ctrls = RE::PlayerControls::GetSingleton();
	if (!ctrls) return RE::NiPointer<RE::Actor>(RE::PlayerCharacter::GetSingleton());

	if (ctrls->actionInterestedActor.size() == 0)
		return RE::NiPointer<RE::Actor>(RE::PlayerCharacter::GetSingleton());

	auto refr = ctrls->actionInterestedActor.front(); // Otherwise the most interested input observer
	if (!refr) return RE::NiPointer<RE::Actor>(RE::PlayerCharacter::GetSingleton());

	return refr.get();
}

bool Camera::Camera::InLoadingScreen() const noexcept {
	return loadScreenDepth != 0;
}

bool Camera::Camera::InMenuMode() const noexcept {
	return inMenuMode;
}

void Camera::Camera::SetShouldForceCameraState(bool force, uint8_t newCameraState) noexcept {
	wantNewCameraState = force;
	wantNewState = newCameraState;
}

// Get the thirdperson camera
Camera::Thirdperson* Camera::Camera::GetThirdpersonCamera() noexcept {
	return cameraThird.get();
}

// Get the firstperson camera
Camera::Firstperson* Camera::Camera::GetFirstpersonCamera() noexcept {
	return cameraFirst.get();
}

RE::NiPointer<RE::NiCamera> Camera::Camera::GetNiCamera(RE::PlayerCamera* camera) const noexcept {
	// Do other things parent stuff to the camera node? Better safe than sorry I guess
	if (camera->cameraRoot->children.size() == 0) return nullptr;
	for (auto& entry : camera->cameraRoot->children) {
		auto asCamera = skyrim_cast<RE::NiCamera*>(entry.get());
		if (asCamera) return RE::NiPointer<RE::NiCamera>(asCamera);
	}
	return nullptr;
}

void Camera::Camera::Render(Render::D3DContext& ctx) {
	if (InLoadingScreen()) return;
	if (activeCamera)
		activeCamera->Render(ctx);
}

// Use this method to snatch modifications done by mods that run after us 
// Called before the internal game method runs which will overwrite most of that
bool Camera::Camera::PreGameUpdate(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
	RE::BSTSmartPointer<RE::TESCameraState>& nextState)
{
	// Force the camera to a new state
	if (wantNewCameraState && !Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken()) {
		auto wantState = camera->cameraStates[wantNewState];
		if (wantState && camera->currentState != wantState) {
			nextState = wantState;
		}
		wantNewCameraState = false;
		return true;
	}

	if (Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken() &&
		!Messaging::SmoothCamInterface::GetInstance()->WantsInterpolatorUpdates())
		return false;

	if (activeCamera && activeCamera->OnPreGameUpdate(player, camera, nextState))
		return true;

	// Store the last actual position the game used for rendering
	cameraNi = GetNiCamera(camera);
	if (cameraNi) {
		gameLastActualPosition = {
			cameraNi->world.translate.x,
			cameraNi->world.translate.y,
			cameraNi->world.translate.z
		};
	}

	return false;
}

// Selects the correct update method and positions the camera
void Camera::Camera::UpdateCamera(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
	RE::BSTSmartPointer<RE::TESCameraState>& nextState)
{
	// Check if the user has turned the camera off
	// Also check if another mod has been given camera control via the interface
	const auto wantsControl = Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken();
	const auto wantsUpdates = Messaging::SmoothCamInterface::GetInstance()->WantsInterpolatorUpdates();

	if (wantsControl) {
		wasCameraAPIControlled = apiControlled = true;
	} else {
		wasCameraAPIControlled = apiControlled;
		apiControlled = false;
	}
		
	if (config->modDisabled || (wantsControl && !wantsUpdates)) {
		if (ranLastFrame) {
			if (activeCamera) activeCamera->OnEnd(player, camera, nullptr);
			cameraThird->GetCrosshairManager()->Reset();
			activeCamera = nullptr;
		}
		ranLastFrame = false;
		return;
	}

	// If we don't have an NiCamera, something else is likely very wrong
	if (!cameraNi) return;

	// Make sure the camera is following some valid reference
	auto refHandle = GetCurrentCameraTarget(camera);
	// We can pass the naked pointer around for the lifetime of the update function safely,
	// up until refHandle's destructor call
	currentFocusObject = refHandle.get();

	if (!currentFocusObject) {
		cameraNi = nullptr;
		return;
	}

	// Update states
	[[maybe_unused]] const auto state = UpdateCurrentCameraState(player, currentFocusObject, camera);
	const auto pov = UpdateCameraPOVState(camera);
	[[maybe_unused]] const auto actionState = UpdateCurrentCameraActionState(player, currentFocusObject, camera);

	// Set our frustum
	frustum = cameraNi->viewFrustum;

	// Now based on POV state, select the active camera
	ICamera* nextCamera = nullptr;
	if (GameState::IsFirstPerson(camera)) {
		nextCamera = cameraFirst.get();
		// Only query TrackIR in first person mode
#ifdef DEVELOPER
		if (TrackIR::IsRunning())
			trackIRData = TrackIR::GetTrackingData();
#endif
	} else if (pov && (skyrim_cast<RE::ThirdPersonState*>(camera->currentState.get()) ||
		camera->currentState->id == RE::CameraState::kAutoVanity))
		nextCamera = cameraThird.get();

	if (activeCamera != nextCamera) {
		if (activeCamera) activeCamera->OnEnd(player, camera, nextCamera);
		if (nextCamera)
			nextCamera->OnBegin(player, camera, activeCamera);
		activeCamera = nextCamera;
	}

	// Update the camera
	if (activeCamera)
		activeCamera->OnUpdateCamera(player, camera, nextState);

	cameraNi = nullptr;
	wasLoading = false;
	ranLastFrame = true;
	wasDialogOpen = false;
	povWasPressed = false;
}