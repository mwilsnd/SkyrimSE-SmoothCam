#pragma once
#include "pch.h"
#include "camera_state.h"
#include "camera_states/thirdperson.h"
#include "camera_states/thirdperson_combat.h"
#include "camera_states/thirdperson_horse.h"

namespace Camera {
	// Selects the camera update method to run
	// These are the base states for selecting camera functionallity
	// All other CameraActionState values modifiy these
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

	// Helps describe more about the current camera and player state
	// Used for selecting camera offsets
	enum class CameraActionState {
		Standing,				// Player is simply standing still
		Walking,				// Player is walking
		Running,				// Player is running
		Sprinting,				// Player is sprinting
		Sneaking,				// Player is sneaking
		Swimming,				// Player is swimming
		SittingTransition,		// Player is sitting down or standing up		!-> Unused
		Sitting,				// Player is sitting (In a chair, on a horse)
		FirstPersonSitting,		// A special mode for improved camera with custom transition rules
		Sleeping,				// Player is in a bed
		Aiming,					// Player is aiming with a bow
		Mounting,				// Player is mounting a horse					!-> Unused
		DisMounting,			// Player is dismounting a horse
		FirstPersonHorseback,	// A special mode for improved camera with custom transition rules
		FirstPersonDragon,		// A special mode for improved camera with custom transition rules
		Unknown,				// State is not known
	};

	const auto UNIT_FORWARD = glm::vec3(1.0f, 0.0f, 0.0f);
	const auto UNIT_RIGHT = glm::vec3(0.0f, 1.0f, 0.0f);
	const auto UNIT_UP = glm::vec3(0.0f, 0.0f, 1.0f);
	constexpr auto SKYRIM_MIN_ZOOM_FRACTION = 0.2f;

#ifdef _DEBUG
	constexpr auto knownMovementBits = mapbox::eternal::map<byte, bool>({
		{ 0, true },    // appears to mean moving backwards, sometimes
		{ 1, true },    // appears to mean moving forwards, sometimes
		{ 2, true },    // indicates forward or diagonal movement
		{ 3, true },    // indicates forward or diagonal movement
		{ 4, true },    // Shows up but unknown
		{ 5, true },    // unknown but always set when walking
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
		// bit 23, 24 unknown - yet to observe being set 
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

	class SmoothCamera /*: public std::enable_shared_from_this<SmoothCamera>*/ {
		public:
			SmoothCamera() noexcept;
			SmoothCamera(const SmoothCamera&) = delete;
			SmoothCamera(SmoothCamera&&) noexcept = delete;
			SmoothCamera& operator=(const SmoothCamera&) = delete;
			SmoothCamera& operator=(SmoothCamera&&) noexcept = delete;
			~SmoothCamera() = default;

		public:
			// Selects the correct update method and positions the camera
			void UpdateCamera(const PlayerCharacter* player, CorrectedPlayerCamera* camera);
			// Called when the player toggle's the POV
			void OnTogglePOV(const ButtonEvent* ev) noexcept;

		private:
			double GetTime() const noexcept;
			float GetFrameDelta() const noexcept;

			// Returns the bits for player->actorState->flags04 which appear to convey movement info
			const std::bitset<32> GetPlayerMovementBits(const PlayerCharacter* player) const noexcept;
			// Returns the bits for player->actorState->flags08 which appear to convey action info
			const std::bitset<32> GetPlayerActionBits(const PlayerCharacter* player) const noexcept;

			/// Camera state detection
			// Returns true if the player is in first person
			const bool IsFirstPerson(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the player is in third person
			const bool IsThirdPerson(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the player has a weapon drawn and in third person
			const bool IsThirdPersonCombat(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if a kill move is playing
			const bool IsInKillMove(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the camera is tweening
			const bool IsInTweenCamera(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the camera is transitioning
			const bool IsInCameraTransition(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the player is using an object
			const bool IsInUsingObjectCamera(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the camera is in auto vanity mode
			const bool IsInAutoVanityCamera(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the camera is in free mode
			const bool IsInFreeCamera(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the camera is in aiming mode
			const bool IsInAimingCamera(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the camera is in furniture mode
			const bool IsInFurnitureCamera(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the player is riding a horse
			const bool IsInHorseCamera(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the player is bleeding out
			const bool IsInBleedoutCamera(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns true if the player is riding a dragon
			const bool IsInDragonCamera(const CorrectedPlayerCamera* camera) const noexcept;

			// Updates our POV state to the true value the game expects for each state
			const bool UpdateCameraPOVState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept;

			/// Player action states
			const bool IsWeaponDrawn(const PlayerCharacter* player) const noexcept;
			// Returns true if the player is sneaking
			const bool IsSneaking(const PlayerCharacter* player) const noexcept;
			// Returns true if the player is sprinting
			const bool IsSprinting(const PlayerCharacter* player) const noexcept;
			// Returns true if the player is swimming
			const bool IsSwimming(const PlayerCharacter* player) const noexcept;
			// Returns true if the player is walking
			const bool IsWalking(const PlayerCharacter* player) const noexcept;
			// Returns true if the player is running
			const bool IsRunning(const PlayerCharacter* player) const noexcept;
			// Returns true if the player is holding a bow and an arrow is drawn and in third person
			const bool IsBowDrawn(const PlayerCharacter* player) const noexcept;
			// Returns true if the player is sitting
			const bool IsSitting(const PlayerCharacter* player) const noexcept;
			// Returns true if the player is sleeping
			const bool IsSleeping(const PlayerCharacter* player) const noexcept;
			// Returns true if the player is mounting a horse
			const bool IsMountingHorse(const PlayerCharacter* player) const noexcept;
			// Returns true if the player is dismounting a horse
			const bool IsDisMountingHorse(const PlayerCharacter* player) const noexcept;

			/// Camera state updates
			// Returns the current camera state for use in selecting an update method
			const CameraState GetCurrentCameraState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera);
			// Returns the current camera action state for use in the selected update method
			const CameraActionState GetCurrentCameraActionState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept;

#ifdef _DEBUG
			// Triggers when the camera action state changes, for debugging
			void OnCameraActionStateTransition(const PlayerCharacter* player, const CameraActionState newState,
				const CameraActionState oldState) const noexcept;
#endif
			// Triggers when the camera state changes, for debugging
			void OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera, const CameraState newState,
				const CameraState oldState) const;


			/// Camera position calculations
			// Returns the zoom value set from the given camera state
			float GetCurrentCameraZoom(const CorrectedPlayerCamera* camera, const CameraState currentState) const noexcept;
			// Returns the camera height for the current player state
			float GetCurrentCameraHeight(const PlayerCharacter* player) const noexcept;
			// Returns the ideal camera distance for the current zoom level
			float GetCurrentCameraDistance(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the camera side offset for the current player state
			float GetCurrentCameraSideOffset(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the full local-space camera offset for the current player state
			glm::vec3 GetCurrentCameraOffset(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the full world-space camera target postion for the current player state
			glm::vec3 GetCurrentCameraTargetWorldPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const;
			// Returns the current smoothing scalar to use for the given distance to the player
			float GetCurrentSmoothingScalar(const float distance, bool zScalar = false) const;

			/// Crosshair stuff
			// Updates the screen position of the crosshair for correct aiming
			void UpdateCrosshairPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const;
			void SetCrosshairPosition(const glm::vec2& pos) const;

			/// Camera getters
			// Returns the camera's yaw
			float GetCameraYawRotation(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the camera's pitch
			float GetCameraPitchRotation(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the camera's current zoom level - Camera must extend ThirdPersonState
			float GetCameraZoomScalar(const CorrectedPlayerCamera* camera, uint16_t cameraState) const noexcept;

		private:
			std::array<std::unique_ptr<State::BaseCameraState>, static_cast<size_t>(CameraState::MAX_STATE)> cameraStates;

			Config::UserConfig* config = nullptr;
			CameraState currentState = CameraState::Unknown;
			CameraState lastState = CameraState::Unknown;
			CameraActionState currentActionState = CameraActionState::Unknown;
			CameraActionState lastActionState = CameraActionState::Unknown;

			glm::vec3 lastPosition = { 0.0f, 0.0f, 0.0f };
			glm::vec3 currentPosition = { 0.0f, 0.0f, 0.0f };
			bool firstFrame = false;
			bool povIsThird = false;
			bool povWasPressed = false;

			double curFrame = 0.0;
			double lastFrame = 0.0;
			double startTime = 0.0;

			friend class State::BaseCameraState;
	};
}