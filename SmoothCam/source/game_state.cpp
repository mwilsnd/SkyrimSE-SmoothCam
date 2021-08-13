#include "game_state.h"
#include "compat.h"

// Returns the bits for player->actorState->flags04 which appear to convey movement info
const eastl::bitset<32> GameState::GetPlayerMovementBits(const Actor* player) noexcept {
	const auto bits = eastl::bitset<32>(player->actorState.flags04);
	return bits;
}

// Returns the bits for player->actorState->flags08 which appear to convey action info
const eastl::bitset<32> GameState::GetPlayerActionBits(const Actor* player) noexcept {
	const auto bits = eastl::bitset<32>(player->actorState.flags08);
	return bits;
}

const bool GameState::IC_InFirstPersonState() noexcept {
	if (Compat::IsPresent(Compat::Mod::ImprovedCamera))
		return Compat::IC_IsFirstPerson();
	else
		return false;
}

const bool GameState::IFPV_InFirstPersonState() noexcept {
	if (Compat::IsPresent(Compat::Mod::ImmersiveFirstPersonView))
		return Compat::IFPV_IsFirstPerson();
	else
		return false;
}

// Returns true if the player is in first person
const bool GameState::IsFirstPerson(const CorrectedPlayerCamera* camera) noexcept {
	if (Compat::IsPresent(Compat::Mod::ImmersiveFirstPersonView)) {
		const auto fps = camera->cameraState == camera->cameraStates[camera->kCameraState_FirstPerson];
		return fps || IFPV_InFirstPersonState();
	} else if (Compat::IsPresent(Compat::Mod::ImprovedCamera)) {
		const auto fps = camera->cameraState == camera->cameraStates[camera->kCameraState_FirstPerson];
		return fps || IC_InFirstPersonState();
	} else {
		return camera->cameraState == camera->cameraStates[camera->kCameraState_FirstPerson];
	}
}

// Returns true if the player is in third person
const bool GameState::IsThirdPerson(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[camera->kCameraState_ThirdPerson2] && !IsFirstPerson(camera);
}

// Returns true if the player has a weapon drawn and in third person
const bool GameState::IsThirdPersonCombat(const Actor* player, const CorrectedPlayerCamera* camera) noexcept {
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
const bool GameState::IsInHorseCamera(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Horse] && !IsFirstPerson(camera);
}

// Returns true if the player is bleeding out
const bool GameState::IsInBleedoutCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Bleedout];
}

// Returns true if the player is riding a dragon
const bool GameState::IsInDragonCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Dragon];
}

const GameState::CameraState GameState::GetCameraState(const Actor* player, const CorrectedPlayerCamera* camera) noexcept {
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
	} else if (GameState::IsInHorseCamera(player, camera)) {
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

const bool GameState::IsWeaponDrawn(const Actor* player) noexcept {
	const auto bits = GameState::GetPlayerActionBits(player);
	return bits[6];
}

// Get an equipped weapon
const TESObjectWEAP* GameState::GetEquippedWeapon(const Actor* player, bool leftHand) noexcept {
	if(!player->processManager) return nullptr;

	TESForm* wep = nullptr;
	if(leftHand)
		wep = player->processManager->equippedObject[ActorProcessManager::kEquippedHand_Left];
	else
		wep = player->processManager->equippedObject[ActorProcessManager::kEquippedHand_Right];

	if (!wep || !wep->IsWeapon()) return nullptr;
	return reinterpret_cast<const TESObjectWEAP*>(wep);
}

const TESAmmo* GameState::GetCurrentAmmo(const Actor* player) noexcept {
	typedef TESAmmo*(__thiscall Actor::* GetAmmo)() const;
	return (player->*reinterpret_cast<GetAmmo>(&Actor::Unk_9E))();
}

float GameState::GetCurrentBowDrawTimer(const PlayerCharacter* player) noexcept {
	// @Note: Read projectile->unk188
	// This is the arrow draw duration/shot power, stored on the player in a small array used as a stack
	// This grows, with the top value being the current timer - until an arrow is fired, clearing the stack
	const auto arr = reinterpret_cast<const PlayerArrayBA0*>(&(player->unkBA0));
	float drawTimerValue = arr->size > 0 ? glm::min(arr->top().bowDrawTime, 1.0f) : 1.0f;
	if (!mmath::IsValid(drawTimerValue)) drawTimerValue = 1.0f;
	return drawTimerValue;
}

bool GameState::IsUsingMagicItem(const Actor* player, bool leftHand) noexcept {
	if (leftHand && !player->leftHandSpell) return false;
	if (!leftHand && !player->rightHandSpell) return false;

	const EnchantmentItem* ench = leftHand ?
		DYNAMIC_CAST(player->leftHandSpell, TESForm, EnchantmentItem) :
		DYNAMIC_CAST(player->rightHandSpell, TESForm, EnchantmentItem);

	if (!ench) return false;
	const auto wep = GetEquippedWeapon(player, leftHand);
	if (!wep) return false;
	
	// Just look for a staff right now, consider anything else to be non-magic
	return wep->gameData.type == TESObjectWEAP::GameData::kType_Staff ||
		wep->gameData.type == TESObjectWEAP::GameData::kType_Staff2;
}

bool GameState::IsCombatMagic(const SpellItem* spell) noexcept {
	if (!spell) return false;

	const auto spellType = spell->data.type;
	const auto castType = spell->data.castType;
	const auto deliveryType = spell->data.unk14;

	// Spell, leveled spell, scrolls
	if (spellType != 0 && spellType != 9 && spellType != 13)
		return false;

	// On self
	if (deliveryType == 0) return false;

	// Constant effect
	if (castType == 0) return false;

	return true;
}

// Returns true if the player has a melee weapon equiped
const bool GameState::IsMeleeWeaponDrawn(const Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	// Continue to look for a staff here - consider a non-enchanted staff to be melee
	if (right) {
		if (right->gameData.type != TESObjectWEAP::GameData::kType_Bow &&
			right->gameData.type != TESObjectWEAP::GameData::kType_CrossBow &&
			right->gameData.type != TESObjectWEAP::GameData::kType_Bow2 &&
			right->gameData.type != TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	if (left) {
		if (left->gameData.type != TESObjectWEAP::GameData::kType_Bow &&
			left->gameData.type != TESObjectWEAP::GameData::kType_CrossBow &&
			left->gameData.type != TESObjectWEAP::GameData::kType_Bow2 &&
			left->gameData.type != TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	return false;
}

// Returns true if the player has magic drawn
const bool GameState::IsMagicDrawn(const Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	
	// Using an item that counts as magic
	if (GameState::IsUsingMagicItem(player))
		return true;

	if (GameState::IsUsingMagicItem(player, true))
		return true;

	// Or a spell
	return (GameState::IsCombatMagic(player->leftHandSpell) && !GetEquippedWeapon(player, true)) ||
		(GameState::IsCombatMagic(player->rightHandSpell) && !GetEquippedWeapon(player));
}

// Returns true if the player has a ranged weapon drawn
const bool GameState::IsRangedWeaponDrawn(const Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	if (right) {
		if (right->gameData.type == TESObjectWEAP::GameData::kType_Bow ||
			right->gameData.type == TESObjectWEAP::GameData::kType_CrossBow ||
			right->gameData.type == TESObjectWEAP::GameData::kType_Bow2 ||
			right->gameData.type == TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	if (left) {
		if (left->gameData.type == TESObjectWEAP::GameData::kType_Bow ||
			left->gameData.type == TESObjectWEAP::GameData::kType_CrossBow ||
			left->gameData.type == TESObjectWEAP::GameData::kType_Bow2 ||
			left->gameData.type == TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	return false;
}

const bool GameState::IsUsingCrossbow(const Actor* player) noexcept {
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

const bool GameState::IsUsingBow(const Actor* player) noexcept {
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
const bool GameState::IsSneaking(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[9];
}

// Returns true if the player is sprinting
const bool GameState::IsSprinting(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[8];
}

// Returns true if the player is running
const bool GameState::IsRunning(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return ((movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[7]) && !IsOverEncumbered(player);
}

// Returns true if the player is swimming
const bool GameState::IsSwimming(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[10];
}

// Returns true if the player is walking
const bool GameState::IsWalking(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		(movementBits[6] || (movementBits[7] && IsOverEncumbered(player)));
}

// Returns true if the player is holding a bow and an arrow is drawn
const bool GameState::IsBowDrawn(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	const auto actionBits = GameState::GetPlayerActionBits(player);

	// AGO compat
	if (Compat::IsPresent(Compat::Mod::ArcheryGameplayOverhaul) && !GameState::IsUsingCrossbow(player)) {
		// We get 1 frame here with AGO where we are at our limits just checking the action bits
		// This convoluted bit of a state machine works around that
		static float lastDrawnTimer = 0.0f;
		static bool drawnLastCall = false;
		static bool objective = false;

		if (!movementBits[31] || movementBits[30]) {
			drawnLastCall = objective = false;
			return false;
		}

		// Bits 28 & 29 flip around, randomly, it seems.
		// We want to see bit 29 get set.
		if (movementBits[29]) objective = true;

		if (!drawnLastCall && objective) {
			drawnLastCall = true;
			lastDrawnTimer = GameTime::CurTime() + 0.1f;
		}

		if (objective && GameTime::CurTime() > lastDrawnTimer) {
			return true;
		}

		return false;
	} else {
		if (GameState::IsUsingCrossbow(player)) {
			// Crossbows are a bit different
			return movementBits[31] && movementBits[29];
		} else {
			return movementBits[31];
		}
	}
}

// Returns true if the player is sitting
const bool GameState::IsSitting(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[14] && movementBits[15];
}

// Returns true if the player is sleeping
const bool GameState::IsSleeping(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (GameState::IsSitting(player) && movementBits[16]) || // in bed
		movementBits[15] && movementBits[16] || //getting in bed
		movementBits[17]; // getting out of bed
}

// Returns true if the player is mounting a horse
const bool GameState::IsMountingHorse(const Actor* player) noexcept {
	const auto actionBits = GameState::GetPlayerActionBits(player);
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (actionBits[3] && actionBits[12] && movementBits[15] || (
		actionBits[12] && movementBits[15]
	)) && !movementBits[14];
}

// Returns true if the player is dismounting a horse
const bool GameState::IsDisMountingHorse(const Actor* player) noexcept {
	const auto actionBits = GameState::GetPlayerActionBits(player);
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return actionBits[3] && actionBits[12] && (movementBits[16]);
}

const bool GameState::InPOVSlideMode() noexcept {
	return PlayerControls::GetSingleton()->unk04B;
}

// Returns true if the player is a vampire lord
const bool GameState::IsVampireLord(const Actor* player) noexcept {
	if (!player->race) return false;
	return strcmp(player->race->fullName.name.c_str(), "Vampire Lord") == 0;
}

// Returns true if the player is a werewolf
const bool GameState::IsWerewolf(const Actor* player) noexcept {
	if (!player->race) return false;
	return strcmp(player->race->fullName.name.c_str(), "Werewolf") == 0;
}

const bool GameState::IsOverEncumbered(const Actor* player) noexcept {
	typedef bool(*getter)(const Actor*);
	return Offsets::Get<getter>(36457)(player);
}