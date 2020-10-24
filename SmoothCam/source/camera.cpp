#include "camera.h"
#include "crosshair.h"

double CurTime() noexcept;
double CurQPC() noexcept;
double GetFrameDelta() noexcept;
double GetQPCDelta() noexcept;

Camera::SmoothCamera::SmoothCamera() : config(Config::GetCurrentConfig()) {
	cameraStates[static_cast<size_t>(GameState::CameraState::ThirdPerson)] =
		std::move(std::make_unique<State::ThirdpersonState>(this));
	cameraStates[static_cast<size_t>(GameState::CameraState::ThirdPersonCombat)] =
		std::move(std::make_unique<State::ThirdpersonCombatState>(this));
	cameraStates[static_cast<size_t>(GameState::CameraState::Horseback)] =
		std::move(std::make_unique<State::ThirdpersonHorseState>(this));

	crosshair = std::make_unique<Crosshair::Manager>();

	if (Render::HasContext()) {
		Render::OnPresent(std::bind(&Camera::SmoothCamera::Render, this, std::placeholders::_1));

#ifdef WITH_CHARTS
		refTreeDisplay = std::make_unique<Render::NiNodeTreeDisplay>(600, 1080, Render::GetContext());

		worldPosTargetGraph = std::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		offsetPosGraph = std::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		offsetTargetPosGraph = std::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		localSpaceGraph = std::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		rotationGraph = std::make_unique<Render::LineGraph>(2, 128, 600, 128, Render::GetContext());
		computeTimeGraph = std::make_unique<Render::LineGraph>(2, 128, 600, 128, Render::GetContext());

		const auto xColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		const auto yColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		const auto zColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		worldPosTargetGraph->SetLineColor(0, xColor);
		worldPosTargetGraph->SetLineColor(1, yColor);
		worldPosTargetGraph->SetLineColor(2, zColor);
		worldPosTargetGraph->SetName(L"World Pos");

		localSpaceGraph->SetLineColor(0, xColor);
		localSpaceGraph->SetLineColor(1, yColor);
		localSpaceGraph->SetLineColor(2, zColor);
		localSpaceGraph->SetPosition(0, 128);
		localSpaceGraph->SetName(L"Local Space");

		offsetPosGraph->SetLineColor(0, xColor);
		offsetPosGraph->SetLineColor(1, yColor);
		offsetPosGraph->SetLineColor(2, zColor);
		offsetPosGraph->SetPosition(0, 256);
		offsetPosGraph->SetName(L"Offset Pos");

		offsetTargetPosGraph->SetLineColor(0, xColor);
		offsetTargetPosGraph->SetLineColor(1, yColor);
		offsetTargetPosGraph->SetLineColor(2, zColor);
		offsetTargetPosGraph->SetPosition(0, 384);
		offsetTargetPosGraph->SetName(L"Target Offset Pos");

		rotationGraph->SetLineColor(0, xColor);
		rotationGraph->SetLineColor(1, yColor);
		rotationGraph->SetPosition(0, 512);
		rotationGraph->SetName(L"Current Rotation (Pitch, Yaw)");

		computeTimeGraph->SetLineColor(0, { 1.0f, 0.266f, 0.984f, 1.0f });
		computeTimeGraph->SetLineColor(1, { 1.0f, 0.541f, 0.218f, 1.0f });
		computeTimeGraph->SetName(L"Compute Time <pink> (Camera::Update()), Frame Time <orange>, seconds");
		computeTimeGraph->SetPosition(0, 640);

		Render::CBufferCreateInfo cbuf;
		cbuf.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		cbuf.cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbuf.size = sizeof(glm::mat4);
		cbuf.initialData = &orthoMatrix;
		perFrameOrtho = std::make_unique<Render::CBuffer>(cbuf, Render::GetContext());
#endif
	}
}

Camera::SmoothCamera::~SmoothCamera() {
	crosshair.reset();
}

// Called when the player toggles the POV
void Camera::SmoothCamera::OnTogglePOV(const ButtonEvent* ev) noexcept {
	povIsThird = !povIsThird;
	povWasPressed = true;
}

void Camera::SmoothCamera::OnKeyPress(const ButtonEvent* ev) noexcept {
	// SkyUI gives us different key codes for mouse buttons, this is a nasty hack to work around that
	auto code = ev->keyMask;
	if (code <= 0x6) { // @HACK: I'm not sure how many mouse codes there are in the game, or what we might be overlapping here
		code += 0x100;
	}

	if (config->shoulderSwapKey >= 0 && config->shoulderSwapKey == code && ev->timer <= 0.000001f)
		shoulderSwap = shoulderSwap == 1 ? -1 : 1;
}

void Camera::SmoothCamera::OnMenuOpenClose(MenuID id, const MenuOpenCloseEvent* const ev) noexcept {
	switch (id) {
		case MenuID::DialogMenu: {
			dialogMenuOpen = ev->opening;
			break;
		}
		default: {
			loadScreenDepth = glm::clamp(
				loadScreenDepth + (ev->opening ? 1 : -1),
				0,
				255
			);
			break;
		}
	}
}

glm::vec3 Camera::SmoothCamera::GetCurrentPosition() const noexcept {
	return currentPosition;
}

// Updates our POV state to the true value the game expects for each state
const bool Camera::SmoothCamera::UpdateCameraPOVState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept {
	const auto zoom = reinterpret_cast<const CorrectedThirdPersonState*>(camera)->cameraZoom;
	const auto lzoom = reinterpret_cast<const CorrectedThirdPersonState*>(camera)->cameraLastZoom;
	povIsThird = zoom == 0.0f || GameState::IsInAutoVanityCamera(camera) || GameState::IsInTweenCamera(camera) ||
		GameState::IsInCameraTransition(camera) || GameState::IsInUsingObjectCamera(camera) || GameState::IsInKillMove(camera) ||
		GameState::IsInBleedoutCamera(camera) || GameState::IsInFurnitureCamera(camera) || GameState::IsInHorseCamera(camera) ||
		GameState::IsInDragonCamera(camera) || GameState::IsThirdPerson(camera);
	return povIsThird;
}

#pragma region Camera state updates
// Check if the camera is near the player's head (for first person mods)
bool Camera::SmoothCamera::CameraNearHead(const PlayerCharacter* player, const CorrectedPlayerCamera* camere, float cutOff) {
	if (!player->loadedState || !player->loadedState->node || !Strings.head.data) return false;

	// Grab the eye vector, if we can't find the head node the origin will be our fallback
	NiPoint3 niOrigin, niNormal;
	typedef void(__thiscall PlayerCharacter::* GetEyeVector)(NiPoint3& origin, NiPoint3& normal, bool factorCameraOffset) const;
	(player->*reinterpret_cast<GetEyeVector>(&PlayerCharacter::Unk_C2))(niOrigin, niNormal, false);

	auto node = player->loadedState->node->GetObjectByName(&Strings.head.data);
	if (node) {
		niOrigin = node->m_worldTransform.pos;
	}

	const auto dist = glm::distance(
		glm::vec3{
			niOrigin.x,
			niOrigin.y,
			niOrigin.z
		},
		gameLastActualPosition
	);

	return dist <= cutOff;
}

// Immersive First Person patch
bool Camera::SmoothCamera::IFPV_InFirstPersonState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) {
	if (!player->loadedState || !player->loadedState->node || !Strings.npc.data) return false;

	const auto npc = player->loadedState->node->GetObjectByName(&Strings.npc.data);
	if (!npc) return false;

	const auto bits = std::bitset<32>(npc->m_flags);
	if (!bits[26]) {
		if ((player->firstPersonSkeleton->m_flags & 1) == 0) // 0xe in IFPV, 0xf otherwise
			return true;
	}

	return false;
}

// Returns the current camera state for use in selecting an update method
const GameState::CameraState Camera::SmoothCamera::GetCurrentCameraState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) {
	GameState::CameraState newState = GameState::CameraState::Unknown;
	const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraState);

	// Improved camera - Sitting
	if (!povWasPressed && !GameState::IsInHorseCamera(camera) && !GameState::IsInDragonCamera(camera) && GameState::IsSitting(player) 
		&& !GameState::IsSleeping(player) && config->compatIC_FirstPersonSitting)
	{
		if (tps && tps->cameraZoom < -1.0f && tps->cameraLastZoom < -1.0f) {
			newState = GameState::CameraState::FirstPerson;
			goto applyStateTransition; // SUE ME
		}
	}

	newState = GameState::GetCameraState(player, camera);
	const auto minZoom = Config::GetGameConfig()->fMinCurrentZoom;

	// IFPV Compat
	if (config->compatIFPV && (newState == GameState::CameraState::ThirdPerson || newState == GameState::CameraState::ThirdPersonCombat)) {
		if (IFPV_InFirstPersonState(player, camera))
			newState = GameState::CameraState::FirstPerson;
	} else if (config->compatIFPV && newState == GameState::CameraState::Horseback) {
		// ditto
		if (IFPV_InFirstPersonState(player, camera))
			newState = GameState::CameraState::FirstPerson;
	}
	
	// Improved camera - Horse and dragon
	if (newState == GameState::CameraState::Horseback && config->comaptIC_FirstPersonHorse && !config->compatIFPV) {
		if (tps) {
			if ((tps->cameraZoom == minZoom && tps->cameraLastZoom == minZoom) ||
				currentActionState == CameraActionState::FirstPersonHorseback ||
				CameraNearHead(player, camera))
			{
				if (povWasPressed)
					newState = GameState::CameraState::Horseback;
				else
					newState = GameState::CameraState::FirstPerson;
			} else {
				newState = GameState::CameraState::Horseback;
			}
		} else {
			newState = GameState::CameraState::Horseback;
		}
	} else if (newState == GameState::CameraState::Dragon && config->comaptIC_FirstPersonDragon) {
		if (tps) {
			if ((tps->cameraZoom == minZoom && tps->cameraLastZoom == minZoom) ||
				currentActionState == CameraActionState::FirstPersonDragon)
			{
				newState = GameState::CameraState::FirstPerson;
			} else {
				newState = GameState::CameraState::Dragon;
			}
		} else {
			newState = GameState::CameraState::Dragon;
		}
	}

applyStateTransition:
	if (newState != currentState) {
		lastState = currentState;
		currentState = newState;
		OnCameraStateTransition(player, camera, newState, lastState);
	}
	return newState;
}

// Returns the current camera action state for use in the selected update method
const Camera::CameraActionState Camera::SmoothCamera::GetCurrentCameraActionState(const PlayerCharacter* player,
	const CorrectedPlayerCamera* camera) noexcept
{
	CameraActionState newState = CameraActionState::Unknown;

	if (GameState::IsInHorseCamera(camera)) {
		// Improved camera compat
		if (!povIsThird) {
			newState = CameraActionState::FirstPersonHorseback;
		} else if (GameState::IsDisMountingHorse(player)) {
			newState = CameraActionState::DisMounting;
		}
	} else if (GameState::IsInDragonCamera(camera)) {
		// Improved camera compat
		if (currentState == GameState::CameraState::FirstPerson) {
			newState = CameraActionState::FirstPersonDragon;
		}
	} else if (GameState::IsSleeping(player)) {
		newState = CameraActionState::Sleeping;
	} else if (GameState::IsInFurnitureCamera(camera)) {
		newState = CameraActionState::SittingTransition;
	} else if (GameState::IsSitting(player)) {
		// Improved camera compat
		if (currentState == GameState::CameraState::FirstPerson) {
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
#ifdef _DEBUG
		OnCameraActionStateTransition(player, newState, lastActionState);
#endif
	}

	return newState;
}

#ifdef _DEBUG
// Triggers when the camera action state changes, for debugging
void Camera::SmoothCamera::OnCameraActionStateTransition(const PlayerCharacter* player,
	const CameraActionState newState, const CameraActionState oldState) const noexcept
{
	// For debugging
}
#endif

// Triggers when the camera state changes
void Camera::SmoothCamera::OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const GameState::CameraState newState, const GameState::CameraState oldState)
{
	switch (oldState) {
		case GameState::CameraState::ThirdPerson: {
			cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson))->OnEnd(player, currentFocusObject, camera);
			break;
		}
		case GameState::CameraState::ThirdPersonCombat: {
			cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat))->OnEnd(player, currentFocusObject, camera);
			break;
		}
		case GameState::CameraState::Horseback: {
			cameraStates.at(static_cast<size_t>(GameState::CameraState::Horseback))->OnEnd(player, currentFocusObject, camera);
			break;
		}
		case GameState::CameraState::Tweening: {
			// Don't copy positions from this state
			break;
		}
		default:
			// Store the position for smoothing in the new state
			lastPosition = lastWorldPosition = currentPosition = glm::vec3{
				camera->cameraNode->m_worldTransform.pos.x,
				camera->cameraNode->m_worldTransform.pos.y,
				camera->cameraNode->m_worldTransform.pos.z
			};
			break;
	}

	switch (newState) {
		case GameState::CameraState::ThirdPerson: {
			cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson))->OnBegin(player, currentFocusObject, camera);
			break;
		}
		case GameState::CameraState::ThirdPersonCombat: {
			cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat))->OnBegin(player, currentFocusObject, camera);
			break;
		}
		case GameState::CameraState::Horseback: {
			cameraStates.at(static_cast<size_t>(GameState::CameraState::Horseback))->OnBegin(player, currentFocusObject, camera);
			break;
		}
		case GameState::CameraState::Free: {
			// Forward our position to the free cam state
			auto state = reinterpret_cast<FreeCameraState*>(camera->cameraState);
			state->unk30[0] = lastPosition.x;
			state->unk30[1] = lastPosition.y;
			state->unk30[2] = lastPosition.z;

			auto quat = NiQuaternion{ currentQuat.w, currentQuat.x, currentQuat.y, currentQuat.z };

			//FUN_140848880((longlong)ppuVar3,local_18);
			typedef void(__fastcall* UpdateFreeCamTransform)(FreeCameraState*, NiQuaternion&);
			Offsets::Get<UpdateFreeCamTransform>(49816)(state, quat);
			
			// fallthrough
		}
		default: {
			// Do this once here on transition to a new state we don't run in
			crosshair->SetCrosshairEnabled(true);
			crosshair->CenterCrosshair();
			crosshair->SetDefaultSize();
			break;
		}
	}
}
#pragma endregion

#pragma region Camera position calculations
// Returns the zoom value set from the given camera state
float Camera::SmoothCamera::GetCurrentCameraZoom(const CorrectedPlayerCamera* camera, const GameState::CameraState currentState) const noexcept {
	switch (currentState) {
		case GameState::CameraState::ThirdPerson:
		case GameState::CameraState::ThirdPersonCombat: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_ThirdPerson2);
		}
		case GameState::CameraState::Horseback: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_Horse);
		}
		case GameState::CameraState::Dragon: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_Dragon);
		}
		case GameState::CameraState::Bleedout: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_Bleedout);
		}
		default:
			return 0.0f;
	}
}

// Returns an offset group for the current player movement state
const Config::OffsetGroup* Camera::SmoothCamera::GetOffsetForState(const CameraActionState state) const noexcept {
	switch (state) {
		case CameraActionState::DisMounting: {
			return &config->standing; // Better when dismounting
		}
		case CameraActionState::Sleeping: {
			return &config->sitting;
		}
		case CameraActionState::Sitting: {
			return &config->sitting;
		}
		case CameraActionState::Sneaking: {
			return &config->sneaking;
		}
		case CameraActionState::Aiming: {
			return &config->bowAim;
		}
		case CameraActionState::Swimming: {
			return &config->swimming;
		}
		case CameraActionState::Sprinting: {
			return &config->sprinting;
		}
		case CameraActionState::Walking: {
			return &config->walking;
		}
		case CameraActionState::Running: {
			return &config->running;
		}
		case CameraActionState::Standing: {
			return &config->standing;
		}
		default: {
			return &config->standing;
		}
	}
}

float Camera::SmoothCamera::GetActiveWeaponStateZoomOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept {
	if (!GameState::IsWeaponDrawn(player)) return group->zoomOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedZoomOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicZoomOffset;
	}
	return group->combatMeleeZoomOffset;
}

// Selects the right offset from an offset group for the player's weapon state
float Camera::SmoothCamera::GetActiveWeaponStateUpOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept {
	if (!GameState::IsWeaponDrawn(player)) return group->upOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedUpOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicUpOffset;
	}
	return group->combatMeleeUpOffset;
}

// Selects the right offset from an offset group for the player's weapon state
float Camera::SmoothCamera::GetActiveWeaponStateSideOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept {
	if (!GameState::IsWeaponDrawn(player)) return group->sideOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedSideOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicSideOffset;
	}
	return group->combatMeleeSideOffset;
}

float Camera::SmoothCamera::GetCurrentCameraZoomOffset(PlayerCharacter* player) const noexcept {
	switch (currentState) {
		case GameState::CameraState::Horseback: {
			if (GameState::IsBowDrawn(player)) {
				return config->bowAim.horseZoomOffset;
			} else {
				return GetActiveWeaponStateUpOffset(player, &config->horseback);
			}
		}
		default:
			break;
	}

	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Aiming:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->zoomOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateZoomOffset(player, offsetState.currentGroup);
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

// Returns the camera height for the current player state
float Camera::SmoothCamera::GetCurrentCameraHeight(PlayerCharacter* player) const noexcept {
	switch (currentState) {
		case GameState::CameraState::Horseback: {
			if (GameState::IsBowDrawn(player)) {
				return config->bowAim.horseUpOffset;
			} else {
				return GetActiveWeaponStateUpOffset(player, &config->horseback);
			}
		}
		default:
			break;
	}

	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Aiming:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->upOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateUpOffset(player, offsetState.currentGroup);
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

// Returns the camera side offset for the current player state
float Camera::SmoothCamera::GetCurrentCameraSideOffset(PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept {
	switch (currentState) {
		case GameState::CameraState::Horseback: {
			if (GameState::IsBowDrawn(player)) {
				return config->bowAim.horseSideOffset * shoulderSwap;
			} else {
				return GetActiveWeaponStateSideOffset(player, &config->horseback) * shoulderSwap;
			}
		}
		default:
			break;
	}

	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Aiming:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->sideOffset * shoulderSwap;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateSideOffset(player, offsetState.currentGroup) * shoulderSwap;
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

// Returns the ideal camera distance for the current zoom level
float Camera::SmoothCamera::GetCurrentCameraDistance(const CorrectedPlayerCamera* camera) const noexcept {
	return -(config->minCameraFollowDistance + (GetCurrentCameraZoom(camera, currentState) * config->zoomMul));
}

// Returns the full local-space camera offset for the current player state
glm::vec3 Camera::SmoothCamera::GetCurrentCameraOffset(PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept {
	return {
		GetCurrentCameraSideOffset(player, camera),
		GetCurrentCameraDistance(camera) + GetCurrentCameraZoomOffset(player),
		GetCurrentCameraHeight(player)
	};
}

NiAVObject* Camera::SmoothCamera::FindFollowBone(const TESObjectREFR* ref) const noexcept {
	if (!ref->loadedState || !ref->loadedState->node) return nullptr;
	auto& boneList = Config::GetBonePriorities();

	for (auto it = boneList.begin(); it != boneList.end(); it++) {
		auto node = ref->loadedState->node->GetObjectByName(&it->data);
		if (node) return node;
	}

	return nullptr;
}

// Returns the full world-space camera target postion for the current player state
glm::vec3 Camera::SmoothCamera::GetCurrentCameraTargetWorldPosition(const TESObjectREFR* ref,
	const CorrectedPlayerCamera* camera) const
{
	if (ref->loadedState && ref->loadedState->node && Strings.spine1.data) {
		NiAVObject* node;
		if (currentState == GameState::CameraState::Horseback) {
			node = ref->loadedState->node->GetObjectByName(&Strings.spine1.data);
		} else {
			node = FindFollowBone(ref);
		}

		if (node) {
			return glm::vec3(
				ref->pos.x,
				ref->pos.y,
				node->m_worldTransform.pos.z
			);
		}
	}

	return {
		ref->pos.x,
		ref->pos.y,
		ref->pos.z
	};
}

void Camera::SmoothCamera::SetPosition(const glm::vec3& pos, const CorrectedPlayerCamera* camera) noexcept {
	auto cameraNode = camera->cameraNode;
	auto cameraNi = reinterpret_cast<NiCamera*>(
		cameraNode->m_children.m_size == 0 ?
		nullptr :
		cameraNode->m_children.m_data[0]
	);
	if (!cameraNi) return;

	currentPosition = pos;

#ifdef _DEBUG
	if (!mmath::IsValid(currentPosition)) {
		__debugbreak();
		// Oops, go ahead and clear both
		lastPosition = currentPosition = gameInitialWorldPosition;
		return;
	}
#endif

	const NiPoint3 niPos = { currentPosition.x, currentPosition.y, currentPosition.z };
	cameraNode->m_localTransform.pos = niPos;
	cameraNode->m_worldTransform.pos = niPos;
	cameraNi->m_worldTransform.pos = niPos;

	if (currentState == GameState::CameraState::ThirdPerson || currentState == GameState::CameraState::ThirdPersonCombat) {
		auto state = reinterpret_cast<CorrectedThirdPersonState*>(camera->cameraState);
		state->translation = niPos;
	}

	// Update world to screen matrices
	UpdateInternalWorldToScreenMatrix(camera, cameraNi, GetCameraPitchRotation(), GetCameraYawRotation());
}

void Camera::SmoothCamera::UpdateInternalWorldToScreenMatrix(const CorrectedPlayerCamera* camera, NiCamera* cameraNi,
	float pitch, float yaw) noexcept
{
	// Run the THT to get a world to scaleform matrix
	auto lastRotation = cameraNi->m_worldTransform.rot;
	cameraNi->m_worldTransform.rot = mmath::ToddHowardTransform(pitch, yaw);
	// Force the game to compute the matrix for us
	static auto toScreenFunc = Offsets::Get<UpdateWorldToScreenMtx>(69271);
	toScreenFunc(cameraNi);
	// Grab it
	worldToScaleform = *reinterpret_cast<mmath::NiMatrix44*>(cameraNi->m_aafWorldToCam);
	// Now restore the normal camera rotation
	cameraNi->m_worldTransform.rot = lastRotation;
	// And compute the normal toScreen matrix
	toScreenFunc(cameraNi);
}

// Returns the current smoothing scalar to use for the given distance to the player
double Camera::SmoothCamera::GetCurrentSmoothingScalar(const float distance, ScalarSelector method) const {
	Config::ScalarMethods scalarMethod;
	
	// Work in FP64 here to eek out some more precision
	// Avoid a divide-by-zero error by clamping to this lower bound
	constexpr const double minZero = 0.000000000001;

	double scalar = 1.0;
	double interpValue = 1.0;
	double remapped = 1.0;

	if (method == ScalarSelector::SepZ) {
		const auto max = static_cast<double>(config->separateZMaxSmoothingDistance);
		scalar = glm::clamp(glm::max(1.0 - (max - distance), minZero) / max, 0.0, 1.0);
		remapped = mmath::Remap<double>(
			scalar, 0.0, 1.0, static_cast<double>(config->separateZMinFollowRate), static_cast<double>(config->separateZMaxFollowRate)
		);
		scalarMethod = config->separateZScalar;
	} else if (method == ScalarSelector::LocalSpace) {
		remapped = distance;
		scalarMethod = config->separateLocalScalar;
	} else {
		const auto max = static_cast<double>(config->zoomMaxSmoothingDistance);
		scalar = glm::clamp(glm::max(1.0 - (max - distance), minZero) / max, 0.0, 1.0);
		remapped = mmath::Remap<double>(
			scalar, 0.0, 1.0, static_cast<double>(config->minCameraFollowRate), static_cast<double>(config->maxCameraFollowRate)
		);
		scalarMethod = config->currentScalar;
	}

	if (!config->disableDeltaTime) {
		const double delta = glm::max(GetFrameDelta(), minZero);
		const double fps = 1.0 / delta;
		const double mul = -fps * glm::log2(1.0 - remapped);
		interpValue = glm::clamp(1.0 - glm::exp2(-mul * delta), 0.0, 1.0);
	} else {
		interpValue = remapped;
	}

	return mmath::RunScalarFunction<double>(scalarMethod, interpValue);
}

// Returns the user defined distance clamping vector pair
std::tuple<glm::vec3, glm::vec3> Camera::SmoothCamera::GetDistanceClamping() const noexcept {
	float minsX = config->cameraDistanceClampXMin;
	float maxsX = config->cameraDistanceClampXMax;
	
	if (config->swapXClamping && shoulderSwap < 1) {
		std::swap(minsX, maxsX);
		maxsX *= -1.0f;
		minsX *= -1.0f;
	}

	return std::tie(
		glm::vec3{ minsX, config->cameraDistanceClampYMin, config->cameraDistanceClampZMin },
		glm::vec3{ maxsX, config->cameraDistanceClampYMax, config->cameraDistanceClampZMax }
	);
}

// Returns true if interpolation is allowed in the current state
bool Camera::SmoothCamera::IsInterpAllowed(PlayerCharacter* player) const noexcept {
	auto ofs = offsetState.currentGroup;
	if (currentState == GameState::CameraState::Horseback) {
		if (GameState::IsWeaponDrawn(player) && GameState::IsBowDrawn(player)) {
			return config->bowAim.interpHorseback;
		} else {
			ofs = &config->horseback;
		}
	}

	if (!GameState::IsWeaponDrawn(player)) return ofs->interp;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return ofs->interpRangedCombat;
	}
	if (GameState::IsMagicDrawn(player)) {
		return ofs->interpMagicCombat;
	}
	return ofs->interpMeleeCombat;
}

// Constructs the view matrix for the camera
glm::mat4 Camera::SmoothCamera::GetViewMatrix() const noexcept {
	// Build a rotation matrix to rotate the camera around pitch and yaw axes.
	auto m = glm::identity<glm::mat4>();
	m = glm::rotate(m, -GetCameraYawRotation(), { 0.0f, 0.0f, 1.0f }); // yaw
	m = glm::rotate(m, -GetCameraPitchRotation(), { 1.0f, 0.0f, 0.0f }); // pitch
	return m;
}
#pragma endregion

#pragma region Camera getters
glm::vec2 Camera::SmoothCamera::GetAimRotation(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) const {
	if (GameState::IsInHorseCamera(camera)) {
		// In horse camera, we need to clamp our local y axis
		const auto yaw = ref->rot.z + glm::pi<float>(); // convert to 0-tau
		const auto yawLocal = glm::mod(GetCameraYawRotation() - yaw, glm::pi<float>() * 2.0f); // confine to tau
		const auto clampedLocal = glm::clamp(yawLocal, mmath::half_pi, mmath::half_pi*3.0f); // clamp it to faceforward zone
		
		return {
			GetCameraPitchRotation(),
			yaw + clampedLocal
		};

	} else if (GameState::InPOVSlideMode()) {
		// In POV slide mode aim yaw is locked to player rotation
		return {
			GetCameraPitchRotation(),
			ref->rot.z
		};

	} else {
		return {
			GetCameraPitchRotation(),
			GetCameraYawRotation()
		};
	}
}

// Returns the camera's pitch
float Camera::SmoothCamera::GetCameraPitchRotation() const noexcept {
	return currentRotation.x;
}

// Returns the camera's yaw
float Camera::SmoothCamera::GetCameraYawRotation() const noexcept {
	return currentRotation.y;
}

// Returns the camera's current zoom level - Camera must extend ThirdPersonState
float Camera::SmoothCamera::GetCameraZoomScalar(const CorrectedPlayerCamera* camera, uint16_t cameraState) const noexcept {
	const auto state = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraStates[cameraState]);
	if (!state) return 0.0f;
	return state->cameraZoom + (Config::GetGameConfig()->fMinCurrentZoom *-1);
}

NiPointer<TESObjectREFR> Camera::SmoothCamera::GetCurrentCameraTarget(const CorrectedPlayerCamera* camera) noexcept {
	// Special case - In horse camera, just return the player
	if (GameState::IsInHorseCamera(camera)) return *g_thePlayer;

	auto ctrls = PlayerControls::GetSingleton();
	if (!ctrls) return nullptr;

	auto controlled = reinterpret_cast<tArray<UInt32>*>(&ctrls->unk150);
	auto controlledRef = controlled->count > 0 ? controlled->entries[controlled->count-1] : 0x0;

	if (controlledRef == 0) return nullptr;
	NiPointer<TESObjectREFR> ref;
	(*LookupREFRByHandle)(controlledRef, ref);
	return ref;
}

bool Camera::SmoothCamera::InLoadingScreen() const noexcept {
	return loadScreenDepth != 0;
}
#pragma endregion

void Camera::SmoothCamera::UpdateInternalRotation(CorrectedPlayerCamera* camera, NiCamera* cameraNi) noexcept {
	const auto tps = reinterpret_cast<CorrectedThirdPersonState*>(camera->cameraState);
	if (!tps) return;
	tps->UpdateRotation();

	currentQuat = glm::quat{ tps->rotation.m_fW, tps->rotation.m_fX, tps->rotation.m_fY, tps->rotation.m_fZ };
	currentRotation.x = glm::pitch(currentQuat) *-1.0f;

	// The game will do this internally to get the base yaw offset
	// Depending on more conditions, the game will add look rotation to this value
	if (currentFocusObject) {
		currentRotation.y = currentFocusObject->rot.z;

		// Horse camera, just read the quat - we get noise on the yaw axis otherwise
		if (GameState::IsInHorseCamera(camera))
			currentRotation.y = glm::roll(currentQuat) * -1.0f;
		else if (tps->freeRotationEnabled)
			currentRotation.y += tps->yaw;
		else
			currentRotation.y = tps->yaw2; // glm::roll(currentQuat) * -1.0f;
	}
}

void Camera::SmoothCamera::Render(Render::D3DContext& ctx) {
	if (InLoadingScreen()) return;
	crosshair->Render(ctx, currentPosition, currentRotation, frustum);

#ifdef WITH_CHARTS
	if (!currentFocusObject) return;
	const auto worldPos = GetCurrentCameraTargetWorldPosition(currentFocusObject, CorrectedPlayerCamera::GetSingleton());
	worldPosTargetGraph->AddPoint(0, lastWorldPosition.x);
	worldPosTargetGraph->AddPoint(1, lastWorldPosition.y);
	worldPosTargetGraph->AddPoint(2, lastWorldPosition.z);

	localSpaceGraph->AddPoint(0, lastLocalPosition.x);
	localSpaceGraph->AddPoint(1, lastLocalPosition.y);
	localSpaceGraph->AddPoint(2, lastLocalPosition.z);

	offsetPosGraph->AddPoint(0, offsetState.position.x);
	offsetPosGraph->AddPoint(1, offsetState.position.y);
	offsetPosGraph->AddPoint(2, offsetState.position.z);

	const auto ofsPos = GetCurrentCameraOffset(*g_thePlayer, CorrectedPlayerCamera::GetSingleton());
	offsetTargetPosGraph->AddPoint(0, ofsPos.x);
	offsetTargetPosGraph->AddPoint(1, ofsPos.y);
	offsetTargetPosGraph->AddPoint(2, ofsPos.z);

	rotationGraph->AddPoint(0, currentRotation.x);
	rotationGraph->AddPoint(1, currentRotation.y);

	computeTimeGraph->AddPoint(0, lastProfSnap);
	computeTimeGraph->AddPoint(1, static_cast<float>(GetFrameDelta()));

	const auto& size = ctx.windowSize;
	orthoMatrix = glm::ortho(0.0f, size.x, size.y, 0.0f);
	perFrameOrtho->Update(&orthoMatrix, 0, sizeof(glm::mat4), ctx);
	perFrameOrtho->Bind(Render::PipelineStage::Vertex, 1, ctx);

	worldPosTargetGraph->Draw(ctx);
	localSpaceGraph->Draw(ctx);
	offsetPosGraph->Draw(ctx);
	offsetTargetPosGraph->Draw(ctx);
	rotationGraph->Draw(ctx);
	computeTimeGraph->Draw(ctx);
	
	refTreeDisplay->SetPosition(size.x - 600, 0);
	if (currentFocusObject->loadedState->node) refTreeDisplay->Draw(ctx, currentFocusObject->loadedState->node);
#endif
}

// Use this method to snatch modifications done by mods that run after us 
// Called before the internal game method runs which will overwrite most of that
bool Camera::SmoothCamera::PreGameUpdate(PlayerCharacter* player, CorrectedPlayerCamera* camera,
	BSTSmartPointer<TESCameraState>& nextState)
{
	const auto state = GameState::GetCameraState(player, camera);
	if (state == GameState::CameraState::Transitioning) {
		auto cstate = reinterpret_cast<CorrectedPlayerCameraTransitionState*>(
			camera->cameraStates[CorrectedPlayerCamera::kCameraState_Transition]
		);
		auto horse = camera->cameraStates[CorrectedPlayerCamera::kCameraState_Horse];
		auto tps = camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2];
		if (cstate->fromState == horse && cstate->toState == tps) {
			// Getting off a horse, MURDER THIS STATE
			// @TODO: Write our own transition for this state - It still looks nicer with it disabled anyways though
			if (nextState.ptr) {
				InterlockedDecrement(&nextState.ptr->refCount.m_refCount);
			}

			InterlockedIncrement(&cstate->toState->refCount.m_refCount);
			nextState.ptr = cstate->toState;

			auto tpsState = reinterpret_cast<CorrectedThirdPersonState*>(
				camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2]
			);

			auto horseState = reinterpret_cast<CorrectedThirdPersonState*>(
				camera->cameraStates[CorrectedPlayerCamera::kCameraState_Horse]
			);

			// Just copy a whole wack ton of stuff
			tpsState->offsetVector = horseState->offsetVector;
			tpsState->translation = horseState->translation;
			tpsState->zoom = horseState->zoom;
			tpsState->cameraZoom = horseState->cameraZoom;
			tpsState->cameraLastZoom = horseState->cameraLastZoom;
			tpsState->fOverShoulderPosX = horseState->fOverShoulderPosX;
			tpsState->fOverShoulderCombatAddY = horseState->fOverShoulderCombatAddY;
			tpsState->fOverShoulderPosZ = horseState->fOverShoulderPosZ;
			tpsState->controllerNode->m_worldTransform = tpsState->controllerNode->m_oldWorldTransform;

			lastPosition = currentPosition = {
				tpsState->cameraNode->m_worldTransform.pos.x,
				tpsState->cameraNode->m_worldTransform.pos.y,
				tpsState->cameraNode->m_worldTransform.pos.z
			};

			return true;
		}
	}

	// Store the last actual position the game used for rendering
	auto cameraNi = reinterpret_cast<NiCamera*>(
		camera->cameraNode->m_children.m_size == 0 ?
		nullptr :
		camera->cameraNode->m_children.m_data[0]
	);
	if (cameraNi)
		gameLastActualPosition = {
			cameraNi->m_worldTransform.pos.x,
			cameraNi->m_worldTransform.pos.y,
			cameraNi->m_worldTransform.pos.z
		};

	return false;
}

// Selects the correct update method and positions the camera
void Camera::SmoothCamera::UpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera,
	BSTSmartPointer<TESCameraState>& nextState)
{
#ifdef WITH_CHARTS
	Profiler prof;
#endif

	auto cameraNode = camera->cameraNode;
	auto cameraNi = reinterpret_cast<NiCamera*>(
		camera->cameraNode->m_children.m_size == 0 ?
		nullptr :
		camera->cameraNode->m_children.m_data[0]
	);

	// Set our frustum
	if (cameraNi)
		frustum = cameraNi->m_frustum;

	// Get position that the game set the camera to
	gameInitialWorldPosition = {
		cameraNode->m_worldTransform.pos.x,
		cameraNode->m_worldTransform.pos.y,
		cameraNode->m_worldTransform.pos.z
	};

	// Perform a bit of setup to smooth out camera loading
	if (!firstFrame) {
		lastPosition = lastWorldPosition = currentPosition = gameInitialWorldPosition;
		firstFrame = true;
	}

	// Make sure the camera is following some valid reference
	auto refHandle = GetCurrentCameraTarget(camera);
	// We can pass the naked pointer around for the lifetime of the update function safely,
	// up until refHandle's destructor call
	currentFocusObject = refHandle.get();
	if (!currentFocusObject) {
		lastPosition = lastWorldPosition = currentPosition = gameInitialWorldPosition;
		return;
	}

	// Update our current crosshair selection if required
	if (Render::HasContext() && config->useWorldCrosshair) {
		crosshair->Set3DCrosshairType(config->worldCrosshairType);
	}

	// Update states & effects
	// Fetch the active camera state
	auto state = GetCurrentCameraState(player, camera);
	const auto pov = UpdateCameraPOVState(player, camera);
	const auto actionState = GetCurrentCameraActionState(player, camera);
	offsetState.currentGroup = GetOffsetForState(actionState);
	const auto currentOffset = GetCurrentCameraOffset(player, camera);
	const auto curTime = CurTime();

	// Don't continue to update transition states if we aren't running update code
	bool shouldRun = false;
	switch (state) {
		case GameState::CameraState::ThirdPerson:
		case GameState::CameraState::ThirdPersonCombat:
		case GameState::CameraState::Horseback:
			shouldRun = true;
			break;
		default: break;
	}

	// Save the camera position
	lastPosition = currentPosition;

	if (config->disableDuringDialog && dialogMenuOpen) {
		lastPosition = lastWorldPosition = currentPosition = gameInitialWorldPosition;
	} else if (shouldRun) {
		// Update transition states
		mmath::UpdateTransitionState<glm::vec2, OffsetTransition>(
			curTime,
			config->enableOffsetInterpolation,
			config->offsetInterpDurationSecs,
			config->offsetScalar,
			offsetTransitionState,
			{ currentOffset.x, currentOffset.z }
		);

		if (!povWasPressed) {
			mmath::UpdateTransitionState<float, ZoomTransition>(
				curTime,
				config->enableZoomInterpolation,
				config->zoomInterpDurationSecs,
				config->zoomScalar,
				zoomTransitionState,
				currentOffset.y
			);
		} else {
			zoomTransitionState.lastPosition = zoomTransitionState.currentPosition =
				zoomTransitionState.targetPosition = currentOffset.y;
		}
		offsetState.position = {
			offsetTransitionState.currentPosition.x,
			zoomTransitionState.currentPosition,
			offsetTransitionState.currentPosition.y
		};

		switch (state) {
			case GameState::CameraState::ThirdPerson: {
				UpdateInternalRotation(camera, cameraNi);
				cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson))->Update(player, currentFocusObject, camera);
				break;
			}
			case GameState::CameraState::ThirdPersonCombat: {
				UpdateInternalRotation(camera, cameraNi);
				cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat))->Update(player, currentFocusObject, camera);
				break;
			}
			case GameState::CameraState::Horseback: {
				UpdateInternalRotation(camera, cameraNi);
				cameraStates.at(static_cast<size_t>(GameState::CameraState::Horseback))->Update(player, currentFocusObject, camera);
				break;
			}

			// Here just for my own reference that these are unused (for now)
			case GameState::CameraState::FirstPerson:
			case GameState::CameraState::KillMove:
			case GameState::CameraState::Tweening:
			case GameState::CameraState::Transitioning:
			case GameState::CameraState::UsingObject:
			case GameState::CameraState::Vanity:
			case GameState::CameraState::Free:
			case GameState::CameraState::IronSights:
			case GameState::CameraState::Furniture:
			case GameState::CameraState::Bleedout:
			case GameState::CameraState::Dragon:
			case GameState::CameraState::Unknown:
			default: {
				break;
			}
		}
	}

	povWasPressed = false;

#ifdef WITH_CHARTS
	lastProfSnap = prof.Snap();
#endif
}