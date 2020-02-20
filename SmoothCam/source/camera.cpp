#include "pch.h"
#include "camera.h"

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

void Camera::SmoothCamera::OnDialogMenuChanged(const MenuOpenCloseEvent* const ev) noexcept {
	dialogMenuOpen = ev->opening;
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
	// Prevent any "accidents" during the inital frame by clamping above 0
	// @Note: This value might've been too large before, reduced one more decimal place
	return glm::max(static_cast<float>(curFrame - lastFrame), 0.00001f);
}

#pragma region Player flags
// Returns the bits for player->actorState->flags04 which appear to convey movement info
const std::bitset<32> Camera::SmoothCamera::GetPlayerMovementBits(const PlayerCharacter* player) const noexcept {
	const auto bits = std::bitset<32>(player->actorState.flags04);
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
	const auto bits = std::bitset<32>(player->actorState.flags08);
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

// Get an equipped weapon
const TESObjectWEAP* Camera::SmoothCamera::GetEquippedWeapon(PlayerCharacter* player, bool leftHand) const noexcept {
	auto value = player->GetEquippedObject(player);
	if (!value || !value->IsWeapon()) return nullptr;
	return reinterpret_cast<TESObjectWEAP*>(value);
}

// Returns true if the player has a melee weapon equiped
const bool Camera::SmoothCamera::IsMeleeWeaponDrawn(PlayerCharacter* player) const noexcept {
	if (!IsWeaponDrawn(player)) return false;
	const auto right = GetEquippedWeapon(player);
	const auto left = GetEquippedWeapon(player, true);

	// Emchanted weapons are considered spells
	if (!right && !left && !IsMagicDrawn(player) && !IsRangedWeaponDrawn(player))
		return true;

	if (right) {
		if (right->gameData.type != TESObjectWEAP::GameData::kType_Bow &&
			right->gameData.type != TESObjectWEAP::GameData::kType_Staff &&
			right->gameData.type != TESObjectWEAP::GameData::kType_CrossBow &&
			right->gameData.type != TESObjectWEAP::GameData::kType_Bow2 &&
			right->gameData.type != TESObjectWEAP::GameData::kType_Staff2 &&
			right->gameData.type != TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	if (left) {
		if (left->gameData.type != TESObjectWEAP::GameData::kType_Bow &&
			left->gameData.type != TESObjectWEAP::GameData::kType_Staff &&
			left->gameData.type != TESObjectWEAP::GameData::kType_CrossBow &&
			left->gameData.type != TESObjectWEAP::GameData::kType_Bow2 &&
			left->gameData.type != TESObjectWEAP::GameData::kType_Staff2 &&
			left->gameData.type != TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	return false;
}

// Returns true if the player has magic drawn
const bool Camera::SmoothCamera::IsMagicDrawn(PlayerCharacter* player) const noexcept {
	if (!IsWeaponDrawn(player)) return false;

	EnchantmentItem* leftWep;
	if (player->leftHandSpell != nullptr)
		leftWep = DYNAMIC_CAST(player->leftHandSpell, TESForm, EnchantmentItem);

	if (player->leftHandSpell != nullptr && leftWep == nullptr) {
		return true;
	}

	EnchantmentItem* rightWep;
	if (player->rightHandSpell != nullptr)
		rightWep = DYNAMIC_CAST(player->rightHandSpell, TESForm, EnchantmentItem);

	if (player->rightHandSpell != nullptr && rightWep == nullptr) {
		return true;
	}

	return false;
}

// Returns true if the player has a ranged weapon drawn
const bool Camera::SmoothCamera::IsRangedWeaponDrawn(PlayerCharacter* player) const noexcept {
	if (!IsWeaponDrawn(player)) return false;
	const auto right = GetEquippedWeapon(player);
	const auto left = GetEquippedWeapon(player, true);

	if (right) {
		if (right->gameData.type == TESObjectWEAP::GameData::kType_Bow ||
			right->gameData.type == TESObjectWEAP::GameData::kType_Staff ||
			right->gameData.type == TESObjectWEAP::GameData::kType_CrossBow ||
			right->gameData.type == TESObjectWEAP::GameData::kType_Bow2 ||
			right->gameData.type == TESObjectWEAP::GameData::kType_Staff2 ||
			right->gameData.type == TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	if (left) {
		if (left->gameData.type == TESObjectWEAP::GameData::kType_Bow ||
			left->gameData.type == TESObjectWEAP::GameData::kType_Staff ||
			left->gameData.type == TESObjectWEAP::GameData::kType_CrossBow ||
			left->gameData.type == TESObjectWEAP::GameData::kType_Bow2 ||
			left->gameData.type == TESObjectWEAP::GameData::kType_Staff2 ||
			left->gameData.type == TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	return false;
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

// Triggers when the camera state changes
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

// Returns an offset group for the current player movement state
const Config::OffsetGroup Camera::SmoothCamera::GetOffsetForState(const CameraActionState state) const noexcept {
	switch (state) {
		case CameraActionState::DisMounting: {
			return config->standing; // Better when dismounting
		}
		case CameraActionState::Sleeping: {
			return config->sitting;
		}
		case CameraActionState::Sitting: {
			return config->sitting;
		}
		case CameraActionState::Sneaking: {
			return config->sneaking;
		}
		case CameraActionState::Aiming: {
			return config->bowAim;
		}
		case CameraActionState::Swimming: {
			return config->swimming;
		}
		case CameraActionState::Sprinting: {
			return config->sprinting;
		}
		case CameraActionState::Walking: {
			return config->walking;
		}
		case CameraActionState::Running: {
			return config->running;
		}
		case CameraActionState::Standing: {
			return config->standing;
		}
		default: {
			return config->standing;
		}
	}
}

// Selects the right offset from an offset group for the player's weapon state
float Camera::SmoothCamera::GetActiveWeaponStateUpOffset(PlayerCharacter* player, const Config::OffsetGroup& group) const noexcept {
	if (!IsWeaponDrawn(player)) return group.upOffset;
	if (IsRangedWeaponDrawn(player)) {
		return group.combatRangedUpOffset;
	}
	if (IsMagicDrawn(player)) {
		return group.combatMagicUpOffset;
	}
	return group.combatMeleeUpOffset;
}

// Selects the right offset from an offset group for the player's weapon state
float Camera::SmoothCamera::GetActiveWeaponStateSideOffset(PlayerCharacter* player, const Config::OffsetGroup& group) const noexcept {
	if (!IsWeaponDrawn(player)) return group.sideOffset;
	if (IsRangedWeaponDrawn(player)) {
		return group.combatRangedSideOffset;
	}
	if (IsMagicDrawn(player)) {
		return group.combatMagicSideOffset;
	}
	return group.combatMeleeSideOffset;
}

// Returns the camera height for the current player state
float Camera::SmoothCamera::GetCurrentCameraHeight(PlayerCharacter* player) const noexcept {
	switch (currentState) {
		case CameraState::Horseback: {
			if (IsBowDrawn(player)) {
				return config->bowAim.horseUpOffset;
			} else {
				return GetActiveWeaponStateUpOffset(player, config->horseback);
			}
		}
		default:
			break;
	}

	const auto ofs = GetOffsetForState(currentActionState);

	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Aiming:
		case CameraActionState::Swimming: {
			return ofs.upOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateUpOffset(player, ofs);
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
		case CameraState::Horseback: {
			if (IsBowDrawn(player)) {
				return config->bowAim.horseSideOffset;
			} else {
				return GetActiveWeaponStateSideOffset(player, config->horseback);
			}
		}
		default:
			break;
	}

	const auto ofs = GetOffsetForState(currentActionState);
	
	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Aiming:
		case CameraActionState::Swimming: {
			return ofs.sideOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateSideOffset(player, ofs);
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

	if (!config->disableDeltaTime) {
		const auto mul = -fps * glm::log2(1.0f - remapped);
		interpValue = glm::clamp(1.0f - glm::exp2(-mul * delta), 0.0f, 1.0f);
	} else {
		interpValue = remapped;
	}

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

// Returns true if interpolation is allowed in the current state
bool Camera::SmoothCamera::IsInterpAllowed(PlayerCharacter* player) const noexcept {
	auto ofs = GetOffsetForState(currentActionState);
	if (currentState == CameraState::Horseback) {
		if (IsWeaponDrawn(player) && IsBowDrawn(player)) {
			return config->bowAim.interpHorseback;
		} else {
			ofs = config->horseback;
		}
	}

	if (!IsWeaponDrawn(player)) return ofs.interp;
	if (IsRangedWeaponDrawn(player)) {
		return ofs.interpRangedCombat;
	}
	if (IsMagicDrawn(player)) {
		return ofs.interpMagicCombat;
	}
	return ofs.interpMeleeCombat;
}
#pragma endregion

#pragma region Crosshair stuff
// Updates the screen position of the crosshair for correct aiming
void Camera::SmoothCamera::UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* camera) const {
	NiPoint3 niOrigin = { 0.01f, 0.01f, 0.01f };
	NiPoint3 niNormal = { 0.0f, 1.00f, 0.0f };

	if (currentState != CameraState::Horseback) {
		// @TODO: Add CommonLibSSE during next major refactor
		typedef void(__thiscall PlayerCharacter::* GetEyeVector)(NiPoint3& origin, NiPoint3& normal, bool factorCameraOffset);
		(player->*reinterpret_cast<GetEyeVector>(&PlayerCharacter::Unk_C2))(niOrigin, niNormal, false);
	} else {
		// EyeVector is busted on horseback
		BSFixedString nodeName = "Throat 2"; // Gets me the closest to niOrigin
		const auto node = player->loadedState->node->GetObjectByName(&nodeName.data);
		if (node) {
			niOrigin = NiPoint3(player->pos.x, player->pos.y, node->m_worldTransform.pos.z);
		}

		auto aproxNormal = glm::vec4(0.0, 1.0, 0.0, 1.0);

		auto m = glm::identity<glm::mat4>();
		m = glm::rotate(m, -GetCameraPitchRotation(camera), UNIT_FORWARD);
		aproxNormal = m * aproxNormal;

		m = glm::identity<glm::mat4>();
		m = glm::rotate(m, -GetCameraYawRotation(camera), UNIT_UP);
		aproxNormal = m * aproxNormal;

		niNormal = NiPoint3(aproxNormal.x, aproxNormal.y, aproxNormal.z);
	}

	// Bow appear to respect the eye vector well enough on it's own

	// This isn't perfect, but better than it was
	if (IsMagicDrawn(player) || IsMeleeWeaponDrawn(player)) {
		BSFixedString nodeName = "NPC Spine1 [Spn1]";
		const auto node = player->loadedState->node->GetObjectByName(&nodeName.data);
		if (node) {
			niOrigin.z -= player->pos.z - node->m_worldTransform.pos.z;
		}
	}

	// Cast the aim ray
	constexpr auto rayLength = 8192.0f;
	const auto origin = glm::vec4(niOrigin.x, niOrigin.y, niOrigin.z, 0.0f);
	const auto ray = glm::vec4(niNormal.x, niNormal.y, niNormal.z, 0.0f) * rayLength;
	const auto result = Raycast::CastRay(origin, origin + ray, 0.01f);

	glm::vec2 crosshairPos(0.0f, 0.0f);
	if (result.hit) {
		glm::vec3 screen = {};

		// Offset our camera local transformation, projview matrix is not in sync with our changes
		const auto localSpace = currentPosition - GetCurrentCameraTargetWorldPosition(player, camera);
		auto pt = NiPoint3(
			result.hitPos.x - localSpace.x,
			result.hitPos.y - localSpace.y,
			result.hitPos.z - localSpace.z
		);

		// Project to screen
		(*WorldPtToScreenPt3_Internal)(
			g_worldToCamMatrix, g_viewPort, &pt,
			&screen.x, &screen.y, &screen.z, 1.0f
		);

		// Remap to scaleform coords
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
		args[1].SetNumber(static_cast<double>(pos.x) + 23.0); // @TODO: These offsets were obtained by just comparing screenshots
		args[2].SetNumber(static_cast<double>(pos.y) - 115.0f); // I really need to figure out what is actually going on here, rather than use magic numbers
		menu->view->Invoke("call", &result, static_cast<GFxValue*>(args), 3);
	}
}

void Camera::SmoothCamera::SetCrosshairEnabled(bool enabled) const {
	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		GFxValue result;
		GFxValue args[2];
		args[0].SetString("SetCrosshairEnabled");
		args[1].SetBool(enabled);
		menu->view->Invoke("call", &result, static_cast<GFxValue*>(args), 2);
	}
}
#pragma endregion

#pragma region Camera getters
// Returns the camera's pitch
float Camera::SmoothCamera::GetCameraPitchRotation(const CorrectedPlayerCamera* camera) const noexcept {
	const auto mat = camera->cameraNode->m_localTransform.rot;
	const auto a = glm::clamp(-mat.data[2][1], -0.99f, 0.99f);
	return glm::asin(a);
}

// Returns the camera's yaw
float Camera::SmoothCamera::GetCameraYawRotation(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->lookYaw;
}

// Returns the camera's current zoom level - Camera must extend ThirdPersonState
float Camera::SmoothCamera::GetCameraZoomScalar(const CorrectedPlayerCamera* camera, uint16_t cameraState) const noexcept {
	const auto state = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraStates[cameraState]);
	if (!state) return 0.0f;
	return state->cameraZoom + SKYRIM_MIN_ZOOM_FRACTION;
}
#pragma endregion

// Selects the correct update method and positions the camera
void Camera::SmoothCamera::UpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera) {
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

	if (config->disableDuringDialog && dialogMenuOpen) {
		currentPosition = {
			cameraNode->m_worldTransform.pos.x,
			cameraNode->m_worldTransform.pos.y,
			cameraNode->m_worldTransform.pos.z
		};
	} else {
		switch (state) {
			case CameraState::ThirdPerson:
			{
				if (cameraStates.at(static_cast<size_t>(CameraState::ThirdPerson))) {
					dynamic_cast<State::ThirdpersonState*>(
						cameraStates.at(static_cast<size_t>(CameraState::ThirdPerson)).get()
					)->Update(player, camera);
					break;
				}
			}
			case CameraState::ThirdPersonCombat:
			{
				if (cameraStates.at(static_cast<size_t>(CameraState::ThirdPersonCombat))) {
					dynamic_cast<State::ThirdpersonCombatState*>(
						cameraStates.at(static_cast<size_t>(CameraState::ThirdPersonCombat)).get()
					)->Update(player, camera);
					break;
				}
			}
			case CameraState::Horseback:
			{
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
			default:
			{
				currentPosition = {
					cameraNode->m_worldTransform.pos.x,
					cameraNode->m_worldTransform.pos.y,
					cameraNode->m_worldTransform.pos.z
				};
				break;
			}
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