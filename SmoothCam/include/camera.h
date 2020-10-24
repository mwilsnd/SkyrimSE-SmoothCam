#pragma once
#include "camera_state.h"
#include "camera_states/thirdperson.h"
#include "camera_states/thirdperson_combat.h"
#include "camera_states/thirdperson_horse.h"
#include "crosshair.h"

#ifdef WITH_CHARTS
#   include "render/line_graph.h"
#   include "render/cbuffer.h"
#   include "render/ninode_tree_display.h"
#endif

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

	// Used to select which scalar function type should be run
	enum class ScalarSelector {
		Normal,
		SepZ,
		LocalSpace,
	};

	enum class MenuID {
		None,
		DialogMenu,
		LoadingMenu,
		MistMenu,
		FaderMenu,
		LoadWaitSpinner,
	};

	class SmoothCamera {
		public:
			SmoothCamera();
			~SmoothCamera();
			SmoothCamera(const SmoothCamera&) = delete;
			SmoothCamera(SmoothCamera&&) noexcept = delete;
			SmoothCamera& operator=(const SmoothCamera&) = delete;
			SmoothCamera& operator=(SmoothCamera&&) noexcept = delete;
			
		public:
			// Runs before the internal game camera logic
			// Return true when changing the camera state
			bool PreGameUpdate(PlayerCharacter* player, CorrectedPlayerCamera* camera, BSTSmartPointer<TESCameraState>& nextState);
			// Selects the correct update method and positions the camera
			void UpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera, BSTSmartPointer<TESCameraState>& nextState);

			// Called when the player toggles the POV
			void OnTogglePOV(const ButtonEvent* ev) noexcept;
			// Called when any other key is pressed
			void OnKeyPress(const ButtonEvent* ev) noexcept;
			// Called when a menu of interest is opening or closing
			void OnMenuOpenClose(MenuID id, const MenuOpenCloseEvent* const ev) noexcept;

			// Returns the current camera state for use in selecting an update method
			const GameState::CameraState GetCurrentCameraState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera);
			// Returns the current camera action state for use in the selected update method
			const CameraActionState GetCurrentCameraActionState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept;

			// Returns our most recent camera position
			glm::vec3 GetCurrentPosition() const noexcept;
			// Set the camera world position
			void SetPosition(const glm::vec3& pos, const CorrectedPlayerCamera* camera) noexcept;
			// Find a node to use as the world position for following
			NiAVObject* FindFollowBone(const TESObjectREFR* ref) const noexcept;
			// Returns the full world-space camera target postion for the current player state
			glm::vec3 GetCurrentCameraTargetWorldPosition(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) const;
			// Return the euler angles for the player's current aim
			glm::vec2 GetAimRotation(const TESObjectREFR* ref, const CorrectedPlayerCamera * camera) const;
			// Get the object the camera is currently focused on
			NiPointer<TESObjectREFR> GetCurrentCameraTarget(const CorrectedPlayerCamera* camera) noexcept;
			// Returns true if a loading screen is active
			bool InLoadingScreen() const noexcept;

		private:
			// Update skyrim's screen projection matrix
			void UpdateInternalWorldToScreenMatrix(const CorrectedPlayerCamera* camera, NiCamera* cameraNi,
				float pitch, float yaw) noexcept;
			// Updates our POV state to the true value the game expects for each state
			const bool UpdateCameraPOVState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept;

			/// Camera state updates
			// Check if the camera is near the player's head (for first person mods)
			bool CameraNearHead(const PlayerCharacter* player, const CorrectedPlayerCamera* camere, float cutOff = 32.0f);
			bool IFPV_InFirstPersonState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera);

#ifdef _DEBUG
			// Triggers when the camera action state changes, for debugging
			void OnCameraActionStateTransition(const PlayerCharacter* player, const CameraActionState newState,
				const CameraActionState oldState) const noexcept;
#endif
			// Triggers when the camera state changes
			void OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera, const GameState::CameraState newState,
				const GameState::CameraState oldState);

			/// Camera position calculations
			// Returns the zoom value set from the given camera state
			float GetCurrentCameraZoom(const CorrectedPlayerCamera* camera, const GameState::CameraState currentState) const noexcept;
			// Returns an offset group for the current player movement state
			const Config::OffsetGroup* GetOffsetForState(const CameraActionState state) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateZoomOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateUpOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateSideOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept;
			//Returns the camera zoom for the current player state
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
			glm::mat4 GetViewMatrix() const noexcept;
			// Update the internal rotation
			void UpdateInternalRotation(CorrectedPlayerCamera* camera, NiCamera* cameraNi) noexcept;
			// Render crosshair objects
			void Render(Render::D3DContext& ctx);

			/// Camera getters
			// Returns the camera's yaw
			float GetCameraYawRotation() const noexcept;
			// Returns the camera's pitch
			float GetCameraPitchRotation() const noexcept;
			// Returns the camera's current zoom level - Camera must extend ThirdPersonState
			float GetCameraZoomScalar(const CorrectedPlayerCamera* camera, uint16_t cameraState) const noexcept;

		private:
			// User config
			Config::UserConfig* config = nullptr;
			// All camera state instances
			std::array<std::unique_ptr<State::BaseCameraState>, static_cast<size_t>(GameState::CameraState::MAX_STATE)> cameraStates;
			// Crosshair manager
			std::unique_ptr<Crosshair::Manager> crosshair;

			// The last and current camera state
			GameState::CameraState currentState = GameState::CameraState::Unknown;
			GameState::CameraState lastState = GameState::CameraState::Unknown;
			// And last and current camera action state
			CameraActionState currentActionState = CameraActionState::Unknown;
			CameraActionState lastActionState = CameraActionState::Unknown;
			// The ref object the camera is focused on
			TESObjectREFR* currentFocusObject = nullptr;
			
			struct {
				mutable BSFixedString weapon = "WEAPON";
				mutable BSFixedString head = "NPC Head [Head]";
				mutable BSFixedString npc = "NPC";
				mutable BSFixedString spine1 = "NPC Spine1 [Spn1]";
				mutable BSFixedString camera3rd = "Camera3rd [Cam3]";
			} Strings;

			// The position the base camera update method picked for the camera before we ran
			glm::vec3 gameInitialWorldPosition = { 0.0f, 0.0f, 0.0f };
			// Whatever position was last set before any camera update code is executed for the frame
			glm::vec3 gameLastActualPosition = { 0.0f, 0.0f, 0.0f };
			// The last position of the camera that WE set
			glm::vec3 lastPosition = { 0.0f, 0.0f, 0.0f };
			// The current position of the camera set by us
			glm::vec3 currentPosition = { 0.0f, 0.0f, 0.0f };
			// Our last position in player-local space
			glm::vec3 lastLocalPosition = { 0.0f, 0.0f, 0.0f };
			// Our last position in world space
			glm::vec3 lastWorldPosition = { 0.0f, 0.0f, 0.0f };
			// The current rotation of the camera in both euler angles and in quaternion form
			glm::vec2 currentRotation = { 0.0f, 0.0f };
			glm::quat currentQuat = glm::identity<glm::quat>();
			// Our current worldToScreen matrix for the hud
			mmath::NiMatrix44 worldToScaleform;
			// Our current view frustum
			NiFrustum frustum;

			// Transition groups for smoothing offset and zoom switches
			using OffsetTransition = mmath::TransitionGroup<glm::vec2>;
			using ZoomTransition = mmath::TransitionGroup<float>;
			// Smooth x, y components of the active offset group
			OffsetTransition offsetTransitionState;
			// Smooth z of the active offset group
			ZoomTransition zoomTransitionState;

			// Our current offset group and offset position, set by the offset transition states
			struct {
				const Config::OffsetGroup* currentGroup = nullptr;
				glm::vec3 position = { 0.0f, 0.0f, 0.0f };
			} offsetState;

			// Set on first execution to perform setup
			bool firstFrame = false;
			// Should we be in third person?
			bool povIsThird = false;
			// Was the POV key pressed this frame?
			bool povWasPressed = false;
			// Is the dialog menu open?
			bool dialogMenuOpen = false;
			// -1 when we have swapped shoulders
			int shoulderSwap = 1;
			// If not 0, we are in a loading screen sequence
			uint8_t loadScreenDepth = 0;

#ifdef WITH_CHARTS
			std::unique_ptr<Render::CBuffer> perFrameOrtho;
			std::unique_ptr<Render::LineGraph> worldPosTargetGraph;
			std::unique_ptr<Render::LineGraph> offsetPosGraph;
			std::unique_ptr<Render::LineGraph> offsetTargetPosGraph;
			std::unique_ptr<Render::LineGraph> localSpaceGraph;
			std::unique_ptr<Render::LineGraph> rotationGraph;
			std::unique_ptr<Render::LineGraph> computeTimeGraph;
			std::unique_ptr<Render::NiNodeTreeDisplay> refTreeDisplay;
			glm::mat4 orthoMatrix;
			float lastProfSnap = 0.0f;
#endif

			friend class State::BaseCameraState;
	};
}