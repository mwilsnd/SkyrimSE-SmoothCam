#include "pch.h"
#include "camera.h"

using namespace mmath;

Camera::SmoothCamera::SmoothCamera() noexcept : config(Config::GetCurrentConfig()) {
	cameraStates[static_cast<size_t>(CameraState::ThirdPerson)] =
		std::move(std::make_unique<State::ThirdpersonState>(this));
	cameraStates[static_cast<size_t>(CameraState::ThirdPersonCombat)] =
		std::move(std::make_unique<State::ThirdpersonCombatState>(this));
	cameraStates[static_cast<size_t>(CameraState::Horseback)] =
		std::move(std::make_unique<State::ThirdpersonHorseState>(this));
}

// Called when the player toggles the POV
void Camera::SmoothCamera::OnTogglePOV(const ButtonEvent* ev) noexcept {
	povIsThird = !povIsThird;
	povWasPressed = true;
}

double Camera::SmoothCamera::GetTime() const noexcept {
	LARGE_INTEGER f, i;
	if (QueryPerformanceCounter(&i) && QueryPerformanceFrequency(&f)) {
		auto frequency = 1.0 / static_cast<double>(f.QuadPart);
		return static_cast<double>(i.QuadPart) * frequency;
	}
	return 0.0;
}

float Camera::SmoothCamera::GetFrameDelta() const noexcept {
	return glm::max(static_cast<float>(curFrame - lastFrame), 0.0001f); // Prevent any "accidents" during the inital frame
}

#pragma region Player flags
// Returns the bits for player->actorState->flags04 which appear to convey movement info
const std::bitset<32> Camera::SmoothCamera::GetPlayerMovementBits(const PlayerCharacter* player) const noexcept {
	const auto bits = std::bitset<32>(static_cast<uint64_t>(player->actorState.flags04));
#ifdef _DEBUG
	// Just to see what actions end up setting these unknown bits
	for (int i = 0; i < 32; i++) {
		if (bits[i]) {
			auto it = knownMovementBits.find(i);
			if (it == knownMovementBits.end()) {
				__debugbreak();
			}
		}
	}
#endif
	return bits;
}

// Returns the bits for player->actorState->flags08 which appear to convey action info
const std::bitset<32> Camera::SmoothCamera::GetPlayerActionBits(const PlayerCharacter* player) const noexcept {
	const auto bits = std::bitset<32>(static_cast<uint64_t>(player->actorState.flags08));
#ifdef _DEBUG
	// Just to see what actions end up setting these unknown bits
	for (int i = 0; i < 32; i++) {
		if (bits[i]) {
			auto it = knownActionBits.find(i);
			if (it == knownActionBits.end()) {
				__debugbreak();
			}
		}
	}
#endif
	return bits;
}
#pragma endregion

#pragma region Camera state detection
// Returns true if the player is in first person
const bool Camera::SmoothCamera::IsFirstPerson(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[camera->kCameraState_FirstPerson];
}

// Returns true if the player is in third person
const bool Camera::SmoothCamera::IsThirdPerson(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[camera->kCameraState_ThirdPerson2];
}

// Returns true if the player has a weapon drawn and in third person
const bool Camera::SmoothCamera::IsThirdPersonCombat(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept {
	return IsThirdPerson(camera) && IsWeaponDrawn(player);
}

// Returns true if a kill move is playing
const bool Camera::SmoothCamera::IsInKillMove(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_VATS];
}

// Returns true if the camera is tweening
const bool Camera::SmoothCamera::IsInTweenCamera(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_TweenMenu];
}

// Returns true if the camera is transitioning
const bool Camera::SmoothCamera::IsInCameraTransition(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Transition];
}

// Returns true if the player is using an object
const bool Camera::SmoothCamera::IsInUsingObjectCamera(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_ThirdPerson1];
}

// Returns true if the camera is in auto vanity mode
const bool Camera::SmoothCamera::IsInAutoVanityCamera(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_AutoVanity];
}

// Returns true if the camera is in free mode
const bool Camera::SmoothCamera::IsInFreeCamera(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Free];
}

// Returns true if the camera is in aiming mode
const bool Camera::SmoothCamera::IsInAimingCamera(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_IronSights];
}

// Returns true if the camera is in furniture mode
const bool Camera::SmoothCamera::IsInFurnitureCamera(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Furniture];
}

// Returns true if the player is riding a horse
const bool Camera::SmoothCamera::IsInHorseCamera(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Horse];
}

// Returns true if the player is bleeding out
const bool Camera::SmoothCamera::IsInBleedoutCamera(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Bleedout];
}

// Returns true if the player is riding a dragon
const bool Camera::SmoothCamera::IsInDragonCamera(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Dragon];
}
#pragma endregion

// Updates our POV state to the true value the game expects for each state
const bool Camera::SmoothCamera::UpdateCameraPOVState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept {
	const auto zoom = reinterpret_cast<const CorrectedThirdPersonState*>(camera)->cameraZoom;
	const auto lzoom = reinterpret_cast<const CorrectedThirdPersonState*>(camera)->cameraLastZoom;
	povIsThird = zoom == 0.0f || IsInAutoVanityCamera(camera) || IsInTweenCamera(camera) ||
		IsInCameraTransition(camera) || IsInUsingObjectCamera(camera) || IsInKillMove(camera) ||
		IsInBleedoutCamera(camera) || IsInFurnitureCamera(camera) || IsInHorseCamera(camera) ||
		IsInDragonCamera(camera) || IsThirdPerson(camera);
	return povIsThird;
}

#pragma region Player action states
const bool Camera::SmoothCamera::IsWeaponDrawn(const PlayerCharacter* player) const noexcept {
	const auto bits = GetPlayerActionBits(player);
	return bits[5] && bits[6];
}

// Returns true if the player is sneaking
const bool Camera::SmoothCamera::IsSneaking(const PlayerCharacter* player) const noexcept {
	const auto movementBits = GetPlayerMovementBits(player);
	return movementBits[9];
}

// Returns true if the player is sprinting
const bool Camera::SmoothCamera::IsSprinting(const PlayerCharacter* player) const noexcept {
	const auto movementBits = GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[8];
}

// Returns true if the player is running
const bool Camera::SmoothCamera::IsRunning(const PlayerCharacter* player) const noexcept {
	const auto movementBits = GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[7];
}

// Returns true if the player is swimming
const bool Camera::SmoothCamera::IsSwimming(const PlayerCharacter* player) const noexcept {
	const auto movementBits = GetPlayerMovementBits(player);
	return movementBits[10];
}

// Returns true if the player is walking
const bool Camera::SmoothCamera::IsWalking(const PlayerCharacter* player) const noexcept {
	const auto movementBits = GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[6];
}

// Returns true if the player is holding a bow and an arrow is drawn
const bool Camera::SmoothCamera::IsBowDrawn(const PlayerCharacter* player) const noexcept {
	const auto movementBits = GetPlayerMovementBits(player);
	return movementBits[31];
}

// Returns true if the player is sitting
const bool Camera::SmoothCamera::IsSitting(const PlayerCharacter* player) const noexcept {
	const auto movementBits = GetPlayerMovementBits(player);
	return movementBits[14] && movementBits[15];
}

// Returns true if the player is sleeping
const bool Camera::SmoothCamera::IsSleeping(const PlayerCharacter* player) const noexcept {
	const auto movementBits = GetPlayerMovementBits(player);
	return (IsSitting(player) && movementBits[16]) || // in bed
		movementBits[15] && movementBits[16] || //getting in bed
		movementBits[17]; // getting out of bed
}

#pragma region UNUSED
// Returns true if the player is mounting a horse
const bool Camera::SmoothCamera::IsMountingHorse(const PlayerCharacter* player) const noexcept {
	const auto actionBits = GetPlayerActionBits(player);
	const auto movementBits = GetPlayerMovementBits(player);
	return (actionBits[3] && actionBits[12] && movementBits[15] || (
		actionBits[12] && movementBits[15]
	)) && !movementBits[14];
}
#pragma endregion

// Returns true if the player is dismounting a horse
const bool Camera::SmoothCamera::IsDisMountingHorse(const PlayerCharacter* player) const noexcept {
	const auto actionBits = GetPlayerActionBits(player);
	const auto movementBits = GetPlayerMovementBits(player);
	return actionBits[3] && actionBits[12] && (movementBits[16]);
}
#pragma endregion

#pragma region Camera state updates
// Returns the current camera state for use in selecting an update method
const Camera::CameraState Camera::SmoothCamera::GetCurrentCameraState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) {
	CameraState newState = CameraState::Unknown;
	if (!povWasPressed && !IsInHorseCamera(camera) && !IsInDragonCamera(camera) && IsSitting(player) 
		&& !IsSleeping(player) && config->compatIC_FirstPersonSitting)
	{
		const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraState);
		if (tps) {
			if (tps->cameraZoom < -1 && tps->cameraLastZoom < -1) {
				newState = CameraState::FirstPerson;
				goto applyStateTransition; // SUE ME
			}
		}
	}

	if (IsSleeping(player)) {
		newState = CameraState::FirstPerson;
	} else if (IsInAutoVanityCamera(camera)) {
		newState = CameraState::Vanity;
	} else if (IsInTweenCamera(camera)) {
		newState = CameraState::Tweening;
	} else if (IsInCameraTransition(camera)) {
		newState = CameraState::Transitioning;
	} else if (IsInUsingObjectCamera(camera)) {
		newState = CameraState::UsingObject;
	} else if (IsInKillMove(camera)) {
		newState = CameraState::KillMove;
	} else if (IsInBleedoutCamera(camera)) {
		newState = CameraState::Bleedout;
	} else if (IsInFreeCamera(camera)) {
		newState = CameraState::Free;
	} else if (IsInAimingCamera(camera)) {
		newState = CameraState::IronSights;
	} else if (IsInFurnitureCamera(camera)) {
		newState = CameraState::Furniture;
	} else if (IsFirstPerson(camera) || povWasPressed) {
		// Holding F goes into that weird mode, just let the first person handler deal with it since we do nothing there
		newState = CameraState::FirstPerson;
	} else if (IsInHorseCamera(camera)) {
		if (config->comaptIC_FirstPersonHorse) {
			const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraState);
			if (tps) {
				if ((tps->cameraZoom == -SKYRIM_MIN_ZOOM_FRACTION && tps->cameraLastZoom == -SKYRIM_MIN_ZOOM_FRACTION) ||
					currentActionState == CameraActionState::FirstPersonHorseback)
				{
					if (povWasPressed)
						newState = CameraState::Horseback;
					else
						newState = CameraState::FirstPerson;
				} else {
					newState = CameraState::Horseback;
				}
			} else {
				newState = CameraState::Horseback;
			}
		} else {
			newState = CameraState::Horseback;
		}
	} else if (IsInDragonCamera(camera)) {
		if (config->comaptIC_FirstPersonDragon) {
			const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraState);
			if (tps) {
				if ((tps->cameraZoom == -SKYRIM_MIN_ZOOM_FRACTION && tps->cameraLastZoom == -SKYRIM_MIN_ZOOM_FRACTION) ||
					currentActionState == CameraActionState::FirstPersonDragon)
				{
					newState = CameraState::FirstPerson;
				} else {
					newState = CameraState::Dragon;
				}
			} else {
				newState = CameraState::Dragon;
			}
		} else {
			newState = CameraState::Dragon;
		}
	} else {
		if (IsThirdPerson(camera)) {
			if (IsThirdPersonCombat(player, camera)) {
				// We have a custom handler for third person with a weapon out
				newState = CameraState::ThirdPersonCombat;
			} else {
				newState = CameraState::ThirdPerson;
			}
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

	if (IsInHorseCamera(camera)) {
		// Improved camera compat
		if (!povIsThird) {
			newState = CameraActionState::FirstPersonHorseback;
		} else if (IsDisMountingHorse(player)) {
			newState = CameraActionState::DisMounting;
		}
	} else if (IsInDragonCamera(camera)) {
		// Improved camera compat
		if (currentState == CameraState::FirstPerson) {
			newState = CameraActionState::FirstPersonDragon;
		}
	} else if (IsSleeping(player)) {
		newState = CameraActionState::Sleeping;
	} else if (IsInFurnitureCamera(camera)) {
		newState = CameraActionState::SittingTransition;
	} else if (IsSitting(player)) {
		// Improved camera compat
		if (currentState == CameraState::FirstPerson) {
			newState = CameraActionState::FirstPersonSitting;
		} else {
			newState = CameraActionState::Sitting;
		}
	} else if (IsSneaking(player)) {
		newState = CameraActionState::Sneaking;
	} else if (IsBowDrawn(player)) {
		newState = CameraActionState::Aiming;
	} else if (IsSwimming(player)) {
		newState = CameraActionState::Swimming;
	} else if (IsSprinting(player)) {
		newState = CameraActionState::Sprinting;
	} else if (IsWalking(player)) {
		newState = CameraActionState::Walking;
	} else if (IsRunning(player)) {
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

// Triggers when the camera state changes, for debugging
void Camera::SmoothCamera::OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const CameraState newState, const CameraState oldState) const
{
	switch (oldState) {
		case CameraState::ThirdPerson: {
			if (cameraStates.at(static_cast<size_t>(CameraState::ThirdPerson))) {
				dynamic_cast<State::ThirdpersonState*>(
					cameraStates.at(static_cast<size_t>(CameraState::ThirdPerson)).get()
				)->OnEnd(player, camera);
				break;
			}
		}
		case CameraState::ThirdPersonCombat: {
			if (cameraStates.at(static_cast<size_t>(CameraState::ThirdPersonCombat))) {
				dynamic_cast<State::ThirdpersonCombatState*>(
					cameraStates.at(static_cast<size_t>(CameraState::ThirdPersonCombat)).get()
				)->OnEnd(player, camera);
				break;
			}
		}
		case CameraState::Horseback: {
			if (cameraStates.at(static_cast<size_t>(CameraState::Horseback))) {
				dynamic_cast<State::ThirdpersonHorseState*>(
					cameraStates.at(static_cast<size_t>(CameraState::Horseback)).get()
				)->OnEnd(player, camera);
				break;
			}
		}
		default:
			break;
	}

	switch (newState) {
		case CameraState::ThirdPerson: {
			if (cameraStates.at(static_cast<size_t>(CameraState::ThirdPerson))) {
				dynamic_cast<State::ThirdpersonState*>(
					cameraStates.at(static_cast<size_t>(CameraState::ThirdPerson)).get()
				)->OnBegin(player, camera);
				break;
			}
		}
		case CameraState::ThirdPersonCombat: {
			if (cameraStates.at(static_cast<size_t>(CameraState::ThirdPersonCombat))) {
				dynamic_cast<State::ThirdpersonCombatState*>(
					cameraStates.at(static_cast<size_t>(CameraState::ThirdPersonCombat)).get()
				)->OnBegin(player, camera);
				break;
			}
		}
		case CameraState::Horseback: {
			if (cameraStates.at(static_cast<size_t>(CameraState::Horseback))) {
				dynamic_cast<State::ThirdpersonHorseState*>(
					cameraStates.at(static_cast<size_t>(CameraState::Horseback)).get()
				)->OnBegin(player, camera);
				break;
			}
		}
		default:
			break;
	}
}
#pragma endregion

#pragma region Camera position calculations
// Returns the zoom value set from the given camera state
float Camera::SmoothCamera::GetCurrentCameraZoom(const CorrectedPlayerCamera* camera, const CameraState currentState) const noexcept {
	switch (currentState) {
		case CameraState::ThirdPerson:
		case CameraState::ThirdPersonCombat: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_ThirdPerson2);
		}
		case CameraState::Horseback: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_Horse);
		}
		case CameraState::Dragon: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_Dragon);
		}
		case CameraState::Bleedout: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_Bleedout);
		}
		default:
			return 0.0f;
	}
}

// Returns the camera height for the current player state
float Camera::SmoothCamera::GetCurrentCameraHeight(const PlayerCharacter* player) const noexcept {
	switch (currentState) {
		case CameraState::Horseback: {
			if (IsBowDrawn(player)) {
				return config->bowAimHorseUpOffset;
			} else {
				return config->horsebackUpOffset;
			}
		}
		default:
			break;
	}

	switch (currentActionState) {
		case CameraActionState::DisMounting: {
			return config->standingUpOffset; // Better when dismounting
		}
		case CameraActionState::Sleeping: {
			return config->sittingUpOffset;
		}
		case CameraActionState::Sitting: {
			return config->sittingUpOffset;
		}
		case CameraActionState::Sneaking: {
			return IsWeaponDrawn(player) ? config->sneakingCombatUpOffset : config->sneakingUpOffset;
		}
		case CameraActionState::Aiming: {
			return config->bowAimUpOffset;
		}
		case CameraActionState::Swimming: {
			return config->swimmingUpOffset;
		}
		case CameraActionState::Sprinting: {
			return IsWeaponDrawn(player) ? config->sprintingCombatUpOffset : config->sprintingUpOffset;
		}
		case CameraActionState::Walking: {
			return IsWeaponDrawn(player) ? config->walkingCombatUpOffset : config->walkingUpOffset;
		}
		case CameraActionState::Running: {
			return IsWeaponDrawn(player) ? config->runningCombatUpOffset : config->runningUpOffset;
		}
		case CameraActionState::Standing: {
			return IsWeaponDrawn(player) ? config->standingCombatUpOffset : config->standingUpOffset;
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
		case CameraState::Horseback: {
			if (IsBowDrawn(player)) {
				return config->bowAimHorseSideOffset;
			} else {
				return config->horsebackSideOffset;
			}
		}
		default:
			break;
	}
	
	switch (currentActionState) {
		case CameraActionState::DisMounting: {
			return config->standingSideOffset; // Better when dismounting
		}
		case CameraActionState::Sleeping: {
			return config->sittingSideOffset;
		}
		case CameraActionState::Sitting: {
			return config->sittingSideOffset;
		}
		case CameraActionState::Sneaking: {
			return IsWeaponDrawn(player) ? config->sneakingCombatSideOffset : config->sneakingSideOffset;
		}
		case CameraActionState::Aiming: {
			return config->bowAimSideOffset;
		}
		case CameraActionState::Swimming: {
			return config->swimmingSideOffset;
		}
		case CameraActionState::Sprinting: {
			return IsWeaponDrawn(player) ? config->sprintingCombatSideOffset : config->sprintingSideOffset;
		}
		case CameraActionState::Walking: {
			return IsWeaponDrawn(player) ? config->walkingCombatSideOffset : config->walkingSideOffset;
		}
		case CameraActionState::Running: {
			return IsWeaponDrawn(player) ? config->runningCombatSideOffset : config->runningSideOffset;
		}
		case CameraActionState::Standing: {
			return IsWeaponDrawn(player) ? config->standingCombatSideOffset : config->standingSideOffset;
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
glm::vec3 Camera::SmoothCamera::GetCurrentCameraOffset(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept {
	return {
		GetCurrentCameraSideOffset(player, camera),
		GetCurrentCameraDistance(camera),
		GetCurrentCameraHeight(player)
	};
}

// Returns the full world-space camera target postion for the current player state
glm::vec3 Camera::SmoothCamera::GetCurrentCameraTargetWorldPosition(const PlayerCharacter* player,
	const CorrectedPlayerCamera* camera) const
{
	if (player->loadedState && player->loadedState->node) {
		BSFixedString nodeName = currentState == CameraState::Horseback ? "HorseSpine2" : "Camera3rd [Cam3]";
		const NiAVObject* node = player->loadedState->node->GetObjectByName(&nodeName.data);
		if (node) {
			return glm::vec3(
				player->pos.x, //node->m_worldTransform.pos.x,
				player->pos.y, //node->m_worldTransform.pos.y,
				node->m_worldTransform.pos.z
			);
		}
	}

	return {
		player->pos.x,
		player->pos.y,
		player->pos.z
	};
}

// Returns the current smoothing scalar to use for the given distance to the player
float Camera::SmoothCamera::GetCurrentSmoothingScalar(const float distance, bool zScalar) const {
	Config::ScalarMethods scalarMethod;

	const auto delta = GetFrameDelta();
	const auto fps = 1.0f / delta;
	float scalar = 1.0f;
	float interpValue = 1.0f;
	float remapped = 1.0f;

	if (zScalar) {
		const auto max = config->separateZMaxSmoothingDistance;
		scalar = glm::clamp(1.0f - ((max - distance) / max), 0.0f, 1.0f);
		remapped = mmath::Remap<float>(scalar, 0.0f, 1.0f, config->separateZMinFollowRate, config->separateZMaxFollowRate);
		scalarMethod = config->separateZScalar;
	} else {
		const auto max = config->zoomMaxSmoothingDistance;
		scalar = glm::clamp(1.0f - ((max - distance) / max), 0.0f, 1.0f);
		remapped = mmath::Remap<float>(scalar, 0.0f, 1.0f, config->minCameraFollowRate, config->maxCameraFollowRate);
		scalarMethod = config->currentScalar;
	}

	const auto mul = -fps * glm::log2(1.0f - remapped);
	interpValue = glm::clamp(1.0f - glm::exp2(-mul * delta), 0.0f, 1.0f);
	
	switch (scalarMethod) {
		case Config::ScalarMethods::LINEAR: {
			scalar = glm::linearInterpolation(interpValue);
			break;
		}
		case Config::ScalarMethods::QUAD_IN: {
			scalar = glm::quadraticEaseIn(interpValue);
			break;
		}
		case Config::ScalarMethods::QUAD_OUT: {
			scalar = glm::quadraticEaseOut(interpValue);
			break;
		}
		case Config::ScalarMethods::QUAD_INOUT: {
			scalar = glm::quadraticEaseInOut(interpValue);
			break;
		}
		case Config::ScalarMethods::CUBIC_IN: {
			scalar = glm::cubicEaseIn(interpValue);
			break;
		}
		case Config::ScalarMethods::CUBIC_OUT: {
			scalar = glm::cubicEaseOut(interpValue);
			break;
		}
		case Config::ScalarMethods::CUBIC_INOUT: {
			scalar = glm::cubicEaseInOut(interpValue);
			break;
		}
		case Config::ScalarMethods::QUART_IN: {
			scalar = glm::quarticEaseIn(interpValue);
			break;
		}
		case Config::ScalarMethods::QUART_OUT: {
			scalar = glm::quarticEaseOut(interpValue);
			break;
		}
		case Config::ScalarMethods::QUART_INOUT: {
			scalar = glm::quarticEaseInOut(interpValue);
			break;
		}
		case Config::ScalarMethods::QUINT_IN: {
			scalar = glm::quinticEaseIn(interpValue);
			break;
		}
		case Config::ScalarMethods::QUINT_OUT: {
			scalar = glm::quinticEaseOut(interpValue);
			break;
		}
		case Config::ScalarMethods::QUINT_INOUT: {
			scalar = glm::quinticEaseInOut(interpValue);
			break;
		}
		case Config::ScalarMethods::SINE_IN: {
			scalar = glm::sineEaseIn(interpValue);
			break;
		}
		case Config::ScalarMethods::SINE_OUT: {
			scalar = glm::sineEaseOut(interpValue);
			break;
		}
		case Config::ScalarMethods::SINE_INOUT: {
			scalar = glm::sineEaseInOut(interpValue);
			break;
		}
		case Config::ScalarMethods::CIRC_IN: {
			scalar = glm::circularEaseIn(interpValue);
			break;
		}
		case Config::ScalarMethods::CIRC_OUT: {
			scalar = glm::circularEaseOut(interpValue);
			break;
		}
		case Config::ScalarMethods::CIRC_INOUT: {
			scalar = glm::circularEaseInOut(interpValue);
			break;
		}
		case Config::ScalarMethods::EXP_IN: {
			scalar = glm::exponentialEaseIn(interpValue);
			break;
		}
		case Config::ScalarMethods::EXP_OUT: {
			scalar = glm::exponentialEaseOut(interpValue);
			break;
		}
		case Config::ScalarMethods::EXP_INOUT: {
			scalar = glm::exponentialEaseInOut(interpValue);
			break;
		}
		default: {
			scalar = glm::linearInterpolation(interpValue);
			break;
		}
	}

	return scalar;
}
#pragma endregion

#pragma region Crosshair stuff
// Updates the screen position of the crosshair for correct aiming
void Camera::SmoothCamera::UpdateCrosshairPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const {
	const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraStates[PlayerCamera::kCameraState_ThirdPerson2]);
	
	auto cameraLocalOfs = glm::vec3(
		0.0f,
		0.0f,
		tps->fOverShoulderPosZ
	);

	if (player->leftHandSpell || player->rightHandSpell) {
		cameraLocalOfs.z = 60.0f;
	}

	auto rotation = glm::identity<glm::mat4>();
	rotation = glm::rotate(rotation, -GetCameraPitchRotation(camera), UNIT_FORWARD); // pitch
	auto rotated = rotation * glm::vec4(0.0f, 2048.0f, 0.0f, 1.0f);
	auto cameraWorldOfs = rotation * glm::vec4(cameraLocalOfs.x, cameraLocalOfs.y, cameraLocalOfs.z, 1.0f);

	rotation = glm::identity<glm::mat4>();
	rotation = glm::rotate(rotation, -GetCameraYawRotation(camera), UNIT_UP); // yaw
	rotated = rotation * rotated;
	cameraWorldOfs = rotation * cameraWorldOfs;

	if (!mmath::IsValid(rotated)) return;

	NiPoint3 xform;
	BSFixedString nodeName = "NPC Head [Head]";
	const NiAVObject* node = player->loadedState->node->GetObjectByName(&nodeName.data);
	if (node) {
		xform = node->m_worldTransform.pos;
	}

	cameraWorldOfs.w = 0.0f;
	const auto origin = glm::vec4(xform.x, xform.y, xform.z, 0.0f) + cameraWorldOfs;

	const auto ray = origin + glm::vec4(rotated.x, rotated.y, rotated.z, 0.0f);

	const auto result = Raycast::CastRay(origin, ray, 1.0f);
	glm::vec2 crosshairPos(0.0f, 0.0f);

	if (result.hit) {
		glm::vec3 screen = {};
		auto pt = NiPoint3(result.hitPos.x, result.hitPos.y, result.hitPos.z);
		(*WorldPtToScreenPt3_Internal)(
			g_worldToCamMatrix, g_viewPort, &pt,
			&screen.x, &screen.y, &screen.z, 1.0f
		);

		constexpr auto w = 1280.0f;
		constexpr auto h = 720.0f;
		constexpr auto half_w = w * 0.5f;
		constexpr auto half_h = h * 0.5f;

		crosshairPos.x = (screen.x * w) - half_w;
		crosshairPos.y = (screen.y * h) - half_h;
	}

	SetCrosshairPosition(crosshairPos);
}

void Camera::SmoothCamera::SetCrosshairPosition(const glm::vec2& pos) const {
	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		GFxValue result;
		GFxValue args[3];
		args[0].SetString("SetCrosshairPosition");
		args[1].SetNumber(static_cast<double>(pos.x) + 23.0); // These offsets were obtained by just comparing screenshots
		args[2].SetNumber(static_cast<double>(pos.y) - 115.0); // I really need to figure out what is actually going on here, rather than use magic numbers
		menu->view->Invoke("call", &result, static_cast<GFxValue*>(args), 3);
	}
}
#pragma endregion

#pragma region Camera getters
// Returns the camera's pitch
float Camera::SmoothCamera::GetCameraPitchRotation(const CorrectedPlayerCamera* camera) const noexcept {
	const auto mat = camera->cameraNode->m_localTransform.rot;
	auto a = glm::clamp(-mat.data[2][1], -0.99f, 0.99f);
	auto arcsin = glm::asin(a);
	return arcsin;
}

// Returns the camera's yaw
float Camera::SmoothCamera::GetCameraYawRotation(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->lookYaw;
}

// Returns the camera's current zoom level - Camera must extend ThirdPersonState
float Camera::SmoothCamera::GetCameraZoomScalar(const CorrectedPlayerCamera* camera, uint16_t cameraState) const noexcept {
	const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraStates[cameraState]);
	if (!tps) return 0.0f;

	if (config->comaptIC_FirstPersonHorse && cameraState == static_cast<uint16_t>(CameraState::Horseback)) {
		if (tps->cameraZoom == SKYRIM_MIN_ZOOM_FRACTION && tps->cameraLastZoom == SKYRIM_MIN_ZOOM_FRACTION) {
			return 0.0f;
		}
	} else if (config->comaptIC_FirstPersonDragon && cameraState == static_cast<uint16_t>(CameraState::Dragon)) {
		if (tps->cameraZoom == SKYRIM_MIN_ZOOM_FRACTION && tps->cameraLastZoom == SKYRIM_MIN_ZOOM_FRACTION) {
			return 0.0f;
		}
	}

	const auto zoom = tps->cameraZoom;
	return zoom + SKYRIM_MIN_ZOOM_FRACTION;
}
#pragma endregion

// Selects the correct update method and positions the camera
void Camera::SmoothCamera::UpdateCamera(const PlayerCharacter* player, CorrectedPlayerCamera* camera) {
	auto cameraNode = camera->cameraNode;
	auto cameraNi = reinterpret_cast<NiCamera*>(
		cameraNode->m_children.m_size == 0 ?
		nullptr :
		cameraNode->m_children.m_data[0]
	);
	if (!cameraNi) return;

	config = Config::GetCurrentConfig();

	// Update states & effects
	const auto pov = UpdateCameraPOVState(player, camera);
	const auto state = GetCurrentCameraState(player, camera);
	const auto actionState = GetCurrentCameraActionState(player, camera);

	// Perform a bit of setup to smooth out camera loading
	if (!firstFrame) {
		startTime = GetTime();
		curFrame = 0.0001;
		lastFrame = 0.0;
		lastPosition = currentPosition = {
			cameraNode->m_worldTransform.pos.x,
			cameraNode->m_worldTransform.pos.y,
			cameraNode->m_worldTransform.pos.z
		};
		firstFrame = true;
	}

	curFrame = GetTime() - startTime;

	// Save the camera position
	lastPosition = currentPosition;

	switch (state) {
		case CameraState::ThirdPerson: {
			if (cameraStates.at(static_cast<size_t>(CameraState::ThirdPerson))) {
				dynamic_cast<State::ThirdpersonState*>(
					cameraStates.at(static_cast<size_t>(CameraState::ThirdPerson)).get()
				)->Update(player, camera);
				break;
			}
		}
		case CameraState::ThirdPersonCombat: {
			if (cameraStates.at(static_cast<size_t>(CameraState::ThirdPersonCombat))) {
				dynamic_cast<State::ThirdpersonCombatState*>(
					cameraStates.at(static_cast<size_t>(CameraState::ThirdPersonCombat)).get()
				)->Update(player, camera);
				break;
			}
		}
		case CameraState::Horseback: {
			if (cameraStates.at(static_cast<size_t>(CameraState::Horseback))) {
				dynamic_cast<State::ThirdpersonHorseState*>(
					cameraStates.at(static_cast<size_t>(CameraState::Horseback)).get()
				)->Update(player, camera);
				break;
			}
		}

		// Here just for my own reference that these are unused (for now)
		case CameraState::FirstPerson:
		case CameraState::KillMove:
		case CameraState::Tweening:
		case CameraState::Transitioning:
		case CameraState::UsingObject:
		case CameraState::Vanity:
		case CameraState::Free:
		case CameraState::IronSights:
		case CameraState::Furniture:
		case CameraState::Bleedout:
		case CameraState::Dragon:
		case CameraState::Unknown:
		default: {
			currentPosition = {
				cameraNode->m_worldTransform.pos.x,
				cameraNode->m_worldTransform.pos.y,
				cameraNode->m_worldTransform.pos.z
			};
			break;
		}
	}

	povWasPressed = false;
	lastFrame = curFrame;

#ifdef _DEBUG
	if (!mmath::IsValid(currentPosition)) {
		__debugbreak();
		// Oops, go ahead and clear both
		lastPosition = currentPosition = {
			cameraNode->m_worldTransform.pos.x,
			cameraNode->m_worldTransform.pos.y,
			cameraNode->m_worldTransform.pos.z
		};
		return;
	}
#endif

	const NiPoint3 pos = { currentPosition.x, currentPosition.y, currentPosition.z };
	cameraNode->m_localTransform.pos = pos;
	cameraNode->m_worldTransform.pos = pos;
	cameraNi->m_worldTransform.pos = pos;
}