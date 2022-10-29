#pragma once
#include "camera.h"
#include "camera_states/base_third.h"
#include "camera_states/thirdperson/thirdperson.h"
#include "camera_states/thirdperson/thirdperson_dialogue.h"
#include "camera_states/thirdperson/thirdperson_vanity.h"
#include "crosshair.h"

#ifdef WITH_CHARTS
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
			virtual void OnBegin(RE::PlayerCharacter* player, RE::PlayerCamera* camera, ICamera* lastState) noexcept override;
			// Called when we are done using this camera
			virtual void OnEnd(RE::PlayerCharacter* player, RE::PlayerCamera* camera, ICamera* newState) noexcept override;
			// Runs before the internal game camera logic
			// Return true when changing the camera state
			virtual bool OnPreGameUpdate(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
				RE::BSTSmartPointer<RE::TESCameraState>& nextState) override;
			// Selects the correct update method and positions the camera
			virtual void OnUpdateCamera(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
				RE::BSTSmartPointer<RE::TESCameraState>& nextState) override;
			// Render crosshair objects
			virtual void Render(Render::D3DContext& ctx) noexcept override;
			// Called when the player toggles the POV
			virtual void OnTogglePOV(RE::ButtonEvent* ev) noexcept override;
			// Called when any other key is pressed
			virtual bool OnKeyPress(const RE::ButtonEvent* ev) noexcept override;
			// Called when a menu of interest is opening or closing
			virtual bool OnMenuOpenClose(MenuID id, const RE::MenuOpenCloseEvent* const ev) noexcept override;
			// Triggers when the camera action state changes
			virtual void OnCameraActionStateTransition(const RE::PlayerCharacter* player, const CameraActionState newState,
				const CameraActionState oldState) noexcept override;
			// Triggers when the camera state changes
			virtual bool OnCameraStateTransition(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
				const GameState::CameraState newState, const GameState::CameraState oldState) noexcept override;

			// Return true to block the input event handler from processing
			bool IsInputLocked(RE::TESCameraState* state) const noexcept;
			// Flag input for the given camera state as locked or unlocked
			void LockInputState(uint8_t stateID, bool locked) noexcept;
			// Run all interpolators for the frame
			void UpdateInterpolators(const RE::Actor* forRef, const RE::PlayerCamera* camera) noexcept;

			// Computes pitch zoom and returns it
			float GetPitchZoom() const noexcept;
			// Returns the full world-space camera target postion for the current player state
			glm::vec3 GetCurrentCameraTargetWorldPosition(const RE::TESObjectREFR* ref) const noexcept;
			// Returns the target world-space position for the camera, with local offsets and rotations applied. This is the goal
			// position ignoring any interpolation
			void GetCameraGoalPosition(glm::vec3& world, glm::vec3& local, const RE::TESObjectREFR* forRef = nullptr);
			// Return the euler angles for the player's current aim
			glm::vec2 GetAimRotation(const RE::TESObjectREFR* ref, const RE::PlayerCamera *camera) const;
			// Return the camera rotation
			const mmath::Rotation& GetCameraRotation() const noexcept;
			// Set the rotation of the camera
			void SetCameraRotation(mmath::Rotation& rot, RE::PlayerCamera* camera) noexcept;
			// Get the last camera position we set
			const mmath::Position& GetPosition() const noexcept;
			// Set the camera world position
			void SetPosition(const glm::vec3& pos, const RE::PlayerCamera* camera) noexcept;
			// Set the camera to the goal position and invalidate interp state
			void MoveToGoalPosition(const RE::PlayerCharacter* player, const RE::Actor* forRef,
				const RE::PlayerCamera* camera) noexcept;
			// True if shoulder swap is active
			bool IsShoulderSwapped() const noexcept;

			// Get the crosshair manager
			Crosshair::Manager* GetCrosshairManager() noexcept;
			// Find a node to use as the world position for following
			RE::NiAVObject* FindFollowBone(const RE::TESObjectREFR* ref, const eastl::string_view& filterBone = "") const noexcept;
			// Offset the gmae FOV by the given amount
			void SetFOVOffset(float fov, bool force = false) noexcept;
			// Return the current offset transition state, for mutation
			mmath::OffsetTransition& GetOffsetTransitionState() noexcept;
			// Return the current FOV transition state, for mutation
			mmath::FloatTransition& GetFOVTransitionState() noexcept;
			// Return the current Zoom transition state, for mutation
			mmath::FloatTransition& GetZoomTransitionState() noexcept;

		private:
			// Update the internal rotation
			void UpdateInternalRotation(const RE::PlayerCamera* camera) noexcept;

			// Returns the ideal camera distance for the current zoom level
			float GetCurrentCameraDistance(const RE::PlayerCamera* camera) const noexcept;
			// Returns the zoom value set from the given camera state
			float GetCurrentCameraZoom(const RE::PlayerCamera* camera) const noexcept;
			// Returns the camera's current zoom level - Camera must extend ThirdPersonState
			float GetCameraZoomScalar(const RE::PlayerCamera* camera, uint32_t cameraState) const noexcept;

			// Returns true if interpolation is allowed in the current state
			bool IsInterpAllowed(const RE::Actor* player) const noexcept;
			// Returns true if interpolation is overloaded for the current camera state
			bool IsInterpOverloaded(const RE::Actor* player) const noexcept;
			// Returns true if local-space interpolation is overloaded for the current camera state
			bool IsLocalInterpOverloaded(const RE::Actor* player) const noexcept;

			// Return the current user-defined scalar group parameters for overloading based on state
			const Config::OffsetGroupScalar* GetCurrentScalarGroup(const RE::Actor* player,
				const Config::OffsetGroup* currentGroup) const noexcept;
			// Returns an offset group for the current player movement state
			const Config::OffsetGroup* GetOffsetForState(const CameraActionState state) const noexcept;

			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateForwardOffset(const RE::Actor* player, const Config::OffsetGroup* group) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateUpOffset(const RE::Actor* player, const Config::OffsetGroup* group) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateSideOffset(const RE::Actor* player, const Config::OffsetGroup* group) const noexcept;
			// Selects the right offset from an offset group for the player's weapon state
			float GetActiveWeaponStateFOVOffset(const RE::Actor* player, const Config::OffsetGroup* group) const noexcept;

			// Returns the camera zoom for the current player state
			float GetCurrentCameraForwardOffset(const RE::Actor* player) const noexcept;
			// Returns the camera height for the current player state
			float GetCurrentCameraHeight(const RE::Actor* player) const noexcept;
			// Returns the camera side offset for the current player state
			float GetCurrentCameraSideOffset(const RE::Actor* player) const noexcept;
			// Returns the camera FOV offset for the current player state
			float GetCurrentCameraFOVOffset(const RE::Actor* player) const noexcept;

			// Returns the full local-space camera offset for the current player state, FOV is packed in .w
			glm::vec4 GetCurrentCameraOffset(const RE::Actor* player) const noexcept;
			// Returns the current smoothing scalar to use for the given distance to the player
			double GetCurrentSmoothingScalar(const RE::Actor* player, const float distance, ScalarSelector method = ScalarSelector::Normal) const;
			// Returns the user defined distance clamping vector pair
			std::tuple<glm::vec3, glm::vec3> GetDistanceClamping() const noexcept;

			// Track the thirdperson state for a POV switch and update as required
			bool UpdatePOVSwitchState(RE::PlayerCamera* camera, uint16_t cameraState) noexcept;
			// Update offset smoothers
			void UpdateOffsetSmoothers(const RE::Actor* player, float curTime) noexcept;

		private:
			struct {
				mutable RE::BSFixedString head = "NPC Head [Head]";
				mutable RE::BSFixedString spine1 = "NPC Spine1 [Spn1]";
			} Strings;

			// User config
			Config::UserConfig* config = nullptr;
			// Crosshair manager
			eastl::unique_ptr<Crosshair::Manager> crosshair = nullptr;
			// Thirdperson state
			eastl::unique_ptr<State::ThirdpersonState> thirdPersonState = nullptr;
			// Thirdperson dialogue
			eastl::unique_ptr<State::ThirdpersonDialogueState> thirdPersonDialogueState = nullptr;
			// Thirdperson vanity
			eastl::unique_ptr<State::ThirdpersonVanityState> thirdPersonVanityState = nullptr;
			// All states
			eastl::array<State::BaseThird*, static_cast<size_t>(GameState::CameraState::MAX_STATE)> cameraStates{};
			// Current camera state
			State::BaseThird* runningState = nullptr;
			// Current actor being followed
			RE::Actor* currentFocusObject = nullptr;
			// Input state lockers
			eastl::array<bool, RE::CameraState::kTotal> inputLockers{};

			// The current rotation of the camera in both euler angles and in quaternion form
			mmath::Rotation rotation{};
			// The last position of the camera we set
			mmath::Position lastPosition{};
			// Our most current camera position we set
			mmath::Position currentPosition{};

			// Data to be saved and restored across certain state transitions
			struct {
				// Yaw rotation for the horse state - We need to restore this after moving from tween->horse
				float horseYaw = 0.0f;
				// Store zoom value when starting a pov switch
				float savedZoomValue = 0.0f;
				// The last zoom scalar observed
				float lastZoomScalar = 0.0f;
				// After getting off a horse, jump to the camera goal position
				bool wantMoveToGoal = false;
			} stateCopyData;

			// Our current offset group and offset position, set by the offset transition states
			struct {
				const Config::OffsetGroup* currentGroup = nullptr;
				glm::vec3 position = { 0.0f, 0.0f, 0.0f };
				float zoom = 0.0f;
				float fov = 0.0f;
			} offsetState;

			// Transition groups for smoothing offset and zoom switches
			// Smooth x, y components of the active offset group
			mmath::OffsetTransition offsetTransitionState{};
			// Smooth z of the active offset group
			mmath::FloatTransition zoomTransitionState{};
			// Smooth FOV of the active offset group
			mmath::FloatTransition fovTransitionState{};
			// Smooth POV switching
			struct {
				double startTime = 0.0;
				float lastValue = 0.0f;
				bool running = false;
			} povTransitionState;

			// Smooth changes in scalar state
			mmath::ScalarTweener globalSmoother{};
			mmath::ScalarTweener localSmoother{};
			// Store the global interp settings in a similar object just so we can lerp them easier
			Config::OffsetGroupScalar globalValues{};

			// Set on first execution to perform setup
			bool firstFrame = false;
			// Was the POV key pressed this frame?
			bool povWasPressed = false;
			// Last read zoom value from the third person state
			float lastZoomValue = -0.2f;
			// -1 when we have swapped shoulders
			int shoulderSwap = 1;

			// Debug overlays
#ifdef WITH_CHARTS
			eastl::unique_ptr<Render::CBuffer> perFrameBuffer = nullptr;

			eastl::unique_ptr<Render::LineGraph> graph_worldPosTarget = nullptr;
			eastl::unique_ptr<Render::LineGraph> graph_offsetPos = nullptr;
			eastl::unique_ptr<Render::LineGraph> graph_targetOffsetPos = nullptr;
			eastl::unique_ptr<Render::LineGraph> graph_localSpace = nullptr;
			eastl::unique_ptr<Render::LineGraph> graph_rotation = nullptr;
			eastl::unique_ptr<Render::LineGraph> graph_tpsRotation = nullptr;
			eastl::unique_ptr<Render::LineGraph> graph_computeTime = nullptr;
			eastl::unique_ptr<Render::LineGraph> graph_fov = nullptr;

			eastl::unique_ptr<Render::NiNodeTreeDisplay> focusTargetNodeTree = nullptr;
			eastl::unique_ptr<Render::StateOverlay> stateOverlay = nullptr;

			enum class DisplayMode : uint8_t {
				None,
				Graphs,
				NodeTree,
				StateOverlay
			};
			DisplayMode curDebugMode = DisplayMode::None;
			bool dbgKeyDown = false;
			float lastProfSnap = 0.0f;
			float desiredFOV = 0.0f;
			glm::mat4 orthoMatrix = {};

			friend class Render::StateOverlay;
#endif

			friend class State::BaseThird;
	};
}