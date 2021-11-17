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

		private:
			// Enables the thirdperson skeleton and hides the face gen head node
			void ToggleThirdpersonSkeleton(bool show) noexcept;
			// Hide the player's head
			void HidePlayerHead(bool hide) noexcept;

			// Discover nodes which we want to flip depth clipping for during render
			void UpdateClipableWeaponNodes(RE::PlayerCharacter* player) noexcept;
			// Discover nodes which we only want to render in the shadow cascades
			void UpdateShadowOnlyNodes(RE::PlayerCharacter* player) noexcept;

		private:
			struct {
				mutable RE::BSFixedString headPositionTarget = "NPCEyeBone";
				mutable RE::BSFixedString faceNode = "BSFaceGenNiNodeSkinned";
				mutable RE::BSFixedString WEAPON = "WEAPON";
				mutable RE::BSFixedString SHIELD = "SHIELD";
			} Strings;

			Config::UserConfig* config = nullptr;

			// For our depth clip hack with weapons, we store a cache
			eastl::map<ID3D11RasterizerState*, winrt::com_ptr<ID3D11RasterizerState>> rasterDepthClipStates;
			// We need to know which nodes to flip depth clipping on during render
			eastl::vector<RE::NiAVObject*> wantClipNodes;
			// And since we hooked the renderer, might as well control what we only want in the shadow maps!
			eastl::vector<RE::NiAVObject*> shadowsOnly;
	};
}