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

	enum class CameraState : uint8_t {
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

	// When drawing an arrow, the game stores the amount of time passed since
	// starting the draw. This is used for arrow shot power calculations and
	// is stored in this format within a small array at PlayerCharacter::BA0.
	// Array appears to be used as a stack, with the most current draw time always being
	// the last entry. Size is reset when an arrow is fired but amusingly grows forever if
	// the player draws and cancels arrows repeatedly.
	//
	// Additionally, the eagle eye perk appears to mess with this timer, forcing it to a low value
	// causing arrows to be fired with less force - this appears to be a bug with the game.
	struct UnkBowDrawnTimerEntry {
		float bowDrawTime = 0.0f;
		float unk1 = 0.0f; // Haven't looked much into the following data, unsure what it is
		float unk2 = 0.0f;
		float unk3 = 0.0f;
	};

	template<typename T, size_t S>
	struct SSA {
		union Data {
			T* ptr;
			char ssa[sizeof(T) * S];
			T obj[S];
		};

		uint32_t capacity : 31;
		uint32_t local : 1;
		Data data;
		uint32_t size;

		inline const T& at(size_t index) const noexcept {
			if (local == 1) {
				return data.obj[index];
			} else {
				return data.ptr[index];
			}
		}

		inline const T& top() const noexcept {
			return at(size - 1);
		}
	};
	using PlayerArrayBA0 = SSA<UnkBowDrawnTimerEntry, 2>;

	// Returns the bits for player->actorState->flags04 which appear to convey movement info
	const eastl::bitset<32> GetPlayerMovementBits(const Actor* player) noexcept;
	// Returns the bits for player->actorState->flags08 which appear to convey action info
	const eastl::bitset<32> GetPlayerActionBits(const Actor* player) noexcept;

	// Check if the camera is near the player's head (for first person mods)
	const bool IC_InFirstPersonState() noexcept;
	const bool IFPV_InFirstPersonState() noexcept;

	/// Camera state detection
	// Returns true if the player is in first person
	const bool IsFirstPerson(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the player is in third person
	const bool IsThirdPerson(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the player has a weapon drawn and in third person
	const bool IsThirdPersonCombat(const Actor* player, const CorrectedPlayerCamera* camera) noexcept;
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
	const bool IsInHorseCamera(const TESObjectREFR* player, const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the player is bleeding out
	const bool IsInBleedoutCamera(const CorrectedPlayerCamera* camera) noexcept;
	// Returns true if the player is riding a dragon
	const bool IsInDragonCamera(const CorrectedPlayerCamera* camera) noexcept;
	// Get the current camera state
	const CameraState GetCameraState(const Actor* player, const CorrectedPlayerCamera* canera) noexcept;

	/// Player action states
	const bool IsWeaponDrawn(const Actor* player) noexcept;
	// Get an equipped weapon
	const TESObjectWEAP* GetEquippedWeapon(const Actor* player, bool leftHand = false) noexcept;
	// Get currently equipped ammo
	const TESAmmo* GetCurrentAmmo(const Actor* player) noexcept;
	// Get the amount of time the player has been holding an arrow back in the bow
	float GetCurrentBowDrawTimer(const PlayerCharacter* player) noexcept;
	// Returns true if the player is holding an enchanted item that counts as 'magic' in the given hand
	bool IsUsingMagicItem(const Actor* player, bool leftHand = false) noexcept;
	// Returns true if the given spell counts as "combat" magic
	bool IsCombatMagic(const SpellItem* spell) noexcept;
	// Returns true if the player has a melee weapon equiped
	const bool IsMeleeWeaponDrawn(const Actor* player) noexcept;
	// Returns true if the player has magic drawn
	const bool IsMagicDrawn(const Actor* player) noexcept;
	// Returns true if the player has a ranged weapon drawn
	const bool IsRangedWeaponDrawn(const Actor* player) noexcept;
	// Returns true if a crossbow is drawn
	const bool IsUsingCrossbow(const Actor* player) noexcept;
	// Returns true if a bow is drawn
	const bool IsUsingBow(const Actor* player) noexcept;
	// Returns true if the player is sneaking
	const bool IsSneaking(const Actor* player) noexcept;
	// Returns true if the player is sprinting
	const bool IsSprinting(const Actor* player) noexcept;
	// Returns true if the player is swimming
	const bool IsSwimming(const Actor* player) noexcept;
	// Returns true if the player is walking
	const bool IsWalking(const Actor* player) noexcept;
	// Returns true if the player is running
	const bool IsRunning(const Actor* player) noexcept;
	// Returns true if the player is holding a bow and an arrow is drawn
	const bool IsBowDrawn(const Actor* player) noexcept;
	// Returns true if the player is sitting
	const bool IsSitting(const Actor* player) noexcept;
	// Returns true if the player is sleeping
	const bool IsSleeping(const Actor* player) noexcept;
	// Returns true if the player is mounting a horse
	const bool IsMountingHorse(const Actor* player) noexcept;
	// Returns true if the player is dismounting a horse
	const bool IsDisMountingHorse(const Actor* player) noexcept;
	// Returns true if we are in POV slide mode (Holding F key)
	const bool InPOVSlideMode() noexcept;
	// Returns true if the player is a vampire lord
	const bool IsVampireLord(const Actor* player) noexcept;
	// Returns true if the player is a werewolf
	const bool IsWerewolf(const Actor* player) noexcept;
	// Returns true if the player is over-encumbered
	const bool IsOverEncumbered(const Actor* player) noexcept;
}