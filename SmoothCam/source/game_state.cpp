#include "game_state.h"
#include "compat.h"

extern Offsets* g_Offsets;

// Returns the bits for player->actorState->flags04 which appear to convey movement info
const eastl::bitset<32> GameState::GetPlayerMovementBits(const RE::Actor* player) noexcept {
	const auto bits = eastl::bitset<32>(*reinterpret_cast<const uint32_t*>(&player->actorState1));
	return bits;
}

// Returns the bits for player->actorState->flags08 which appear to convey action info
const eastl::bitset<32> GameState::GetPlayerActionBits(const RE::Actor* player) noexcept {
	const auto bits = eastl::bitset<32>(*reinterpret_cast<const uint32_t*>(&player->actorState2));
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
const bool GameState::IsFirstPerson(const RE::PlayerCamera* camera) noexcept {
	if (Compat::IsPresent(Compat::Mod::ImmersiveFirstPersonView)) {
		const auto fps = camera->currentState == camera->cameraStates[RE::CameraState::kFirstPerson];
		return fps || IFPV_InFirstPersonState();
	} else if (Compat::IsPresent(Compat::Mod::ImprovedCamera)) {
		const auto fps = camera->currentState == camera->cameraStates[RE::CameraState::kFirstPerson];
		return fps || IC_InFirstPersonState();
	} else {
		return camera->currentState == camera->cameraStates[RE::CameraState::kFirstPerson];
	}
}

// Returns true if the player is in third person
const bool GameState::IsThirdPerson(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kThirdPerson] && !IsFirstPerson(camera);
}

// Returns true if the player has a weapon drawn and in third person
const bool GameState::IsThirdPersonCombat(const RE::Actor* player, const RE::PlayerCamera* camera) noexcept {
	return GameState::IsThirdPerson(camera) && GameState::IsWeaponDrawn(player);
}

// Returns true if a kill move is playing
const bool GameState::IsInKillMove(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kVATS];
}

// Returns true if the camera is tweening
const bool GameState::IsInTweenCamera(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kTween];
}

// Returns true if the camera is transitioning
const bool GameState::IsInCameraTransition(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kPCTransition];
}

// Returns true if the player is using an object
const bool GameState::IsInUsingObjectCamera(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kAnimated];
}

// Returns true if the camera is in auto vanity mode
const bool GameState::IsInAutoVanityCamera(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kAutoVanity];
}

// Returns true if the camera is in free mode
const bool GameState::IsInFreeCamera(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kFree];
}

// Returns true if the camera is in aiming mode
const bool GameState::IsInAimingCamera(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kIronSights];
}

// Returns true if the camera is in furniture mode
const bool GameState::IsInFurnitureCamera(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kFurniture];
}

// Returns true if the player is riding a horse
const bool GameState::IsInHorseCamera(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kMount] && !IsFirstPerson(camera);
}

// Returns true if the player is bleeding out
const bool GameState::IsInBleedoutCamera(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kBleedout];
}

// Returns true if the player is riding a dragon
const bool GameState::IsInDragonCamera(const RE::PlayerCamera* camera) noexcept {
	return camera->currentState == camera->cameraStates[RE::CameraState::kDragon];
}

const bool GameState::IsInDialogue() noexcept {
	return RE::MenuTopicManager::GetSingleton()->unkB1 != 0;
}

RE::NiPointer<RE::TESObjectREFR> GameState::GetDialogueTarget() noexcept {
	return RE::MenuTopicManager::GetSingleton()->speaker.get();
}

const GameState::CameraState GameState::GetCameraState(const RE::Actor* player, const RE::PlayerCamera* camera) noexcept {
	GameState::CameraState newState = GameState::CameraState::Unknown;

	if (GameState::IsInDialogue() && GameState::IsThirdPerson(camera)) {
		newState = CameraState::ThirdPersonDialogue;
	} else if (GameState::IsSleeping(player)) {
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

#ifdef DEBUG
	assert(newState != GameState::CameraState::Unknown);
#endif
	return newState;
}

const bool GameState::IsWeaponDrawn(const RE::Actor* player) noexcept {
	const auto bits = GameState::GetPlayerActionBits(player);
	return bits[6];
}

// Get an equipped weapon
const RE::TESObjectWEAP* GameState::GetEquippedWeapon(const RE::Actor* player, bool leftHand) noexcept {
	if(!player->currentProcess) return nullptr;

	RE::TESForm* wep = nullptr;
	if(leftHand)
		wep = player->currentProcess->equippedObjects[RE::AIProcess::Hand::kLeft];
	else
		wep = player->currentProcess->equippedObjects[RE::AIProcess::Hand::kRight];

	if (!wep || !wep->IsWeapon()) return nullptr;
	return reinterpret_cast<const RE::TESObjectWEAP*>(wep);
}

float GameState::GetCurrentBowDrawTimer(const RE::PlayerCharacter* player) noexcept {
	// @Note: Read projectile->unk188
	// This is the arrow draw duration/shot power, stored on the player in a small array used as a stack
	// This grows, with the top value being the current timer - until an arrow is fired, clearing the stack
	const auto arr = reinterpret_cast<const PlayerArrayBA0*>(&(player->unkBA0));
	float drawTimerValue = arr->size > 0 ? glm::min(arr->top().bowDrawTime, 1.0f) : 1.0f;
	if (!mmath::IsValid(drawTimerValue)) drawTimerValue = 1.0f;
	return drawTimerValue;
}

bool GameState::IsUsingMagicItem(const RE::Actor* player, bool leftHand) noexcept {
	if (leftHand && !player->selectedSpells[RE::Actor::SlotTypes::kLeftHand]) return false;
	if (!leftHand && !player->selectedSpells[RE::Actor::SlotTypes::kRightHand]) return false;

	const RE::EnchantmentItem* ench = leftHand ?
		skyrim_cast<RE::EnchantmentItem*>(player->selectedSpells[RE::Actor::SlotTypes::kLeftHand]) :
		skyrim_cast<RE::EnchantmentItem*>(player->selectedSpells[RE::Actor::SlotTypes::kRightHand]);

	if (!ench) return false;
	const auto wep = GetEquippedWeapon(player, leftHand);
	if (!wep) return false;
	
	// Just look for a staff right now, consider anything else to be non-magic
	return wep->weaponData.animationType == RE::WEAPON_TYPE::kStaff;
}

bool GameState::IsCombatMagic(const RE::MagicItem* spell) noexcept {
	if (!spell) return false;

	const auto spellType = spell->GetSpellType();
	if (spellType != RE::MagicSystem::SpellType::kSpell &&
		spellType != RE::MagicSystem::SpellType::kLeveledSpell &&
		spellType != RE::MagicSystem::SpellType::kScroll)
		return false;

	// On self
	if (spell->GetDelivery() == RE::MagicSystem::Delivery::kSelf)
		return false;

	// Constant effect
	if (spell->GetCastingType() == RE::MagicSystem::CastingType::kConstantEffect)
		return false;

	return true;
}

// Returns true if the player has a melee weapon equiped
const bool GameState::IsMeleeWeaponDrawn(const RE::Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	// Continue to look for a staff here - consider a non-enchanted staff to be melee
	if (right) {
		if (right->weaponData.animationType != RE::WEAPON_TYPE::kBow &&
			right->weaponData.animationType != RE::WEAPON_TYPE::kCrossbow)
		{
			return true;
		}
	}

	if (left) {
		if (left->weaponData.animationType != RE::WEAPON_TYPE::kBow &&
			left->weaponData.animationType != RE::WEAPON_TYPE::kCrossbow)
		{
			return true;
		}
	}

	return !right && !left; // hand-to-hand
}

// Returns true if the player has magic drawn
const bool GameState::IsMagicDrawn(const RE::Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	
	// Using an item that counts as magic
	if (GameState::IsUsingMagicItem(player))
		return true;

	if (GameState::IsUsingMagicItem(player, true))
		return true;

	// Or a spell
	return (GameState::IsCombatMagic(player->selectedSpells[RE::Actor::SlotTypes::kLeftHand]) && !GetEquippedWeapon(player, true)) ||
		(GameState::IsCombatMagic(player->selectedSpells[RE::Actor::SlotTypes::kRightHand]) && !GetEquippedWeapon(player));
}

// Returns true if the player has a ranged weapon drawn
const bool GameState::IsRangedWeaponDrawn(const RE::Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	if (right) {
		if (right->weaponData.animationType == RE::WEAPON_TYPE::kBow ||
			right->weaponData.animationType == RE::WEAPON_TYPE::kCrossbow)
		{
			return true;
		}
	}

	if (left) {
		if (left->weaponData.animationType == RE::WEAPON_TYPE::kBow ||
			left->weaponData.animationType == RE::WEAPON_TYPE::kCrossbow)
		{
			return true;
		}
	}

	return false;
}

const bool GameState::IsUsingCrossbow(const RE::Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	if (right && (right->weaponData.animationType == RE::WEAPON_TYPE::kCrossbow))
		return true;

	if (left && (left->weaponData.animationType == RE::WEAPON_TYPE::kCrossbow))
		return true;

	return false;
}

const bool GameState::IsUsingBow(const RE::Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	if (right && (right->weaponData.animationType == RE::WEAPON_TYPE::kBow))
		return true;

	if (left && (left->weaponData.animationType == RE::WEAPON_TYPE::kBow))
		return true;

	return false;
}

// Returns true if the player is sneaking
const bool GameState::IsSneaking(const RE::Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[9];
}

// Returns true if the player is sprinting
const bool GameState::IsSprinting(const RE::Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[8];
}

// Returns true if the player is running
const bool GameState::IsRunning(const RE::Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return ((movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[7]) && !IsOverEncumbered(player);
}

// Returns true if the player is swimming
const bool GameState::IsSwimming(const RE::Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[10];
}

// Returns true if the player is walking
const bool GameState::IsWalking(const RE::Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		(movementBits[6] || (movementBits[7] && IsOverEncumbered(player)));
}

// Returns true if the player is holding a bow and an arrow is drawn
const bool GameState::IsBowDrawn(const RE::Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);

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
			lastDrawnTimer = static_cast<float>(GameTime::CurTime()) + 0.1f;
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
const bool GameState::IsSitting(const RE::Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[14] && movementBits[15];
}

// Returns true if the player is sleeping
const bool GameState::IsSleeping(const RE::Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (GameState::IsSitting(player) && movementBits[16]) || // in bed
		movementBits[15] && movementBits[16] || //getting in bed
		movementBits[17]; // getting out of bed
}

// Returns true if the player is mounting a horse
const bool GameState::IsMountingHorse(const RE::Actor* player) noexcept {
	const auto actionBits = GameState::GetPlayerActionBits(player);
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (actionBits[3] && actionBits[12] && movementBits[15] || (
		actionBits[12] && movementBits[15]
	)) && !movementBits[14];
}

// Returns true if the player is dismounting a horse
const bool GameState::IsDisMountingHorse(const RE::Actor* player) noexcept {
	const auto actionBits = GameState::GetPlayerActionBits(player);
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return actionBits[3] && actionBits[12] && (movementBits[16]);
}

const bool GameState::InPOVSlideMode() noexcept {
	return RE::PlayerControls::GetSingleton()->data.fovSlideMode; // unk04B;
}

// Returns true if the player is a vampire lord
const bool GameState::IsVampireLord(const RE::Actor* player) noexcept {
	if (!player->race) return false;
	return strcmp(player->race->fullName.c_str(), "Vampire Lord") == 0;
}

// Returns true if the player is a werewolf
const bool GameState::IsWerewolf(const RE::Actor* player) noexcept {
	if (!player->race) return false;
	return strcmp(player->race->fullName.c_str(), "Werewolf") == 0;
}

const bool GameState::IsOverEncumbered(const RE::Actor* player) noexcept {
	typedef bool(*getter)(const RE::Actor*);
	return REL::Relocation<getter>(g_Offsets->IsOverEncumbered)(player);
}

bool GameState::IsActorTalking(RE::Actor* actor) noexcept {
	const auto bits = eastl::bitset<32>(actor->boolBits.underlying());
	return !(bits[7] && // kSoundFileDone
			bits[8]) && // kVoiceFileDone
			actor->voiceTimer > 0.0f; // voiceTimer
}

glm::vec4 GameState::GetBoundingSphere(RE::NiAVObject* obj) noexcept {
	return *reinterpret_cast<glm::vec4*>(&obj->worldBound);
}