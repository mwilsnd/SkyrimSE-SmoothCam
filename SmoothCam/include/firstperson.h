#pragma once
#include "camera.h"
#include "camera_states/base_first.h"

namespace Camera {
	// All firstperson only logic
	class Firstperson : public ICamera {
		public:
			Firstperson(Camera* baseCamera);
			Firstperson(const Firstperson&) = delete;
			Firstperson(Firstperson&&) noexcept = delete;
			Firstperson& operator=(const Firstperson&) = delete;
			Firstperson& operator=(Firstperson&&) noexcept = delete;

		public:
			virtual ~Firstperson();

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

		private:
			// Enables the thirdperson skeleton and hides the face gen head node
			void ToggleThirdpersonSkeleton(bool show) noexcept;
			// Hide the player's head
			void HidePlayerHead(bool hide) noexcept;

		private:
			struct {
				mutable BSFixedString headPositionTarget = "NPCEyeBone";
				mutable BSFixedString faceNode = "BSFaceGenNiNodeSkinned";
			} Strings;

			Config::UserConfig* config = nullptr;
	};
}