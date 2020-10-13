#pragma once
#include "game_state.h"
#include "camera_state.h"
#include "camera_states/thirdperson.h"
#include "camera_states/thirdperson_combat.h"
#include "camera_states/thirdperson_horse.h"

namespace Camera {
	typedef void(*UpdateWorldToScreenMtx)(NiCamera*);

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

	enum class ScalarSelector {
		Normal,
		SepZ,
		LocalSpace,
	};

	const auto UNIT_FORWARD = glm::vec3(1.0f, 0.0f, 0.0f);
	const auto UNIT_RIGHT = glm::vec3(0.0f, 1.0f, 0.0f);
	const auto UNIT_UP = glm::vec3(0.0f, 0.0f, 1.0f);

	class SmoothCamera {
		public:
			SmoothCamera() noexcept;
			SmoothCamera(const SmoothCamera&) = delete;
			SmoothCamera(SmoothCamera&&) noexcept = delete;
			SmoothCamera& operator=(const SmoothCamera&) = delete;
			SmoothCamera& operator=(SmoothCamera&&) noexcept = delete;
			~SmoothCamera() = default;

		public:
			// Runs before the internal game camera logic
			void PreGameUpdate(PlayerCharacter* player, CorrectedPlayerCamera* camera);
			// Selects the correct update method and positions the camera
			void UpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera);
			// Called when the player toggles the POV
			void OnTogglePOV(const ButtonEvent* ev) noexcept;
			// Called when any other key is pressed
			void OnKeyPress(const ButtonEvent* ev) noexcept;
			// Called when the dialog menu is shown or hidden
			void OnDialogMenuChanged(const MenuOpenCloseEvent* const ev) noexcept;
			// Returns our most recent camera position
			glm::vec3 GetCurrentPosition() const noexcept;
			// Returns the full world-space camera target postion for the current player state
			glm::vec3 GetCurrentCameraTargetWorldPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const;
			// Set the camera world position
			void SetPosition(const glm::vec3& pos, const CorrectedPlayerCamera* camera) noexcept;

		private:
			void UpdateInternalWorldToScreenMatrix(NiCamera* camera, float pitch, float yaw) noexcept;

			// Updates our POV state to the true value the game expects for each state
			const bool UpdateCameraPOVState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept;

			/// Camera state updates
			// Check if the camera is near the player's head (for first person mods)
			bool CameraNearHead(const PlayerCharacter* player, const CorrectedPlayerCamera* camere, float cutOff = 32.0f);
			bool IFPV_InFirstPersonState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera);
			// Returns the current camera state for use in selecting an update method
			const GameState::CameraState GetCurrentCameraState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera);
			// Returns the current camera action state for use in the selected update method
			const CameraActionState GetCurrentCameraActionState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept;

#ifdef _DEBUG
			// Triggers when the camera action state changes, for debugging
			void OnCameraActionStateTransition(const PlayerCharacter* player, const CameraActionState newState,
				const CameraActionState oldState) const noexcept;
#endif
			// Triggers when the camera state changes
			void OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera, const GameState::CameraState newState,
				const GameState::CameraState oldState) const;


			/// Camera position calculations
			// Returns the zoom value set from the given camera state
			float GetCurrentCameraZoom(const CorrectedPlayerCamera* camera, const GameState::CameraState currentState) const noexcept;
			// Returns an offset group for the current player movement state
			const Config::OffsetGroup* GetOffsetForState(const CameraActionState state) const noexcept;
			// 
			float GetActiveWeaponStateZoomOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateUpOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateSideOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept;
			//
			float GetCurrentCameraZoomOffset(PlayerCharacter* player) const noexcept;
			// Returns the camera height for the current player state
			float GetCurrentCameraHeight(PlayerCharacter* player) const noexcept;
			// Returns the ideal camera distance for the current zoom level
			float GetCurrentCameraDistance(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the camera side offset for the current player state
			float GetCurrentCameraSideOffset(PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the full local-space camera offset for the current player state
			glm::vec3 GetCurrentCameraOffset(PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the current smoothing scalar to use for the given distance to the player
			double GetCurrentSmoothingScalar(const float distance, ScalarSelector method = ScalarSelector::Normal) const;
			// Returns the user defined distance clamping vector pair
			std::tuple<glm::vec3, glm::vec3> GetDistanceClamping() const noexcept;
			// Returns true if interpolation is allowed in the current state
			bool IsInterpAllowed(PlayerCharacter* player) const noexcept;
			// Constructs the view matrix for the camera
			glm::mat4 GetViewMatrix(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept;

			/// Crosshair stuff
			// Updates the screen position of the crosshair for correct aiming
			void UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* camera);
			// Read initial values for the crosshair during startup
			void ReadInitialCrosshairInfo();
			// Set the 3D crosshair position
			void SetCrosshairPosition(const glm::vec2& pos) const;
			// Center the position of the crosshair
			void CenterCrosshair() const;
			// Set the size of the 3D crosshair
			void SetCrosshairSize(const glm::vec2& size) const;
			// Show or hide the crosshair
			void SetCrosshairEnabled(bool enabled) const;

			/// Camera getters
			// Update the internal rotation
			void UpdateInternalRotation(CorrectedPlayerCamera* camera) noexcept;
			// Returns the camera's yaw
			float GetCameraYawRotation(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the camera's pitch
			float GetCameraPitchRotation(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the camera's current zoom level - Camera must extend ThirdPersonState
			float GetCameraZoomScalar(const CorrectedPlayerCamera* camera, uint16_t cameraState) const noexcept;

			// Run a transition state
			template<typename T, typename S>
			void UpdateTransitionState(double curTime, bool enabled, float duration, Config::ScalarMethods method,
				S& transitionState, const T& currentValue)
			{
				// Check our current offset and see if we need to run a transition 
				if (enabled) {
					if (currentValue != transitionState.targetPosition) {
						// Start the task
						if (transitionState.running)
							transitionState.lastPosition = transitionState.currentPosition;

						transitionState.running = true;
						transitionState.startTime = curTime;
						transitionState.targetPosition = currentValue;
					}

					if (transitionState.running) {
						// Update the transition smoothing
						const auto scalar = glm::clamp(
							static_cast<float>(curTime - transitionState.startTime) / glm::max(duration, 0.01f),
							0.0f, 1.0f
						);

						if (scalar < 1.0f) {
							transitionState.currentPosition = mmath::Interpolate<T, float>(
								transitionState.lastPosition,
								transitionState.targetPosition,
								mmath::RunScalarFunction<float>(method, scalar)
							);
						} else {
							transitionState.currentPosition = transitionState.targetPosition;
							transitionState.running = false;
							transitionState.lastPosition = transitionState.currentPosition;
						}
					} else {
						transitionState.lastPosition = transitionState.targetPosition =
							transitionState.currentPosition = currentValue;
					}
				} else {
					// Disabled
					transitionState.running = false;
					transitionState.lastPosition = transitionState.targetPosition =
						transitionState.currentPosition = currentValue;
				}
			}
			
		private:
			std::array<std::unique_ptr<State::BaseCameraState>, static_cast<size_t>(GameState::CameraState::MAX_STATE)> cameraStates;

			Config::UserConfig* config = nullptr;
			GameState::CameraState currentState = GameState::CameraState::Unknown;
			GameState::CameraState lastState = GameState::CameraState::Unknown;
			CameraActionState currentActionState = CameraActionState::Unknown;
			CameraActionState lastActionState = CameraActionState::Unknown;
			mmath::NiMatrix44 worldToScreen = {};

			float lastNearPlane = 0.0f;
			glm::vec3 gameInitialWorldPosition = { 0.0f, 0.0f, 0.0f };
			glm::vec3 gameLastActualPosition = { 0.0f, 0.0f, 0.0f };
			glm::vec3 lastPosition = { 0.0f, 0.0f, 0.0f };
			glm::vec3 currentPosition = { 0.0f, 0.0f, 0.0f };
			glm::vec3 lastLocalPosition = { 0.0f, 0.0f, 0.0f };
			glm::vec3 lastWorldPosition = { 0.0f, 0.0f, 0.0f };

			glm::vec2 currentRotation = { 0.0f, 0.0f };
			glm::quat currentQuat = glm::identity<glm::quat>();

			template<typename T>
			struct TransitionGroup {
				T lastPosition = {};
				T targetPosition = {};
				T currentPosition = {};
				bool running = false;
				double startTime = 0.0;
			};

			using OffsetTransition = TransitionGroup<glm::vec2>;
			using ZoomTransition = TransitionGroup<float>;
			OffsetTransition offsetTransitionState;
			ZoomTransition zoomTransitionState;

			struct {
				const Config::OffsetGroup* currentGroup = nullptr;
				glm::vec3 position = { 0.0f, 0.0f, 0.0f };
			} offsetState;

			struct {
				bool captured = false;
				double xOff = 0.0;
				double yOff = 0.0;
				double xScale = 0.0;
				double yScale = 0.0;
				double xCenter = 0.0;
				double yCenter = 0.0;
			} baseCrosshairData;

			bool firstFrame = false;
			bool povIsThird = false;
			bool povWasPressed = false;
			bool dialogMenuOpen = false;
			int shoulderSwap = 1;

			friend class State::BaseCameraState;
	};
}