#pragma once
#include "camera.h"
#include "camera_states/base_third.h"
#include "camera_states/thirdperson/thirdperson.h"
#include "camera_states/thirdperson/thirdperson_combat.h"
#include "camera_states/thirdperson/thirdperson_horse.h"
#include "crosshair.h"

#ifdef WITH_CHARTS
#   include "render/cbuffer.h"
#   include "render/line_graph.h"
#   include "render/state_overlay.h"
#   include "render/ninode_tree_display.h"
#endif

namespace Camera {
	// Used to select which scalar function type should be run
	enum class ScalarSelector {
		Normal,
		SepZ,
		LocalSpace,
	};

	// All thirdperson only logic (Aim rotation, interp, crosshair, so on)
	class Thirdperson : public ICamera {
		public:
			Thirdperson(Camera* baseCamera);
			Thirdperson(const Thirdperson&) = delete;
			Thirdperson(Thirdperson&&) noexcept = delete;
			Thirdperson& operator=(const Thirdperson&) = delete;
			Thirdperson& operator=(Thirdperson&&) noexcept = delete;

		public:
			virtual ~Thirdperson();

			// Called when we are switching to this camera
			virtual void OnBegin(PlayerCharacter* player, CorrectedPlayerCamera* camera, ICamera* lastState) noexcept override;
			// Called when we are done using this camera
			virtual void OnEnd(PlayerCharacter* player, CorrectedPlayerCamera* camera, ICamera* newState) noexcept override;

			// Runs before the internal game camera logic
			// Return true when changing the camera state
			virtual bool OnPreGameUpdate(PlayerCharacter* player, CorrectedPlayerCamera* camera,
				BSTSmartPointer<TESCameraState>& nextState) override;
			// Selects the correct update method and positions the camera
			virtual void OnUpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera,
				BSTSmartPointer<TESCameraState>& nextState) override;
			// Render crosshair objects
			virtual void Render(Render::D3DContext& ctx) noexcept override;

			// Called when the player toggles the POV
			virtual void OnTogglePOV(const ButtonEvent* ev) noexcept override;
			// Called when any other key is pressed
			virtual bool OnKeyPress(const ButtonEvent* ev) noexcept override;
			// Called when a menu of interest is opening or closing
			virtual bool OnMenuOpenClose(MenuID id, const MenuOpenCloseEvent* const ev) noexcept override;

			// Triggers when the camera action state changes
			virtual void OnCameraActionStateTransition(const PlayerCharacter* player, const CameraActionState newState,
				const CameraActionState oldState) noexcept override;
			// Triggers when the camera state changes
			virtual void OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
				const GameState::CameraState newState, const GameState::CameraState oldState) noexcept override;

			// Returns the full world-space camera target postion for the current player state
			glm::vec3 GetCurrentCameraTargetWorldPosition(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) const;
			// Returns the target world-space position for the camera, with local offsets and rotations applied. This is the goal
			// position ignoring any interpolation
			void GetCameraGoalPosition(const CorrectedPlayerCamera* camera, glm::vec3& world, glm::vec3& local);
			// Return the euler angles for the player's current aim
			glm::vec2 GetAimRotation(const TESObjectREFR* ref, const CorrectedPlayerCamera * camera) const;
			// Return the camera rotation
			const mmath::Rotation& GetCameraRotation() const noexcept;
			// Set the camera world position
			void SetPosition(const glm::vec3& pos, const CorrectedPlayerCamera* camera) noexcept;
			// Get the crosshair manager
			Crosshair::Manager* GetCrosshairManager() noexcept;

		private:
			// Set the camera to the goal position and invalidate interp state
			void MoveToGoalPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept;
			// Update the internal rotation
			void UpdateInternalRotation(CorrectedPlayerCamera* camera) noexcept;
			// Find a node to use as the world position for following
			NiAVObject* FindFollowBone(const TESObjectREFR* ref) const noexcept;

			// Returns the zoom value set from the given camera state
			float GetCurrentCameraZoom(const CorrectedPlayerCamera* camera,
				const GameState::CameraState currentState) const noexcept;
			// Returns the camera's current zoom level - Camera must extend ThirdPersonState
			float GetCameraZoomScalar(const CorrectedPlayerCamera* camera, uint16_t cameraState) const noexcept;
			// Returns true if interpolation is allowed in the current state
			bool IsInterpAllowed(const PlayerCharacter* player) const noexcept;	

			// Returns an offset group for the current player movement state
			const Config::OffsetGroup* GetOffsetForState(const CameraActionState state) const noexcept;

			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateZoomOffset(const PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateUpOffset(const PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateSideOffset(const PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateFOVOffset(const PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept;

			//Returns the camera zoom for the current player state
			float GetCurrentCameraZoomOffset(const PlayerCharacter* player) const noexcept;
			// Returns the camera height for the current player state
			float GetCurrentCameraHeight(const PlayerCharacter* player) const noexcept;
			// Returns the camera side offset for the current player state
			float GetCurrentCameraSideOffset(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the camera FOV offset for the current player state
			float GetCurrentCameraFOVOffset(const PlayerCharacter* player) const noexcept;

			// Returns the ideal camera distance for the current zoom level
			float GetCurrentCameraDistance(const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the full local-space camera offset for the current player state, FOV is packed in .w
			glm::vec4 GetCurrentCameraOffset(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept;
			// Returns the current smoothing scalar to use for the given distance to the player
			double GetCurrentSmoothingScalar(const float distance, ScalarSelector method = ScalarSelector::Normal) const;
			// Returns the user defined distance clamping vector pair
			std::tuple<glm::vec3, glm::vec3> GetDistanceClamping() const noexcept;

			// Offset the gmae FOV by the given amount
			void SetFOVOffset(float fov, bool force = false) noexcept;
			// Track the thirdperson state for a POV switch and update as required
			bool UpdatePOVSwitchState(CorrectedPlayerCamera* camera, uint16_t cameraState) noexcept;

		private:
			struct {
				mutable BSFixedString head = "NPC Head [Head]";
				mutable BSFixedString spine1 = "NPC Spine1 [Spn1]";
			} Strings;

			// User config
			Config::UserConfig* config = nullptr;
			// Crosshair manager
			eastl::unique_ptr<Crosshair::Manager> crosshair;
			// All camera state instances
			eastl::array<eastl::unique_ptr<State::BaseThird>, static_cast<size_t>(GameState::CameraState::MAX_STATE)> cameraStates;
			// Current actor being followed
			Actor* currentFocusObject = nullptr;

			// The current rotation of the camera in both euler angles and in quaternion form
			mmath::Rotation rotation;
			// The last position of the camera we set
			mmath::Position lastPosition;
			// Our most current camera position we set
			mmath::Position currentPosition;

			// Data to be saved and restored across certain state transitions
			struct {
				// Yaw rotation for the horse state - We need to restore this after moving from tween->horse
				float horseYaw = 0.0f;
				// ACC writes over pitch rotation of the player with an incorrect value (are we causing that?)
				// Store pitch when entering dialog and restore it after
				float accPitch = 0.0f;
			} stateCopyData;

			// Our current offset group and offset position, set by the offset transition states
			struct {
				const Config::OffsetGroup* currentGroup = nullptr;
				glm::vec3 position = { 0.0f, 0.0f, 0.0f };
				float fov = 0.0f;
			} offsetState;

			// Transition groups for smoothing offset and zoom switches
			using OffsetTransition = mmath::TransitionGroup<glm::vec2>;
			using ZoomTransition = mmath::TransitionGroup<float>;
			using POVTransition = mmath::FixedTransitionGoal<float>;
			// Smooth x, y components of the active offset group
			OffsetTransition offsetTransitionState;
			// Smooth z of the active offset group
			ZoomTransition zoomTransitionState;
			// Smooth FOV of the active offset group
			ZoomTransition fovTransitionState;
			// Smooth POV switching
			struct {
				double startTime = 0.0;
				float lastValue = 0.0f;
				bool running = false;
			} povTransitionState;

			// Set on first execution to perform setup
			bool firstFrame = false;
			// Was the POV key pressed this frame?
			bool povWasPressed = false;
			// Last read zoom value from the third person state
			float lastZoomValue = -0.2f;
			// Is the dialog menu open?
			bool dialogMenuOpen = false;
			// Was the dialog menu open last frame?
			bool wasDialogMenuOpen = false;
			// -1 when we have swapped shoulders
			int shoulderSwap = 1;

			// Debug overlays
#ifdef WITH_CHARTS
			eastl::unique_ptr<Render::CBuffer> perFrameBuffer;

			eastl::unique_ptr<Render::LineGraph> graph_worldPosTarget;
			eastl::unique_ptr<Render::LineGraph> graph_offsetPos;
			eastl::unique_ptr<Render::LineGraph> graph_targetOffsetPos;
			eastl::unique_ptr<Render::LineGraph> graph_localSpace;
			eastl::unique_ptr<Render::LineGraph> graph_rotation;
			eastl::unique_ptr<Render::LineGraph> graph_tpsRotation;
			eastl::unique_ptr<Render::LineGraph> graph_computeTime;

			eastl::unique_ptr<Render::NiNodeTreeDisplay> focusTargetNodeTree;
			eastl::unique_ptr<Render::StateOverlay> stateOverlay;

			enum class DisplayMode : uint8_t {
				None,
				Graphs,
				NodeTree,
				StateOverlay
			};
			DisplayMode curDebugMode = DisplayMode::None;
			bool dbgKeyDown = false;
			float lastProfSnap = 0.0f;
			glm::mat4 orthoMatrix = {};

			friend class Render::StateOverlay;
#endif

			friend class State::BaseThird;
	};
}