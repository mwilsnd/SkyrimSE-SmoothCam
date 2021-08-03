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
		SittingTransition,		// Player is sitting down or standing up		!-> Unused
		Sitting,				// Player is sitting (In a chair, on a horse)
		FirstPersonSitting,		// A special mode for improved camera with custom transition rules
		Sleeping,				// Player is in a bed
		Aiming,					// Player is aiming with a bow
		Mounting,				// Player is mounting a horse					!-> Unused
		DisMounting,			// Player is dismounting a horse
		FirstPersonHorseback,	// A special mode for improved camera with custom transition rules
		FirstPersonDragon,		// A special mode for improved camera with custom transition rules
		VampireLord,			// Player is a vampire lord
		Werewolf,				// Player is a werewolf
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
			virtual void OnBegin(PlayerCharacter* player, CorrectedPlayerCamera* camera, ICamera* lastState) noexcept = 0;
			// Called when we are done using this camera
			virtual void OnEnd(PlayerCharacter* player, CorrectedPlayerCamera* camera, ICamera* newState) noexcept = 0;

			// Runs before the internal game camera logic
			// Return true when changing the camera state
			virtual bool OnPreGameUpdate(PlayerCharacter* player, CorrectedPlayerCamera* camera,
				BSTSmartPointer<TESCameraState>& nextState) = 0;
			// Selects the correct update method and positions the camera
			virtual void OnUpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera,
				BSTSmartPointer<TESCameraState>& nextState) = 0;
			// Render crosshair objects
			virtual void Render(Render::D3DContext& ctx) noexcept = 0;

			// Called when the player toggles the POV
			virtual void OnTogglePOV(const ButtonEvent* ev) noexcept = 0;
			// Called when any other key is pressed
			virtual bool OnKeyPress(const ButtonEvent* ev) noexcept = 0;
			// Called when a menu of interest is opening or closing
			virtual bool OnMenuOpenClose(MenuID id, const MenuOpenCloseEvent* const ev) noexcept = 0;

			// Triggers when the camera action state changes
			virtual void OnCameraActionStateTransition(const PlayerCharacter* player, const CameraActionState newState,
				const CameraActionState oldState) noexcept = 0;
			// Triggers when the camera state changes
			virtual void OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
				const GameState::CameraState newState, const GameState::CameraState oldState) noexcept = 0;

		public:
			const CameraID m_id = CameraID::None;

		protected:
			Camera* m_camera = nullptr;

	};

	// Low level camera, holding both the thirdperson and firstperson cameras
	class Camera {
		public:
			Camera();
			~Camera();
			Camera(const Camera&) = delete;
			Camera(Camera&&) noexcept = delete;
			Camera& operator=(const Camera&) = delete;
			Camera& operator=(Camera&&) noexcept = delete;
			
		public:
			// Runs before the internal game camera logic
			// Return true when changing the camera state
			bool PreGameUpdate(PlayerCharacter* player, CorrectedPlayerCamera* camera,
				BSTSmartPointer<TESCameraState>& nextState);
			// Selects the correct update method and positions the camera
			void UpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera,
				BSTSmartPointer<TESCameraState>& nextState);
			// Render crosshair objects
			void Render(Render::D3DContext& ctx);

			// Called when the player toggles the POV
			void OnTogglePOV(const ButtonEvent* ev) noexcept;
			// Called when any other key is pressed
			void OnKeyPress(const ButtonEvent* ev) noexcept;
			// Called when a menu of interest is opening or closing
			void OnMenuOpenClose(MenuID id, const MenuOpenCloseEvent* const ev) noexcept;

			// Returns the current camera state for use in selecting an update method
			const GameState::CameraState GetCurrentCameraState()  const noexcept;
			// Returns the current camera action state for use in the selected update method
			const CameraActionState GetCurrentCameraActionState() const noexcept;

			// Set the camera world position
			void SetPosition(const glm::vec3& pos, const CorrectedPlayerCamera* camera) noexcept;

			// Return the current frustum
			const NiFrustum& GetFrustum() const noexcept;
			// Get the object the camera is currently focused on
			NiPointer<Actor> GetCurrentCameraTarget(const CorrectedPlayerCamera* camera) noexcept;
			// Returns true if a loading screen is active
			bool InLoadingScreen() const noexcept;
			// Option for the MCM - Allow forcing the camera to a new state
			// Adding this after observing a bug - Camera got stuck in kCameraState_Furniture
			void SetShouldForceCameraState(bool force, uint8_t newCameraState) noexcept;

			// Get the thirdperson camera
			Thirdperson* GetThirdpersonCamera() noexcept;
			// Get the firstperson camera
			Firstperson* GetFirstpersonCamera() noexcept;

		private:
			// Update skyrim's screen projection matrix
			void UpdateInternalWorldToScreenMatrix(const mmath::Rotation& rot, const CorrectedPlayerCamera* camera) noexcept;
			// Updates our POV state to the true value the game expects for each state
			const bool UpdateCameraPOVState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept;
			// Returns the current camera state for use in selecting an update method
			const GameState::CameraState UpdateCurrentCameraState(const PlayerCharacter* player,
				const CorrectedPlayerCamera* camera) noexcept;
			// Returns the current camera action state for use in the selected update method
			const CameraActionState UpdateCurrentCameraActionState(const PlayerCharacter* player,
				const CorrectedPlayerCamera* camera) noexcept;

			// Triggers when the camera action state changes
			void OnCameraActionStateTransition(const PlayerCharacter* player, const CameraActionState newState,
				const CameraActionState oldState) const noexcept;
			// Triggers when the camera state changes
			void OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
				const GameState::CameraState newState, const GameState::CameraState oldState);

			NiPointer<NiCamera> GetNiCamera(CorrectedPlayerCamera* camera) const noexcept;

		private:
			Config::UserConfig* config = nullptr;       // User config
			Actor* currentFocusObject = nullptr;        // The ref object the camera is focused on
			NiPointer<NiCamera> cameraNi = nullptr;     // Active NiCamera
			ICamera* activeCamera = nullptr;            // And the active camera

			eastl::unique_ptr<Thirdperson> cameraThird; // Third person camera
			eastl::unique_ptr<Firstperson> cameraFirst; // First person camera
		
			bool ranLastFrame = false;					// Did the camera run last frame?
			bool povIsThird = false;                    // Our current POV state
			bool povWasPressed = false;					// Change POV was pressed
			bool wasLoading = false;					// True if we saw the loading screen menu
			int8_t loadScreenDepth = 0;                 // If not 0, we are in a loading screen sequence
			NiFrustum frustum;                          // Our current view frustum
			mmath::NiMatrix44 worldToScaleform;         // Our current worldToScreen matrix for the hud
#ifdef DEVELOPER
			TrackIR::TrackingSnapshot trackIRData;      // If using TrackIR, the current data from the tracker
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
			uint8_t wantNewState = CorrectedPlayerCamera::kCameraState_ThirdPerson2; // The state to switch to

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