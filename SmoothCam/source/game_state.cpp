#include "game_state.h"

// Returns the bits for player->actorState->flags04 which appear to convey movement info
const std::bitset<32> GameState::GetPlayerMovementBits(const PlayerCharacter* player) noexcept {
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
const std::bitset<32> GameState::GetPlayerActionBits(const PlayerCharacter* player) noexcept {
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

// Returns true if the player is in first person
const bool GameState::IsFirstPerson(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[camera->kCameraState_FirstPerson];
}

// Returns true if the player is in third person
const bool GameState::IsThirdPerson(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[camera->kCameraState_ThirdPerson2];
}

// Returns true if the player has a weapon drawn and in third person
const bool GameState::IsThirdPersonCombat(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept {
	return GameState::IsThirdPerson(camera) && GameState::IsWeaponDrawn(player);
}

// Returns true if a kill move is playing
const bool GameState::IsInKillMove(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_VATS];
}

// Returns true if the camera is tweening
const bool GameState::IsInTweenCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_TweenMenu];
}

// Returns true if the camera is transitioning
const bool GameState::IsInCameraTransition(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Transition];
}

// Returns true if the player is using an object
const bool GameState::IsInUsingObjectCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_ThirdPerson1];
}

// Returns true if the camera is in auto vanity mode
const bool GameState::IsInAutoVanityCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_AutoVanity];
}

// Returns true if the camera is in free mode
const bool GameState::IsInFreeCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Free];
}

// Returns true if the camera is in aiming mode
const bool GameState::IsInAimingCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_IronSights];
}

// Returns true if the camera is in furniture mode
const bool GameState::IsInFurnitureCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Furniture];
}

// Returns true if the player is riding a horse
const bool GameState::IsInHorseCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Horse];
}

// Returns true if the player is bleeding out
const bool GameState::IsInBleedoutCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Bleedout];
}

// Returns true if the player is riding a dragon
const bool GameState::IsInDragonCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Dragon];
}

const GameState::CameraState GameState::GetCameraState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept {
	GameState::CameraState newState = GameState::CameraState::Unknown;

	if (GameState::IsSleeping(player)) {
		newState = CameraState::FirstPerson;
	} else if (GameState::IsInAutoVanityCamera(camera)) {
		newState = CameraState::Vanity;
	} else if (GameState::IsInTweenCamera(camera)) {
		newState = CameraState::Tweening;
	} else if (GameState::IsInCameraTransition(camera)) {
		newState = CameraState::Transitioning;
	} else if (GameState::IsInUsingObjectCamera(camera)) {
		newState = CameraState::UsingObject;
	} else if (GameState::IsInKillMove(camera)) {
		newState = CameraState::KillMove;
	} else if (GameState::IsInBleedoutCamera(camera)) {
		newState = CameraState::Bleedout;
	} else if (GameState::IsInFreeCamera(camera)) {
		newState = CameraState::Free;
	} else if (GameState::IsInAimingCamera(camera)) {
		newState = CameraState::IronSights;
	} else if (GameState::IsInFurnitureCamera(camera)) {
		newState = CameraState::Furniture;
	} else if (GameState::IsFirstPerson(camera)) {
		newState = CameraState::FirstPerson;
	} else if (GameState::IsInHorseCamera(camera)) {
		newState = CameraState::Horseback;
	} else if (GameState::IsInDragonCamera(camera)) {
		newState = CameraState::Dragon;
	} else {
		if (GameState::IsThirdPerson(camera)) {
			if (GameState::IsThirdPersonCombat(player, camera)) {
				// We have a custom handler for third person with a weapon out
				newState = CameraState::ThirdPersonCombat;
			} else {
				newState = CameraState::ThirdPerson;
			}
		}
	}

#ifdef _DEBUG
	assert(newState != GameState::CameraState::Unknown);
#endif
	return newState;
}

const bool GameState::IsWeaponDrawn(const PlayerCharacter* player) noexcept {
	const auto bits = GameState::GetPlayerActionBits(player);
	return bits[5] && bits[6];
}

// Get an equipped weapon
const TESObjectWEAP* GameState::GetEquippedWeapon(PlayerCharacter* player, bool leftHand) noexcept {
	auto value = player->GetEquippedObject(player);
	if (!value || !value->IsWeapon()) return nullptr;
	return reinterpret_cast<TESObjectWEAP*>(value);
}

// Returns true if the player has a melee weapon equiped
const bool GameState::IsMeleeWeaponDrawn(PlayerCharacter* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	// Emchanted weapons are considered spells
	if (!right && !left && !GameState::IsMagicDrawn(player) && !GameState::IsRangedWeaponDrawn(player))
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
const bool GameState::IsMagicDrawn(PlayerCharacter* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;

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
const bool GameState::IsRangedWeaponDrawn(PlayerCharacter* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

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

const bool GameState::IsUsingCrossbow(PlayerCharacter* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	if (right && (right->gameData.type == TESObjectWEAP::GameData::kType_CrossBow ||
		right->gameData.type == TESObjectWEAP::GameData::kType_CBow))
	{
		return true;
	}

	if (left && (left->gameData.type == TESObjectWEAP::GameData::kType_CrossBow ||
		left->gameData.type == TESObjectWEAP::GameData::kType_CBow))
	{
		return true;
	}

	return false;
}

const bool GameState::IsUsingBow(PlayerCharacter* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	if (right && (right->gameData.type == TESObjectWEAP::GameData::kType_Bow ||
		right->gameData.type == TESObjectWEAP::GameData::kType_Bow2))
	{
		return true;
	}

	if (left && (left->gameData.type == TESObjectWEAP::GameData::kType_Bow ||
		left->gameData.type == TESObjectWEAP::GameData::kType_Bow2))
	{
		return true;
	}

	return false;
}

// Returns true if the player is sneaking
const bool GameState::IsSneaking(const PlayerCharacter* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[9];
}

// Returns true if the player is sprinting
const bool GameState::IsSprinting(const PlayerCharacter* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[8];
}

// Returns true if the player is running
const bool GameState::IsRunning(const PlayerCharacter* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[7];
}

// Returns true if the player is swimming
const bool GameState::IsSwimming(const PlayerCharacter* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[10];
}

// Returns true if the player is walking
const bool GameState::IsWalking(const PlayerCharacter* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[6];
}

// Returns true if the player is holding a bow and an arrow is drawn
const bool GameState::IsBowDrawn(const PlayerCharacter* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[31];
}

// Returns true if the player is sitting
const bool GameState::IsSitting(const PlayerCharacter* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[14] && movementBits[15];
}

// Returns true if the player is sleeping
const bool GameState::IsSleeping(const PlayerCharacter* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (GameState::IsSitting(player) && movementBits[16]) || // in bed
		movementBits[15] && movementBits[16] || //getting in bed
		movementBits[17]; // getting out of bed
}

// Returns true if the player is mounting a horse
const bool GameState::IsMountingHorse(const PlayerCharacter* player) noexcept {
	const auto actionBits = GameState::GetPlayerActionBits(player);
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (actionBits[3] && actionBits[12] && movementBits[15] || (
		actionBits[12] && movementBits[15]
		)) && !movementBits[14];
}

// Returns true if the player is dismounting a horse
const bool GameState::IsDisMountingHorse(const PlayerCharacter* player) noexcept {
	const auto actionBits = GameState::GetPlayerActionBits(player);
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return actionBits[3] && actionBits[12] && (movementBits[16]);
}