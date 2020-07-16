#pragma once

namespace GameState {
	#ifdef _DEBUG
		constexpr auto knownMovementBits = mapbox::eternal::map<byte, bool>({
			{ 0, true },    // backwards
			{ 1, true },    // forward
			{ 2, true },    // right
			{ 3, true },    // left
			{ 4, true },    // rotate left
			{ 5, true },    // rotate right
			{ 6, true },    // walking?
			{ 7, true },    // running?
			{ 8, true },    // sprinting?
			{ 9, true },    // sneaking
			{ 10, true },   // swimming
							// bits 11, 12, 13 are unknown - yet to observe them being set
			{ 14, true },   // sitting
			{ 15, true },   // sitting
			{ 16, true },   // with 14 & 15, sitting in bed/on side, 15 & 16 - getting in to bed
			{ 17, true },   // getting out of bed
							// bits 18, 19, 20 are unknown - yet to observe them being set
			{ 21, true },   // actor is about to die? could also be the death spin animation
			{ 22, true },   // actor is dead - could also be player is a ragdoll
			{ 23, true },	// Set when on the chopping block during intro - playing a cutsceen?
							// bit 24 unknown - yet to observe being set 
			{ 25, true },	// player knocked down
			{ 26, true },	// player knocked down, is a ragdoll?
			{ 27, true },	// player getting up
			{ 28, true },   // grabbing an arrow from the quiver?
			{ 29, true },   // placing the arrow in the bow? or drawing the arrow back?
			{ 30, true },   // drawing the arrow back or could just mean 'arrow in motion'
			{ 31, true },   // bow drawn
		});

		constexpr auto knownActionBits = mapbox::eternal::map<byte, bool>({
			// 3, 12 = no weapon
			{ 3, true },  // 3, 5, 6, 12 = bow/sword drawn
			{ 5, true },  // 5, 6, 12 = arrow nocked/sword power attack held
			{ 6, true },  // 3, 6, 12 = bow drawing
			{ 7, true },  // 3, 5, 7, 12 = bow/sword stored
			{ 8, true },  // zooming with bow
			{ 10, true }, // swing left?
			{ 11, true }, // swing right?
			{ 12, true }, // unknown
			{ 13, true }, // means hurt? or just got shot with an arrow? might mean player got staggered
						  // all other bits unknown - yet to observe them being set
		});
	#endif

	enum class CameraState {
		FirstPerson,
		ThirdPerson,
		ThirdPersonCombat,
		KillMove,
		Tweening,
		Transitioning,
		UsingObject,
		Vanity,
		Free,
		IronSights,
		Furniture,
		Horseback,
		Bleedout,
		Dragon,
		Unknown,
		MAX_STATE,
	};

	// Returns the bits for player->actorState->flags04 which appear to convey movement info
	const std::bitset<32> GetPlayerMovementBits(const PlayerCharacter* player) noexcept;
	// Returns the bits for player->actorState->flags08 which appear to convey action info
	const std::bitset<32> GetPlayerActionBits(const PlayerCharacter* player) noexcept;

	/// Camera state detection
	// Returns true if the player is in first person
	const bool IsFirstPerson(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the player is in third person
	const bool IsThirdPerson(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the player has a weapon drawn and in third person
	const bool IsThirdPersonCombat(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if a kill move is playing
	const bool IsInKillMove(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the camera is tweening
	const bool IsInTweenCamera(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the camera is transitioning
	const bool IsInCameraTransition(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the player is using an object
	const bool IsInUsingObjectCamera(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the camera is in auto vanity mode
	const bool IsInAutoVanityCamera(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the camera is in free mode
	const bool IsInFreeCamera(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the camera is in aiming mode
	const bool IsInAimingCamera(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the camera is in furniture mode
	const bool IsInFurnitureCamera(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the player is riding a horse
	const bool IsInHorseCamera(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the player is bleeding out
	const bool IsInBleedoutCamera(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the player is riding a dragon
	const bool IsInDragonCamera(const CorrectedPlayerCamera* camera) noexcept;

	const CameraState GetCameraState(const PlayerCharacter* player, const CorrectedPlayerCamera* canera) noexcept;

	/// Player action states
	const bool IsWeaponDrawn(const PlayerCharacter* player) noexcept;
	// Get an equipped weapon
	const TESObjectWEAP* GetEquippedWeapon(PlayerCharacter* player, bool leftHand = false) noexcept;
	// Returns true if the player has a melee weapon equiped
	const bool IsMeleeWeaponDrawn(PlayerCharacter* player) noexcept;
	// Returns true if the player has magic drawn
	const bool IsMagicDrawn(PlayerCharacter* player) noexcept;
	// Returns true if the player has a ranged weapon drawn
	const bool IsRangedWeaponDrawn(PlayerCharacter* player) noexcept;
	// Returns true if a crossbow is drawn
	const bool IsUsingCrossbow(PlayerCharacter* player) noexcept;
	// Returns true if a bow is drawn
	const bool IsUsingBow(PlayerCharacter* player) noexcept;
	// Returns true if the player is sneaking
	const bool IsSneaking(const PlayerCharacter* player) noexcept;
	// Returns true if the player is sprinting
	const bool IsSprinting(const PlayerCharacter* player) noexcept;
	// Returns true if the player is swimming
	const bool IsSwimming(const PlayerCharacter* player) noexcept;
	// Returns true if the player is walking
	const bool IsWalking(const PlayerCharacter* player) noexcept;
	// Returns true if the player is running
	const bool IsRunning(const PlayerCharacter* player) noexcept;
	// Returns true if the player is holding a bow and an arrow is drawn and in third person
	const bool IsBowDrawn(const PlayerCharacter* player) noexcept;
	// Returns true if the player is sitting
	const bool IsSitting(const PlayerCharacter* player) noexcept;
	// Returns true if the player is sleeping
	const bool IsSleeping(const PlayerCharacter* player) noexcept;
	// Returns true if the player is mounting a horse
	const bool IsMountingHorse(const PlayerCharacter* player) noexcept;
	// Returns true if the player is dismounting a horse
	const bool IsDisMountingHorse(const PlayerCharacter* player) noexcept;
}