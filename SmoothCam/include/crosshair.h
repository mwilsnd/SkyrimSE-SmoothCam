#pragma once
#include "render/line_drawer.h"
#include "render/mesh_drawer.h"
#include "render/model.h"
#include "crosshair/base.h"

namespace Crosshair {
	class Manager;

	class Manager {
		public:
			struct CurrentCrosshairData {
				glm::dvec2 ofs = { 0.0, 0.0 };
				glm::dvec2 position = { 0.0, 0.0 };
				glm::dvec2 scale = { 1.0, 1.0 };

				glm::dvec2 stealthMeterOfs = { 0.0, 0.0 };
				glm::dvec2 stealthMeterPosition = { 0.0, 0.0 };

				bool enabled = true;
				bool invalidated = false;
				bool alertMode = false;
				bool stealthMeterMutated = false;
			};

		public:
			Manager() noexcept;
			~Manager();

			// Get a normal vector pointing in the direction of the crosshair
			glm::vec3 GetCrosshairTargetNormal(const glm::vec2& aimRotation, float pitchMod = 0.0f) const noexcept;

			// Updates the screen position of the crosshair for correct aiming
			void UpdateCrosshairPosition(const RE::Actor* player, const glm::vec2& aimRotation,
				const glm::vec2& cameraRotation, mmath::NiMatrix44& worldToScaleform) noexcept;

			// Set the 3D crosshair position
			void SetCrosshairPosition(const glm::dvec2& pos) noexcept;

			// Set the stealth meter position (eye crosshair)
			void SetStealthMeterPosition(const glm::vec2& pos) noexcept;

			// Center the position of the crosshair
			void CenterCrosshair() noexcept;

			// Center the position of the stealth meter
			void CenterStealthMeter() noexcept;

			// Set the size of the 3D crosshair
			void SetCrosshairSize(const glm::dvec2& size) noexcept;

			// Set the crosshair size back to default
			void SetDefaultSize() noexcept;

			// Show or hide the crosshair
			void SetCrosshairEnabled(bool enabled) noexcept;

			// Select the type of 3D crosshair to render with
			void Set3DCrosshairType(Config::CrosshairType type) noexcept;

			// When loading cells, the crosshair appears to re-enable, which means our cache ends up
			// in a conflicting state - This method will invalidate the cache, such that the new call
			// to SetCrosshairEnabled actually invokes the scaleform function.
			void InvalidateEnablementCache() noexcept;

			// Update state each frame
			void Update(RE::Actor* player) noexcept;

			// Render crosshair objects
			void Render(Render::D3DContext& ctx, const glm::vec3& cameraPosition, const glm::vec2& cameraRotation,
				const RE::NiFrustum& frustum) noexcept;

			// Reset mutations, hard reset = true to clear crosshair related user settings
			void Reset(bool hard = false) noexcept;
			void ResetCrosshair() noexcept;
			void ResetStealthMeter(bool hard = false) noexcept;

			// Get the current state of the cached crosshair data
			CurrentCrosshairData& GetCurrentCrosshairData() noexcept;

			// Set the crosshair alert mode (Used by conjuration magic)
			void SetAlertMode(bool alert) noexcept;
			// Called by the game, we need to re-apply mutations here
			void ValidateCrosshair() noexcept;

		private:
			// Returns true if the manager has captured the base crosshair data correctly
			// If returning false, the manager is not allowed to modify the crosshair
			bool IsCrosshairDataValid() const noexcept;

			// Read initial values for the crosshair during startup
			void ReadInitialCrosshairInfo() noexcept;

			// Simulate a time slice of projectile physics
			void TickProjectilePath(glm::vec3& position, glm::vec3& vel, const glm::vec3& gravity,
				float gravityScale, float dt) noexcept;

			// Compute the initial impulse vector for the given projectile
			glm::vec3 ComputeProjectileVelocityVector(const RE::Actor* player,
				const RE::TESAmmo* ammo, const glm::vec2& aimRotation) noexcept;

			// Cast a curved ray for the currently equipped projectile
			bool ProjectilePredictionCurve(const RE::Actor* player, const glm::vec2& aimRotation,
				const glm::vec3& startPos, glm::vec3& hitPos, bool& hitCharacter) noexcept;

			// Try to find the arrow node on the player. It can either be attached to the weapon node or a magic node.
			RE::NiAVObject* FindArrowNode(const RE::Actor* player) const noexcept;

			// With True Directional Movement, player yaw is unlocked from camera yaw - meaning our crosshair, while still
			// "correct", looks very strange when we are standing still. This method will correct any rotation delta from
			// player to camera.
			glm::vec3 TranslateFirePostion(const RE::Actor* player, const glm::vec2& cameraRotation,
				const glm::vec3& firePosition) const noexcept;

		private:
			struct {
				mutable RE::BSFixedString weapon = "WEAPON";
				mutable RE::BSFixedString arrowName = "Arrow:0";
				mutable RE::BSFixedString magic = "NPC Head MagicNode [Hmag]";
				mutable RE::BSFixedString lmag = "NPC L MagicNode [LMag]";
				mutable RE::BSFixedString rmag = "NPC R MagicNode [RMag]";
			} Strings;

			// Crosshair metrics read at game start before we mess with them
			struct {
				bool captured = false;
				double xOff = 0.0;
				double yOff = 0.0;
				double xScale = 0.0;
				double yScale = 0.0;
				double xCenter = 0.0;
				double yCenter = 0.0;

				double stealthXOff = 0.0;
				double stealthYOff = 0.0;
			} baseCrosshairData;

			// The last state we left the crosshair in
			CurrentCrosshairData currentCrosshairData;

			// Renderable objects
			struct {
				// Data which should really only change once each frame
				struct VSMatricesCBuffer {
					glm::mat4 matProjView = glm::identity<glm::mat4>();
					glm::vec4 tint = { 1.0f, 1.0f, 1.0f, 1.0f };
					float curTime = 0.0f;
					float pad[3] = { 0.0f, 0.0f, 0.0f };
				};
				static_assert(sizeof(VSMatricesCBuffer) % 16 == 0);

				// Data which is likely to change each draw call
				struct VSPerObjectCBuffer {
					glm::mat4 model = glm::identity<glm::mat4>();
				};
				static_assert(sizeof(VSPerObjectCBuffer) % 16 == 0);

				VSMatricesCBuffer cbufPerFrameStaging = {};
				VSPerObjectCBuffer cbufPerObjectStaging = {};
				eastl::shared_ptr<Render::CBuffer> cbufPerFrame;
				eastl::shared_ptr<Render::CBuffer> cbufPerObject;

				eastl::unique_ptr<Crosshair::Base> curCrosshair;
				Config::CrosshairType crosshairType = Config::CrosshairType::None;
				bool drawCrosshair = false;
				bool hitCharacter = false;

				// Resouces for drawing the arrow prediction arc
				Render::LineList arrowTailSegments;
				eastl::unique_ptr<Render::LineDrawer> tailDrawer;

				// D3D expects resources to be released in a certain order
				void release() {
					tailDrawer.reset();
					curCrosshair.reset();
					cbufPerObject.reset();
					cbufPerFrame.reset();
				}
			} renderables;
	};
}