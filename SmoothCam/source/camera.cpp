#include "camera.h"
#include "crosshair.h"

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
		stateOverlay = std::make_unique<Render::StateOverlay>(600, 128, this, Render::GetContext());

		worldPosTargetGraph = std::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		offsetPosGraph = std::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		offsetTargetPosGraph = std::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		localSpaceGraph = std::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		rotationGraph = std::make_unique<Render::LineGraph>(2, 128, 600, 128, Render::GetContext());
		tpsRotationGraph = std::make_unique<Render::LineGraph>(4, 128, 600, 128, Render::GetContext());
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

		tpsRotationGraph->SetLineColor(0, xColor);
		tpsRotationGraph->SetLineColor(1, yColor);
		tpsRotationGraph->SetLineColor(2, zColor);
		tpsRotationGraph->SetLineColor(3, { 1.0f, 1.0f, 0.0f, 1.0f });
		tpsRotationGraph->SetPosition(0, 640);
		tpsRotationGraph->SetName(L"TPS Rotation (Yaw1, Yaw2, Yaw), camera->lookYaw");

		computeTimeGraph->SetLineColor(0, { 1.0f, 0.266f, 0.984f, 1.0f });
		computeTimeGraph->SetLineColor(1, { 1.0f, 0.541f, 0.218f, 1.0f });
		computeTimeGraph->SetName(L"Compute Time <pink> (Camera::Update()), Frame Time <orange>, seconds");
		computeTimeGraph->SetPosition(0, 768);

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
	auto code = ev->keyMask;
	if (code <= 0x6 && ev->deviceType == kDeviceType_Mouse) {
		code += 0x100;
	}

	if (config->shoulderSwapKey >= 0 && config->shoulderSwapKey == code && ev->timer <= 0.000001f)
		shoulderSwap = shoulderSwap == 1 ? -1 : 1;
}

void Camera::SmoothCamera::OnMenuOpenClose(MenuID id, const MenuOpenCloseEvent* const ev) noexcept {
	switch (id) {
		case MenuID::DialogMenu: {
			if (dialogMenuOpen && !ev->opening) wasDialogMenuOpen = true;
			dialogMenuOpen = ev->opening;

			if (dialogMenuOpen && config->compatACC) {
				stateCopyData.accPitch = (*g_thePlayer)->rot.x;
			}
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

// Updates our POV state to the true value the game expects for each state
const bool Camera::SmoothCamera::UpdateCameraPOVState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept {
	povIsThird = GameState::IsInAutoVanityCamera(camera) ||
		GameState::IsInCameraTransition(camera) || GameState::IsInUsingObjectCamera(camera) || GameState::IsInKillMove(camera) ||
		GameState::IsInBleedoutCamera(camera) || GameState::IsInFurnitureCamera(camera) ||
		GameState::IsInHorseCamera(player, camera) || GameState::IsInDragonCamera(camera) || GameState::IsThirdPerson(player, camera);
	return povIsThird;
}

#pragma region Camera state updates
const GameState::CameraState Camera::SmoothCamera::GetCurrentCameraState()  const noexcept {
	return currentState;
}

const Camera::CameraActionState Camera::SmoothCamera::GetCurrentCameraActionState() const noexcept {
	return currentActionState;
}

// Returns the current camera state for use in selecting an update method
const GameState::CameraState Camera::SmoothCamera::UpdateCurrentCameraState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) {
	GameState::CameraState newState = GameState::CameraState::Unknown;
	const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraState);
	const auto minZoom = Config::GetGameConfig()->fMinCurrentZoom;

	// Improved camera - Sitting
	if (config->compatIC && GameState::IC_InFirstPersonState(player, camera) &&
		!GameState::IsInHorseCamera(player, camera) && !GameState::IsInDragonCamera(camera) &&
		GameState::IsSitting(player) && !GameState::IsSleeping(player))
	{
		newState = GameState::CameraState::FirstPerson;
		goto applyStateTransition; // SUE ME
	}

	newState = GameState::GetCameraState(player, camera);
	
	// Improved camera - Horse and dragon
	if (newState == GameState::CameraState::Horseback && config->compatIC && !config->compatIFPV) {
		if (tps) {
			if (GameState::IC_InFirstPersonState(player, camera))
				newState = GameState::CameraState::FirstPerson;
			else
				newState = GameState::CameraState::Horseback;

		} else {
			newState = GameState::CameraState::Horseback;
		}
	} else if (newState == GameState::CameraState::Dragon && config->compatIC && !config->compatIFPV) {
		if (tps) {
			if (GameState::IC_InFirstPersonState(player, camera))
				newState = GameState::CameraState::FirstPerson;
			else
				newState = GameState::CameraState::Dragon;

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
const Camera::CameraActionState Camera::SmoothCamera::UpdateCurrentCameraActionState(const PlayerCharacter* player,
	const CorrectedPlayerCamera* camera) noexcept
{
	CameraActionState newState = CameraActionState::Unknown;

	if (GameState::IsVampireLord(player)) {
		newState = CameraActionState::VampireLord;
	} else if (GameState::IsWerewolf(player)) {
		newState = CameraActionState::Werewolf;
	} else if (camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Horse]) {
		// Improved camera compat
		if (!povIsThird) {
			newState = CameraActionState::FirstPersonHorseback;
		} else if (GameState::IsDisMountingHorse(player)) {
			newState = CameraActionState::DisMounting;
		} else {
			newState = CameraActionState::Standing;
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
	auto& oldCameraState = cameraStates.at(static_cast<size_t>(oldState));
	auto& newCameraState = cameraStates.at(static_cast<size_t>(newState));

	switch (oldState) {
		case GameState::CameraState::UsingObject: // We also want to do this for this case
		case GameState::CameraState::FirstPerson: {
			// We want to invalidate any interpolation state when exiting first-person
			// Set both current and last positions to the target third-person position
			const auto pov = UpdateCameraPOVState(player, camera);
			const auto actionState = UpdateCurrentCameraActionState(player, camera);
			offsetState.currentGroup = GetOffsetForState(actionState);

			auto ofs = GetCurrentCameraOffset(player, camera);
			offsetTransitionState.currentPosition = offsetTransitionState.lastPosition = {
				ofs.x, ofs.z
			};
			zoomTransitionState.currentPosition = zoomTransitionState.lastPosition = ofs.y;
			fovTransitionState.currentPosition = fovTransitionState.lastPosition = ofs.w;

			GetCameraGoalPosition(camera, currentPosition.world, currentPosition.local);
			lastPosition = currentPosition;
			break;
		}
		case GameState::CameraState::ThirdPerson: {
			oldCameraState->OnEnd(player, currentFocusObject, camera, newCameraState.get());
			break;
		}
		case GameState::CameraState::ThirdPersonCombat: {
			oldCameraState->OnEnd(player, currentFocusObject, camera, newCameraState.get());
			break;
		}
		case GameState::CameraState::Horseback: {
			oldCameraState->OnEnd(player, currentFocusObject, camera, newCameraState.get());

			if (newState == GameState::CameraState::Tweening) {
				// The game clears horseYaw to 0 when going from tween back to horseback, which results in a rotation jump
				// once the horse starts moving again. We can store the horse yaw value here to restore later.
				stateCopyData.horseYaw = reinterpret_cast<CorrectedHorseCameraState*>(
					camera->cameraStates[CorrectedPlayerCamera::kCameraState_Horse]
				)->horseYaw;

			} else if (newState == GameState::CameraState::ThirdPerson) {
				// We need to smooth out the camera position
				const auto pov = UpdateCameraPOVState(player, camera);
				const auto actionState = UpdateCurrentCameraActionState(player, camera);
				offsetState.currentGroup = GetOffsetForState(actionState);

				auto ofs = GetCurrentCameraOffset(player, camera);
				offsetTransitionState.currentPosition = offsetTransitionState.lastPosition = {
					ofs.x, ofs.z
				};
				zoomTransitionState.currentPosition = zoomTransitionState.lastPosition = ofs.y;
				fovTransitionState.currentPosition = fovTransitionState.lastPosition = ofs.w;

				GetCameraGoalPosition(camera, currentPosition.world, currentPosition.local);
				lastPosition = currentPosition;
			}

			break;
		}
		case GameState::CameraState::Tweening:
		case GameState::CameraState::Transitioning: {
			// Don't copy positions from these states
			break;
		}
		default:
			// Store the position for smoothing in the new state
			lastPosition.world = currentPosition.world = glm::vec3{
				camera->cameraNode->m_worldTransform.pos.x,
				camera->cameraNode->m_worldTransform.pos.y,
				camera->cameraNode->m_worldTransform.pos.z
			};
			break;
	}

	switch (newState) {
		case GameState::CameraState::ThirdPerson: {
			newCameraState->OnBegin(player, currentFocusObject, camera, oldCameraState.get());
			break;
		}
		case GameState::CameraState::ThirdPersonCombat: {
			newCameraState->OnBegin(player, currentFocusObject, camera, oldCameraState.get());
			break;
		}
		case GameState::CameraState::Horseback: {
			newCameraState->OnBegin(player, currentFocusObject, camera, oldCameraState.get());

			if (oldState == GameState::CameraState::Tweening) {
				reinterpret_cast<CorrectedHorseCameraState*>(
					camera->cameraStates[CorrectedPlayerCamera::kCameraState_Horse]
				)->horseYaw = stateCopyData.horseYaw;
			}

			break;
		}
		case GameState::CameraState::Free: {
			// Forward our position to the free cam state
			auto state = reinterpret_cast<FreeCameraState*>(camera->cameraState);
			state->unk30[0] = lastPosition.world.x;
			state->unk30[1] = lastPosition.world.y;
			state->unk30[2] = lastPosition.world.z;

			const auto quat = rotation.ToNiQuat();

			//FUN_140848880((longlong)ppuVar3,local_18);
			typedef void(__fastcall* UpdateFreeCamTransform)(FreeCameraState*, const NiQuaternion&);
			Offsets::Get<UpdateFreeCamTransform>(49816)(state, quat);
		}
		[[fallthrough]];
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
		case CameraActionState::VampireLord: {
			return &config->vampireLord;
		}
		case CameraActionState::Werewolf: {
			return &config->werewolf;
		}
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

float Camera::SmoothCamera::GetActiveWeaponStateZoomOffset(const PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept {
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
float Camera::SmoothCamera::GetActiveWeaponStateUpOffset(const PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept {
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
float Camera::SmoothCamera::GetActiveWeaponStateSideOffset(const PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept {
	if (!GameState::IsWeaponDrawn(player)) return group->sideOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedSideOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicSideOffset;
	}
	return group->combatMeleeSideOffset;
}

// Selects the right offset from an offset group for the player's weapon state
float Camera::SmoothCamera::GetActiveWeaponStateFOVOffset(const PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept {
	if (!GameState::IsWeaponDrawn(player)) return group->fovOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedFOVOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicFOVOffset;
	}
	return group->combatMeleeFOVOffset;
}


float Camera::SmoothCamera::GetCurrentCameraZoomOffset(const PlayerCharacter* player) const noexcept {
	switch (currentState) {
		case GameState::CameraState::Horseback: {
			if (GameState::IsBowDrawn(player)) {
				return config->bowAim.horseZoomOffset;
			} else {
				return GetActiveWeaponStateZoomOffset(player, &config->horseback);
			}
		}
		default:
			break;
	}

	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->zoomOffset;
		}
		case CameraActionState::Aiming: {
			if (GameState::IsSneaking(player))
				return config->bowAim.combatMeleeZoomOffset;
			return offsetState.currentGroup->zoomOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::VampireLord:
		case CameraActionState::Werewolf:
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
float Camera::SmoothCamera::GetCurrentCameraHeight(const PlayerCharacter* player) const noexcept {
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
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->upOffset;
		}
		case CameraActionState::Aiming: {
			if (GameState::IsSneaking(player))
				return config->bowAim.combatMeleeUpOffset;
			return offsetState.currentGroup->upOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::VampireLord:
		case CameraActionState::Werewolf:
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
float Camera::SmoothCamera::GetCurrentCameraSideOffset(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept {
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
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->sideOffset * shoulderSwap;
		}
		case CameraActionState::Aiming: {
			if (GameState::IsSneaking(player))
				return config->bowAim.combatMeleeSideOffset * shoulderSwap;
			return offsetState.currentGroup->sideOffset * shoulderSwap;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::VampireLord:
		case CameraActionState::Werewolf:
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

float Camera::SmoothCamera::GetCurrentCameraFOVOffset(const PlayerCharacter* player) const noexcept {
	switch (currentState) {
		case GameState::CameraState::Horseback: {
			if (GameState::IsBowDrawn(player)) {
				return config->bowAim.horseFOVOffset;
			} else {
				return GetActiveWeaponStateFOVOffset(player, &config->horseback);
			}
		}
		default:
			break;
	}

	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->fovOffset;
		}
		case CameraActionState::Aiming:{
			if (GameState::IsSneaking(player))
				return config->bowAim.combatMeleeFOVOffset;
			return offsetState.currentGroup->fovOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::VampireLord:
		case CameraActionState::Werewolf:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateFOVOffset(player, offsetState.currentGroup);
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

// Returns the full local-space camera offset for the current player state, FOV is packed in .w
glm::vec4 Camera::SmoothCamera::GetCurrentCameraOffset(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept {
	return {
		GetCurrentCameraSideOffset(player, camera),
		GetCurrentCameraDistance(camera) + GetCurrentCameraZoomOffset(player),
		GetCurrentCameraHeight(player),
		GetCurrentCameraFOVOffset(player)
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

void Camera::SmoothCamera::GetCameraGoalPosition(const CorrectedPlayerCamera* camera, glm::vec3& world, glm::vec3& local) {
	const auto cameraLocal = offsetState.position;
	auto translated = rotation.ToRotationMatrix() * glm::vec4(
		cameraLocal.x,
		cameraLocal.y,
		0.0f,
		1.0f
	);
	translated.z += cameraLocal.z;

	local = static_cast<glm::vec3>(translated);
	world = 
		GetCurrentCameraTargetWorldPosition(currentFocusObject, camera) +
		local;
}

void Camera::SmoothCamera::SetPosition(const glm::vec3& pos, const CorrectedPlayerCamera* camera) noexcept {
	auto cameraNode = camera->cameraNode;
	currentPosition.SetWorldPosition(pos);

#ifdef _DEBUG
	if (!mmath::IsValid(currentPosition.world)) {
		__debugbreak();
		// Oops, go ahead and clear both
		lastPosition.SetWorldPosition(gameLastActualPosition);
		currentPosition.SetWorldPosition(gameLastActualPosition);
		return;
	}
#endif
	cameraNode->m_localTransform.pos = cameraNode->m_worldTransform.pos = cameraNi->m_worldTransform.pos =
		currentPosition.ToNiPoint3();

	if (currentState == GameState::CameraState::ThirdPerson || currentState == GameState::CameraState::ThirdPersonCombat) {
		auto state = reinterpret_cast<CorrectedThirdPersonState*>(camera->cameraState);
		state->translation = currentPosition.ToNiPoint3();
	}

	// Update world to screen matrices
	UpdateInternalWorldToScreenMatrix(camera);
}

void Camera::SmoothCamera::UpdateInternalWorldToScreenMatrix(const CorrectedPlayerCamera* camera) noexcept {
	// Run the THT to get a world to scaleform matrix
	auto lastRotation = cameraNi->m_worldTransform.rot;
	cameraNi->m_worldTransform.rot = rotation.THT();
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
		const double delta = glm::max(GameTime::GetFrameDelta(), minZero);
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
bool Camera::SmoothCamera::IsInterpAllowed(const PlayerCharacter* player) const noexcept {
	auto ofs = offsetState.currentGroup;
	if (currentState == GameState::CameraState::Horseback) {
		if (GameState::IsBowDrawn(player)) {
			return config->bowAim.interpHorseback;
		} else {
			ofs = &config->horseback;
		}
	}

	if (GameState::IsSneaking(player)) {
		if (GameState::IsBowDrawn(player)) {
			return config->bowAim.interpMeleeCombat;
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
#pragma endregion

#pragma region Camera getters
glm::vec2 Camera::SmoothCamera::GetAimRotation(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) const {
	if (GameState::IsInHorseCamera(ref, camera)) {
		// In horse camera, we need to clamp our local y axis
		const auto yaw = ref->rot.z + glm::pi<float>(); // convert to 0-tau
		const auto yawLocal = glm::mod(rotation.euler.y - yaw, glm::pi<float>() * 2.0f); // confine to tau
		const auto clampedLocal = glm::clamp(yawLocal, mmath::half_pi, mmath::half_pi*3.0f); // clamp it to faceforward zone
		
		return {
			rotation.euler.x,
			yaw + clampedLocal
		};
	} else {
		const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2]);
		if (tps && tps->freeRotationEnabled) {
			// In free rotation mode aim yaw is locked to player rotation
			return {
				rotation.euler.x,
				ref->rot.z
			};
		}
	}

	return rotation.euler;
}

const mmath::Rotation& Camera::SmoothCamera::GetCameraRotation() const noexcept {
	return rotation;
}

// Returns the camera's current zoom level - Camera must extend ThirdPersonState
float Camera::SmoothCamera::GetCameraZoomScalar(const CorrectedPlayerCamera* camera, uint16_t cameraState) const noexcept {
	const auto state = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraStates[cameraState]);
	if (!state) return 0.0f;
	return state->cameraZoom + (Config::GetGameConfig()->fMinCurrentZoom *-1);
}

NiPointer<Actor> Camera::SmoothCamera::GetCurrentCameraTarget(const CorrectedPlayerCamera* camera) noexcept {
	auto ctrls = PlayerControls::GetSingleton();
	if (!ctrls) return *g_thePlayer;

	auto controlled = reinterpret_cast<tArray<UInt32>*>(&ctrls->unk150);
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

bool Camera::SmoothCamera::InLoadingScreen() const noexcept {
	return loadScreenDepth != 0;
}
#pragma endregion

void Camera::SmoothCamera::UpdateInternalRotation(CorrectedPlayerCamera* camera) noexcept {
	const auto tps = reinterpret_cast<CorrectedThirdPersonState*>(camera->cameraState);
	if (!tps) return;

	if (config->compatACC && wasDialogMenuOpen) {
		auto pl = *g_thePlayer;
		if (pl) {
			tps->rotation = rotation.ToNiQuat();
			tps->yaw1 = tps->yaw2 = rotation.euler.y;
			camera->lookYaw = rotation.euler.y;
			pl->rot.x = stateCopyData.accPitch;
		}

	} else {
		// Alright, just lie and force the game to compute yaw for us
		// We get some jitter when things like magic change our character, not sure why yet @TODO
		auto last = tps->freeRotationEnabled;
		auto pl = *g_thePlayer;
		tps->freeRotationEnabled = true;
		tps->UpdateRotation();
			rotation.SetQuaternion(tps->rotation);
		tps->freeRotationEnabled = last;
		tps->UpdateRotation();
	}
}

void Camera::SmoothCamera::SetFOVOffset(float fov) noexcept {
	// This is an offset value applied to any call to SetFOV.
	// Used internally in the game by things like zooming with the bow.
	// Appears to be set by the game every frame so we can just add our offset on top like this.
	const auto baseValue = CorrectedPlayerCamera::GetSingleton()->worldFOV;
	const auto currentOffset = *Offsets::Get<float*>(527997);

	// We never want the FOV to go negative or past 180, so clamp it to a sane-ish range
	// (10-170)
	const auto actualFov = baseValue + currentOffset;
	constexpr auto fovMax = 170.0f;
	constexpr auto fovMin = 10.0f;

	// Break early if we are already past our limit
	if (actualFov >= fovMax) return;
	if (actualFov <= fovMin) return;

	// Compute the maximal range we can offset before hitting the limit
	const auto fullOffset = currentOffset + fov;
	const auto f = baseValue + fullOffset;
	auto finalOffset = fov;

	if (f > fovMax) {
		auto delta = f - fovMax;
		finalOffset -= delta;
	} else if (f < fovMin) {
		auto delta = fovMin - f;
		finalOffset += delta;
	}

	*Offsets::Get<float*>(527997) += finalOffset;
}

NiPointer<NiCamera> Camera::SmoothCamera::GetNiCamera(CorrectedPlayerCamera* camera) const noexcept {
	// Do other things parent stuff to the camera node? Better safe than sorry I guess
	if (camera->cameraNode->m_children.m_size == 0) return nullptr;
	for (auto i = 0; i < camera->cameraNode->m_children.m_size; i++) {
		auto entry = camera->cameraNode->m_children.m_data[i];
		auto asCamera = DYNAMIC_CAST(entry, NiAVObject, NiCamera);
		if (asCamera) return asCamera;
	}
	return nullptr;
}

void Camera::SmoothCamera::Render(Render::D3DContext& ctx) {
	if (InLoadingScreen()) return;
	crosshair->Render(ctx, currentPosition.world, rotation.euler, frustum);

#ifdef WITH_CHARTS
	if (!currentFocusObject) return;

	if (GetAsyncKeyState(VK_UP)) {
		if (!dbgKeyDown) {
			switch (curDebugMode) {
				case DisplayMode::None:
					curDebugMode = DisplayMode::Graphs;
					break;
				case DisplayMode::Graphs:
					curDebugMode = DisplayMode::NodeTree;
					break;
				case DisplayMode::NodeTree:
					curDebugMode = DisplayMode::StateOverlay;
					break;
				case DisplayMode::StateOverlay:
					curDebugMode = DisplayMode::None;
					break;
			}
			dbgKeyDown = true;
		}
	} else {
		dbgKeyDown = false;
	}

	const auto& size = ctx.windowSize;
	orthoMatrix = glm::ortho(0.0f, size.x, size.y, 0.0f);
	perFrameOrtho->Update(&orthoMatrix, 0, sizeof(glm::mat4), ctx);
	perFrameOrtho->Bind(Render::PipelineStage::Vertex, 1, ctx);

	switch (curDebugMode) {
		case DisplayMode::None:
			break;

		case DisplayMode::Graphs: {
			const auto worldPos = GetCurrentCameraTargetWorldPosition(currentFocusObject, CorrectedPlayerCamera::GetSingleton());
			worldPosTargetGraph->AddPoint(0, lastPosition.world.x);
			worldPosTargetGraph->AddPoint(1, lastPosition.world.y);
			worldPosTargetGraph->AddPoint(2, lastPosition.world.z);

			localSpaceGraph->AddPoint(0, lastPosition.local.x);
			localSpaceGraph->AddPoint(1, lastPosition.local.y);
			localSpaceGraph->AddPoint(2, lastPosition.local.z);

			offsetPosGraph->AddPoint(0, offsetState.position.x);
			offsetPosGraph->AddPoint(1, offsetState.position.y);
			offsetPosGraph->AddPoint(2, offsetState.position.z);

			const auto ofsPos = GetCurrentCameraOffset(*g_thePlayer, CorrectedPlayerCamera::GetSingleton());
			offsetTargetPosGraph->AddPoint(0, ofsPos.x);
			offsetTargetPosGraph->AddPoint(1, ofsPos.y);
			offsetTargetPosGraph->AddPoint(2, ofsPos.z);

			rotationGraph->AddPoint(0, rotation.euler.x);
			rotationGraph->AddPoint(1, rotation.euler.y);

			if (GameState::IsThirdPerson(currentFocusObject, CorrectedPlayerCamera::GetSingleton())) {
				auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(CorrectedPlayerCamera::GetSingleton()->cameraState);
				tpsRotationGraph->AddPoint(0, tps->yaw1);
				tpsRotationGraph->AddPoint(1, tps->yaw2);
				tpsRotationGraph->AddPoint(2, tps->yaw);
				tpsRotationGraph->AddPoint(3, CorrectedPlayerCamera::GetSingleton()->lookYaw);
			}

			computeTimeGraph->AddPoint(0, lastProfSnap);
			computeTimeGraph->AddPoint(1, static_cast<float>(GameTime::GetFrameDelta()));

			worldPosTargetGraph->Draw(ctx);
			localSpaceGraph->Draw(ctx);
			offsetPosGraph->Draw(ctx);
			offsetTargetPosGraph->Draw(ctx);
			rotationGraph->Draw(ctx);
			tpsRotationGraph->Draw(ctx);
			computeTimeGraph->Draw(ctx);

			break;
		}
		case DisplayMode::NodeTree: {
			refTreeDisplay->SetPosition(0, 0);
			refTreeDisplay->SetSize(size.x, size.y);
			if (currentFocusObject->loadedState->node)
				refTreeDisplay->Draw(ctx, currentFocusObject->loadedState->node);
			break;
		}
		case DisplayMode::StateOverlay: {
			stateOverlay->SetPosition((size.x / 2) - 300, size.y - 600);
			stateOverlay->SetSize(600, 400);
			stateOverlay->Draw(offsetState.currentGroup, ctx);
			break;
		}
	}
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
			if (!(config->compatIFPV && GameState::IFPV_InFirstPersonState(player, camera)) &&
				!(config->compatIC && GameState::IC_InFirstPersonState(player, camera)))
			{
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

				currentPosition.SetWorldPosition(tpsState->cameraNode->m_worldTransform.pos);
				currentPosition.SetLocalPosition(glm::vec3{ 0.0f, 0.0f, 0.0f });
				lastPosition = currentPosition;

				return true;
			}
		}
	}

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
void Camera::SmoothCamera::UpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera,
	BSTSmartPointer<TESCameraState>& nextState) {
#ifdef WITH_CHARTS
	Profiler prof;
#endif

	// Invalidate while we are in a loading screen state
	if (InLoadingScreen())
		crosshair->InvalidateEnablementCache();

	// If we don't have an NiCamera, something else is likely very wrong
	if (!cameraNi) return;

	// Make sure the camera is following some valid reference
	auto refHandle = GetCurrentCameraTarget(camera);
	// We can pass the naked pointer around for the lifetime of the update function safely,
	// up until refHandle's destructor call
	currentFocusObject = refHandle.get();
	if (!currentFocusObject) {
		lastPosition.SetWorldPosition(gameLastActualPosition);
		currentPosition.SetWorldPosition(gameLastActualPosition);

		cameraNi = nullptr;
		return;
	}

	// Update our current crosshair selection if required
	if (Render::HasContext() && config->useWorldCrosshair) {
		crosshair->Set3DCrosshairType(config->worldCrosshairType);
	}

	// Update states & effects
	// Fetch the active camera state
	auto state = UpdateCurrentCameraState(player, camera);
	const auto pov = UpdateCameraPOVState(player, camera);
	const auto actionState = UpdateCurrentCameraActionState(player, camera);
	offsetState.currentGroup = GetOffsetForState(actionState);
	const auto currentOffset = GetCurrentCameraOffset(player, camera);
	const auto curTime = GameTime::CurTime();

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

	// And set our current reference position
	if (wasDialogMenuOpen && config->compatACC) {
		offsetTransitionState.currentPosition = offsetTransitionState.lastPosition = {
			currentOffset.x, currentOffset.z
		};
		zoomTransitionState.currentPosition = zoomTransitionState.lastPosition = currentOffset.y;
		fovTransitionState.currentPosition = fovTransitionState.lastPosition = currentOffset.w;

		GetCameraGoalPosition(camera, currentPosition.world, currentPosition.local);
		lastPosition = currentPosition;
	} else {
		currentPosition.SetRef(currentFocusObject->pos, currentFocusObject->rot);
	}

	// Set our frustum
	frustum = cameraNi->m_frustum;

	// Perform a bit of setup to smooth out camera loading
	if (!firstFrame) {
		GetCameraGoalPosition(camera, currentPosition.world, currentPosition.local);
		lastPosition = currentPosition;
		firstFrame = true;
	}

	// Save our last position
	lastPosition = currentPosition;

	if (!shouldRun || (config->compatACC && dialogMenuOpen)) {
		SetFOVOffset(0.0f);

	} else {
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

		mmath::UpdateTransitionState<float, ZoomTransition>(
			curTime,
			config->enableFOVInterpolation,
			config->fovInterpDurationSecs,
			config->fovScalar,
			fovTransitionState,
			currentOffset.w
		);

		offsetState.position = {
			offsetTransitionState.currentPosition.x,
			zoomTransitionState.currentPosition,
			offsetTransitionState.currentPosition.y
		};
		offsetState.fov = fovTransitionState.currentPosition;
		SetFOVOffset(offsetState.fov);

		// Now run the active camera state
		switch (state) {
			case GameState::CameraState::ThirdPerson: {
				UpdateInternalRotation(camera);
				cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson))->Update(player, currentFocusObject, camera);
				break;
			}
			case GameState::CameraState::ThirdPersonCombat: {
				UpdateInternalRotation(camera);
				cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat))->Update(player, currentFocusObject, camera);
				break;
			}
			case GameState::CameraState::Horseback: {
				UpdateInternalRotation(camera);
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
	wasDialogMenuOpen = false;
	cameraNi = nullptr;

#ifdef WITH_CHARTS
	lastProfSnap = prof.Snap();
#endif
}