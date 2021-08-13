#include "camera.h"
#include "crosshair.h"
#include "compat.h"
#ifdef DEVELOPER
#include "trackir/trackir.h"
#endif
#include "firstperson.h"
#include "thirdperson.h"

#include "debug/eh.h"

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
void Camera::Camera::OnTogglePOV(const ButtonEvent* ev) noexcept {
	povIsThird = !povIsThird;
	povWasPressed = true;
	if (activeCamera)
		activeCamera->OnTogglePOV(ev);
}

void Camera::Camera::OnKeyPress(const ButtonEvent* ev) noexcept {
	auto code = ev->keyMask;
	if (code <= 0x6 && ev->deviceType == kDeviceType_Mouse)
		code += 0x100;

	// Cycle next preset
	if (config->nextPresetKey >= 0 && code == config->nextPresetKey && ev->timer <= 0.000001f) {
		// wrap to 0..5
		if (++currentPresetIndex > 5) currentPresetIndex = 0;

		// Try and load from currentIndex, up to n slots wrapping back to startIndex
		const auto startIndex = currentPresetIndex;
		while (!Config::LoadPreset(currentPresetIndex)) {
			if (++currentPresetIndex > 5) currentPresetIndex = 0;
			if (currentPresetIndex == startIndex) break; // No valid preset to load
		}

		return;
	} else if (config->modToggleKey >= 0 && code == config->modToggleKey && ev->timer <= 0.000001f) {
		// Mod enable/disable
		config->modDisabled = !config->modDisabled;
	}


	if (activeCamera && activeCamera->OnKeyPress(ev)) return;
}

void Camera::Camera::OnMenuOpenClose(MenuID id, const MenuOpenCloseEvent* const ev) noexcept {
	if (activeCamera && activeCamera->OnMenuOpenClose(id, ev)) return;

	switch (id) {
		case MenuID::InventoryMenu:
		case MenuID::DialogMenu:
		case MenuID::MapMenu:
			break;
		case MenuID::LoadingMenu:
			wasLoading = true;
			[[fallthrough]];
		default: {
			loadScreenDepth = glm::clamp(
				loadScreenDepth + (ev->opening ? 1 : -1),
				0,
				127
			);
			break;
		}
	}
}

// Updates our POV state to the true value the game expects for each state
const bool Camera::Camera::UpdateCameraPOVState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept {
	povIsThird = GameState::IsInAutoVanityCamera(camera) ||
		GameState::IsInCameraTransition(camera) || GameState::IsInUsingObjectCamera(camera) ||
		GameState::IsInKillMove(camera) || GameState::IsInBleedoutCamera(camera) ||
		GameState::IsInFurnitureCamera(camera) || GameState::IsInHorseCamera(player, camera) ||
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
const GameState::CameraState Camera::Camera::UpdateCurrentCameraState(const PlayerCharacter* player,
	const CorrectedPlayerCamera* camera) noexcept
{
	const GameState::CameraState newState = GameState::GetCameraState(player, camera);
	const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraState);
	if (newState != currentState) {
		lastState = currentState;
		currentState = newState;
		OnCameraStateTransition(player, camera, newState, lastState);
	}
	return newState;
}

// Returns the current camera action state for use in the selected update method
const Camera::CameraActionState Camera::Camera::UpdateCurrentCameraActionState(const PlayerCharacter* player,
	const CorrectedPlayerCamera* camera) noexcept
{
	CameraActionState newState = CameraActionState::Unknown;

	if (GameState::IsVampireLord(player)) {
		newState = CameraActionState::VampireLord;
	} else if (GameState::IsWerewolf(player)) {
		newState = CameraActionState::Werewolf;
	} else if (camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Horse]) {
		// FPV camera mod compat
		if (GameState::IsFirstPerson(camera)) {
			newState = CameraActionState::FirstPersonHorseback;
		} else if (GameState::IsDisMountingHorse(player)) {
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
	} else if (GameState::IsSleeping(player)) {
		newState = CameraActionState::Sleeping;
	} else if (GameState::IsInFurnitureCamera(camera)) {
		newState = CameraActionState::SittingTransition;
	} else if (GameState::IsSitting(player)) {
		// FPV camera mod compat
		if (GameState::IsFirstPerson(camera)) {
			newState = CameraActionState::FirstPersonSitting;
		} else {
			newState = CameraActionState::Sitting;
		}
	} else if (GameState::IsBowDrawn(player)) { // Bow being drawn should have priority here
		newState = CameraActionState::Aiming;
	} else if (GameState::IsSneaking(player)) {
		newState = CameraActionState::Sneaking;
	} else if (GameState::IsSwimming(player)) {
		newState = CameraActionState::Swimming;
	} else if (GameState::IsSprinting(player)) {
		newState = CameraActionState::Sprinting;
	} else if (GameState::IsWalking(player)) {
		newState = CameraActionState::Walking;
	} else if (GameState::IsRunning(player)) {
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
void Camera::Camera::OnCameraActionStateTransition(const PlayerCharacter* player,
	const CameraActionState newState, const CameraActionState oldState) const noexcept
{
	if (activeCamera)
		activeCamera->OnCameraActionStateTransition(player, newState, oldState);
}

// Triggers when the camera state changes
void Camera::Camera::OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const GameState::CameraState newState, const GameState::CameraState oldState) noexcept
{
	if (activeCamera)
		activeCamera->OnCameraStateTransition(player, camera, newState, oldState);
}

void Camera::Camera::SetPosition(const glm::vec3& pos, const CorrectedPlayerCamera* camera) noexcept {
	auto cameraNode = camera->cameraNode;

#ifdef _DEBUG
	if (!mmath::IsValid(pos)) {
		__debugbreak();
		return;
	}
#endif

	cameraNode->m_localTransform.pos = cameraNode->m_worldTransform.pos = cameraNi->m_worldTransform.pos =
		{ pos.x, pos.y, pos.z };

	if (currentState == GameState::CameraState::ThirdPerson || currentState == GameState::CameraState::ThirdPersonCombat) {
		auto state = reinterpret_cast<CorrectedThirdPersonState*>(camera->cameraState);
		state->translation = cameraNode->m_localTransform.pos;
	}
}

void Camera::Camera::UpdateInternalWorldToScreenMatrix(const mmath::Rotation& rot, const CorrectedPlayerCamera* camera) noexcept {
	// Run the THT to get a world to scaleform matrix
	const auto lastRotation = cameraNi->m_worldTransform.rot;
	cameraNi->m_worldTransform.rot = rot.THT();
	// Force the game to compute the matrix for us
	typedef void(*UpdateWorldToScreenMtx)(NiCamera*);
	static auto toScreenFunc = Offsets::Get<UpdateWorldToScreenMtx>(69271);
	toScreenFunc(cameraNi);
	// Grab it
	worldToScaleform = *reinterpret_cast<mmath::NiMatrix44*>(cameraNi->m_aafWorldToCam);
	// Now restore the normal camera rotation
	cameraNi->m_worldTransform.rot = lastRotation;
	// And compute the normal toScreen matrix
	toScreenFunc(cameraNi);
}

const NiFrustum& Camera::Camera::GetFrustum() const noexcept {
	return frustum;
}

NiPointer<Actor> Camera::Camera::GetCurrentCameraTarget(const CorrectedPlayerCamera* camera) noexcept {
	const auto ctrls = PlayerControls::GetSingleton();
	if (!ctrls) return *g_thePlayer;

	auto controlled = reinterpret_cast<const tArray<UInt32>*>(&ctrls->unk150);
	auto controlledRef = controlled->count > 0 ? controlled->entries[controlled->count-1] : 0x0;

	if (controlledRef == 0) return *g_thePlayer;
	NiPointer<TESObjectREFR> refr;
	(*LookupREFRByHandle)(controlledRef, refr);

	if (refr) {
		NiPointer<Actor> actor = DYNAMIC_CAST(refr.get(), TESObjectREFR, Actor);
		if (actor) {
			// Special case - In horse camera, just return the player
			if (GameState::IsInHorseCamera(actor, camera))
				return *g_thePlayer;
			return actor;
		}
	}

	// Fallback to the player
	return *g_thePlayer;
}

bool Camera::Camera::InLoadingScreen() const noexcept {
	return loadScreenDepth != 0;
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

NiPointer<NiCamera> Camera::Camera::GetNiCamera(CorrectedPlayerCamera* camera) const noexcept {
	// Do other things parent stuff to the camera node? Better safe than sorry I guess
	if (camera->cameraNode->m_children.m_size == 0) return nullptr;
	for (auto i = 0; i < camera->cameraNode->m_children.m_size; i++) {
		auto entry = camera->cameraNode->m_children.m_data[i];
		auto asCamera = DYNAMIC_CAST(entry, NiAVObject, NiCamera);
		if (asCamera) return asCamera;
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
bool Camera::Camera::PreGameUpdate(PlayerCharacter* player, CorrectedPlayerCamera* camera,
	BSTSmartPointer<TESCameraState>& nextState)
{
	if (Messaging::SmoothCamAPIV1::GetInstance()->IsCameraTaken()) return false;

	// Check if ACC should be in control of the camera
	if (MenuTopicManager::GetSingleton()->unkB1 && Compat::IsPresent(Compat::Mod::AlternateConversationCamera)) {
		if (!accControl) {
			accSavePitch = player->rot.x;
			accControl = true;
		}
		return false;
	} else if (accControl) {
		player->rot.x = accSavePitch;
		wasDialogOpen = true;
		accControl = false;
	}

	// Force the camera to a new state
	if (wantNewCameraState) {
		auto wantState = camera->cameraStates[wantNewState];
		if (wantState && camera->cameraState != wantState) {
			if (nextState.ptr)
				InterlockedDecrement(&nextState.ptr->refCount.m_refCount);
			InterlockedIncrement(&wantState->refCount.m_refCount);
			nextState.ptr = wantState;
		}
		wantNewCameraState = false;
		return true;
	}

	if (activeCamera && activeCamera->OnPreGameUpdate(player, camera, nextState))
		return true;

	// Store the last actual position the game used for rendering
	cameraNi = GetNiCamera(camera);
	if (cameraNi) {
		gameLastActualPosition = {
			cameraNi->m_worldTransform.pos.x,
			cameraNi->m_worldTransform.pos.y,
			cameraNi->m_worldTransform.pos.z
		};
	}

	return false;
}

// Selects the correct update method and positions the camera
void Camera::Camera::UpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera,
	BSTSmartPointer<TESCameraState>& nextState)
{
	// Check if the user has turned the camera off
	// Also check if another mod has been given camera control via the interface
	if (config->modDisabled || Messaging::SmoothCamAPIV1::GetInstance()->IsCameraTaken()) {
		if (ranLastFrame) {
			if (activeCamera) activeCamera->OnEnd(player, camera, nullptr);
			cameraThird->GetCrosshairManager()->Reset();
			activeCamera = nullptr;
		}
		ranLastFrame = false;
		return;
	}

	// Check if ACC should be in control of the camera
	if (MenuTopicManager::GetSingleton()->unkB1 && Compat::IsPresent(Compat::Mod::AlternateConversationCamera)) {
		if (!accControl) {
			accSavePitch = player->rot.x;
			accControl = true;
		}
		return;
	} else if (accControl) {
		player->rot.x = accSavePitch;
		wasDialogOpen = true;
		accControl = false;
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
	const auto state = UpdateCurrentCameraState(player, camera);
	const auto pov = UpdateCameraPOVState(player, camera);
	const auto actionState = UpdateCurrentCameraActionState(player, camera);

	// Set our frustum
	frustum = cameraNi->m_frustum;

	// Now based on POV state, select the active camera
	ICamera* nextCamera = nullptr;
	if (GameState::IsFirstPerson(camera)) {
		nextCamera = cameraFirst.get();
		// Only query TrackIR in first person mode
#ifdef DEVELOPER
		if (TrackIR::IsRunning())
			trackIRData = TrackIR::GetTrackingData();
#endif
	} else if (pov)
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
}