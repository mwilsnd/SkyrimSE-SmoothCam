#pragma once
#include "camera_states/base_first.h"
#include "camera_states/base_third.h"
#include "trackir/trackir.h"

namespace Camera {
	class Thirdperson;
	class Firstperson;
	class Camera;

	// Helps describe more about the current camera and player state
	// Used for selecting camera offsets
	enum class CameraActionState : uint8_t {
		Standing,				// Player is simply standing still
		Walking,				// Player is walking
		Running,				// Player is running
		Sprinting,				// Player is sprinting
		Sneaking,				// Player is sneaking
		Swimming,				// Player is swimming
		SittingTransition,		// Player is sitting down or standing up
		Sitting,				// Player is sitting
		FirstPersonSitting,		// A special mode for improved camera with custom transition rules
		Sleeping,				// Player is in a bed
		Aiming,					// Player is aiming with a bow
		Mounting,				// Player is mounting a horse	!-> Unused
		DisMounting,			// Player is dismounting a horse
		Horseback,				// Player is riding a horse
		Dragon,					// Player is riding a dragon
		FirstPersonHorseback,	// A special mode for improved camera with custom transition rules
		FirstPersonDragon,		// A special mode for improved camera with custom transition rules
		VampireLord,			// Player is a vampire lord
		Werewolf,				// Player is a werewolf
		Vanity,					// Vanity camera is running
		Unknown,				// State is not known
		MAX_STATE
	};

	enum class MenuID : uint8_t {
		None,
		DialogMenu,
		LoadingMenu,
		MistMenu,
		FaderMenu,
		LoadWaitSpinner,
		MapMenu,
		InventoryMenu,
	};

	enum class CameraID : uint8_t {
		None,
		Firstperson,
		Thirdperson
	};

	// Interface to different cameras
	class ICamera {
		public:
			ICamera(Camera* baseCamera, CameraID id) : m_camera(baseCamera), m_id(id) {};
			virtual ~ICamera() {};

			// Called when we are switching to this camera
			virtual void OnBegin(RE::PlayerCharacter* player, RE::PlayerCamera* camera, ICamera* lastState) noexcept = 0;
			// Called when we are done using this camera
			virtual void OnEnd(RE::PlayerCharacter* player, RE::PlayerCamera* camera, ICamera* newState) noexcept = 0;

			// Runs before the internal game camera logic
			// Return true when changing the camera state
			virtual bool OnPreGameUpdate(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
				RE::BSTSmartPointer<RE::TESCameraState>& nextState) = 0;
			// Selects the correct update method and positions the camera
			virtual void OnUpdateCamera(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
				RE::BSTSmartPointer<RE::TESCameraState>& nextState) = 0;
			// Render crosshair objects
			virtual void Render(Render::D3DContext& ctx) noexcept = 0;

			// Called when the player toggles the POV
			virtual void OnTogglePOV(RE::ButtonEvent* ev) noexcept = 0;
			// Called when any other key is pressed
			virtual bool OnKeyPress(const RE::ButtonEvent* ev) noexcept = 0;
			// Called when a menu of interest is opening or closing
			virtual bool OnMenuOpenClose(MenuID id, const RE::MenuOpenCloseEvent* const ev) noexcept = 0;

			// Triggers when the camera action state changes
			virtual void OnCameraActionStateTransition(const RE::PlayerCharacter* player, const CameraActionState newState,
				const CameraActionState oldState) noexcept = 0;
			// Triggers when the camera state changes
			virtual bool OnCameraStateTransition(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
				const GameState::CameraState newState, const GameState::CameraState oldState) noexcept = 0;

		public:
			const CameraID m_id = CameraID::None;

		protected:
			Camera* m_camera = nullptr;

	};

	// Low level camera, holding both the thirdperson and firstperson cameras
	class Camera {
		public:
			Camera() noexcept;
			~Camera();
			Camera(const Camera&) = delete;
			Camera(Camera&&) noexcept = delete;
			Camera& operator=(const Camera&) = delete;
			Camera& operator=(Camera&&) noexcept = delete;
			
		public:
			// Runs before the internal game camera logic
			// Return true when changing the camera state
			bool PreGameUpdate(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
				RE::BSTSmartPointer<RE::TESCameraState>& nextState);
			// Selects the correct update method and positions the camera
			void UpdateCamera(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
				RE::BSTSmartPointer<RE::TESCameraState>& nextState);
			// Render crosshair objects
			void Render(Render::D3DContext& ctx);

			// Called when the player toggles the POV
			void OnTogglePOV(RE::ButtonEvent* ev) noexcept;
			// Called when any other key is pressed
			void OnKeyPress(const RE::ButtonEvent* ev) noexcept;
			// Called when a menu of interest is opening or closing
			void OnMenuOpenClose(MenuID id, const RE::MenuOpenCloseEvent* const ev) noexcept;
			// Called when player menu mode changes
			void OnMenuModeChange(bool isMenuMode) noexcept;

			// Return true to block the input event handler from processing
			bool IsInputLocked(RE::TESCameraState* state) noexcept;

			// Returns the current camera state for use in selecting an update method
			const GameState::CameraState GetCurrentCameraState()  const noexcept;
			// Returns the current camera action state for use in the selected update method
			const CameraActionState GetCurrentCameraActionState() const noexcept;

			// Set the camera world position
			void SetPosition(const glm::vec3& pos, const RE::PlayerCamera* camera, RE::NiCamera* niCamera = nullptr) noexcept;

			// Return the current frustum
			const RE::NiFrustum& GetFrustum() const noexcept;
			// Return the last recorded position of the camera set by the game
			const glm::vec3& GetLastRecordedCameraPosition() const noexcept;
			// Get the object the camera is currently focused on
			RE::NiPointer<RE::Actor> GetCurrentCameraTarget(const RE::PlayerCamera* camera) noexcept;
			// Returns true if a loading screen is active
			bool InLoadingScreen() const noexcept;
			// Returns true if the player is in menu mode
			bool InMenuMode() const noexcept;
			// Option for the MCM - Allow forcing the camera to a new state
			// Adding this after observing a bug - Camera got stuck in kCameraState_Furniture
			void SetShouldForceCameraState(bool force, uint8_t newCameraState) noexcept;

			// Get the thirdperson camera
			Thirdperson* GetThirdpersonCamera() noexcept;
			// Get the firstperson camera
			Firstperson* GetFirstpersonCamera() noexcept;

		private:
			// Update skyrim's screen projection matrix
			void UpdateInternalWorldToScreenMatrix(RE::NiCamera* niCamera = nullptr) noexcept;
			// Updates our POV state to the true value the game expects for each state
			const bool UpdateCameraPOVState(const RE::PlayerCamera* camera) noexcept;
			// Returns the current camera state for use in selecting an update method
			const GameState::CameraState UpdateCurrentCameraState(RE::PlayerCharacter* player,
				const RE::Actor* forRef, RE::PlayerCamera* camera) noexcept;
			// Returns the current camera action state for use in the selected update method
			const CameraActionState UpdateCurrentCameraActionState(const RE::PlayerCharacter* player,
				const RE::Actor* forRef, const RE::PlayerCamera* camera) noexcept;

			// Triggers when the camera action state changes
			void OnCameraActionStateTransition(const RE::PlayerCharacter* player, const CameraActionState newState,
				const CameraActionState oldState) const noexcept;
			// Triggers when the camera state changes
			bool OnCameraStateTransition(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
				const GameState::CameraState newState, const GameState::CameraState oldState) noexcept;

			RE::NiPointer<RE::NiCamera> GetNiCamera(RE::PlayerCamera* camera) const noexcept;

		private:
			Config::UserConfig* config = nullptr;               // User config
			RE::Actor* currentFocusObject = nullptr;            // The ref object the camera is focused on
			RE::NiPointer<RE::NiCamera> cameraNi = nullptr;     // Active NiCamera
			ICamera* activeCamera = nullptr;                    // And the active camera

			eastl::unique_ptr<Thirdperson> cameraThird;         // Third person camera
			eastl::unique_ptr<Firstperson> cameraFirst;         // First person camera
		
			bool ranLastFrame = false;                          // Did the camera run last frame?
			bool povIsThird = false;                            // Our current POV state
			bool povWasPressed = false;                         // Change POV was pressed
			bool wasLoading = false;                            // True if we saw the loading screen menu

			bool apiControlled = false;
			bool wasCameraAPIControlled = false;                // Was the camera API controlled last frame?
			bool accControl = false;                            // Set when camera assumes ACC should be running
			bool wasDialogOpen = false;                         // Set if the player was in dialog the last time the camera ran, but not this time

			bool inMenuMode = false;							// True if the player is in menu mode
			int8_t loadScreenDepth = 0;                         // If not 0, we are in a loading screen sequence
			RE::NiFrustum frustum;                              // Our current view frustum
			mmath::NiMatrix44 worldToScaleform;                 // Our current worldToScreen matrix for the hud
#ifdef DEVELOPER
			TrackIR::TrackingSnapshot trackIRData;              // If using TrackIR, the current data from the tracker
#endif
			// The last and current camera state
			GameState::CameraState currentState = GameState::CameraState::Unknown;
			GameState::CameraState lastState = GameState::CameraState::Unknown;

			// And last and current camera action state
			CameraActionState currentActionState = CameraActionState::Unknown;
			CameraActionState lastActionState = CameraActionState::Unknown;

			// Whatever position was last set before any camera update code is executed for the frame
			glm::vec3 gameLastActualPosition = { 0.0f, 0.0f, 0.0f };

			bool wantNewCameraState = false; // If true, we are to switch to a new camera state next update
			uint8_t wantNewState = RE::CameraState::kThirdPerson; // The state to switch to

			// When using cycle preset hotkeys, this will point at our current preset ID.
			// We could store the preset slot in the config structure, but that would cause compat issues with older
			// save files and somewhat prevent renaming preset files outside of the game.
			// Just start at slot 0 every time and keep track of it here.
			int currentPresetIndex = 0; 

			friend class State::BaseThird;
			friend class State::BaseFirst;
			friend class Thirdperson;
			friend class Firstperson;
	};
}