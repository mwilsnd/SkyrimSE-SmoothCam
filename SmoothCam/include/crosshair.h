#pragma once
#include "render/line_drawer.h"
#include "render/mesh_drawer.h"
#include "render/model.h"
#include "render/shader.h"
#include "render/vertex_buffer.h"

#include "crosshair/base.h"

namespace Crosshair {
	class Manager {
		public:
			Manager();
			~Manager();

			// Get a normal vector pointing in the direction of the crosshair
			glm::vec3 GetCrosshairTargetNormal(const glm::vec2& aimRotation, float pitchMod = 0.0f);

			// Updates the screen position of the crosshair for correct aiming
			void UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* camera,
				const glm::vec2& aimRotation, mmath::NiMatrix44& worldToScaleform);

			// Set the 3D crosshair position
			void SetCrosshairPosition(const glm::dvec2& pos);

			// Center the position of the crosshair
			void CenterCrosshair();

			// Set the size of the 3D crosshair
			void SetCrosshairSize(const glm::dvec2& size);

			// Set the crosshair size back to default
			void SetDefaultSize();

			// Show or hide the crosshair
			void SetCrosshairEnabled(bool enabled);

			// Select the type of 3D crosshair to render with
			void Set3DCrosshairType(Config::CrosshairType type);

			// Render crosshair objects
			void Render(Render::D3DContext& ctx, const glm::vec3& cameraPosition, const glm::vec2& cameraRotation,
				const NiFrustum& frustum) noexcept;

		private:
			// Read initial values for the crosshair during startup
			void ReadInitialCrosshairInfo();

			// Simulate a time slice of projectile physics
			void TickProjectilePath(glm::vec3& position, glm::vec3& vel, float gravity, float dt) noexcept;

			// Compute the initial impulse vector for the given projectile
			glm::vec3 ComputeProjectileVelocityVector(PlayerCharacter* player, const CorrectedPlayerCamera* camera,
				const TESAmmo* ammo, float gravity, const glm::vec2& aimRotation) noexcept;

			// Cast a curved ray for the currently equipped projectile
			bool ProjectilePredictionCurve(PlayerCharacter* player, const CorrectedPlayerCamera* camera,
				const glm::vec2& aimRotation, const glm::vec3& startPos, glm::vec3& hitPos) noexcept;

		private:
			BSFixedString weapon = "WEAPON";

			// Crosshair metrics read at game start before we mess with them
			struct {
				bool captured = false;
				double xOff = 0.0;
				double yOff = 0.0;
				double xScale = 0.0;
				double yScale = 0.0;
				double xCenter = 0.0;
				double yCenter = 0.0;
			} baseCrosshairData;

			// The last state we left the crosshair in
			struct {
				glm::dvec2 position = { 0.0, 0.0 };
				glm::dvec2 scale = { 1.0, 1.0 };
				bool enabled = true;
			} currentCrosshairData;

			// Renderable objects
			struct {
				// Data which should really only change once each frame
				struct VSMatricesCBuffer {
					glm::mat4 matProjView;
					float curTime;
					float pad[3];
				};
				static_assert(sizeof(VSMatricesCBuffer) % 16 == 0);

				// Data which is likely to change each draw call
				struct VSPerObjectCBuffer {
					glm::mat4 model;
				};
				static_assert(sizeof(VSMatricesCBuffer) % 16 == 0);

				VSMatricesCBuffer cbufPerFrameStaging;
				VSPerObjectCBuffer cbufPerObjectStaging;
				std::shared_ptr<Render::CBuffer> cbufPerFrame;
				std::shared_ptr<Render::CBuffer> cbufPerObject;

				std::unique_ptr<Crosshair::Base> curCrosshair;
				Config::CrosshairType crosshairType = Config::CrosshairType::None;
				bool drawCrosshair = false;

				// Resouces for drawing the arrow prediction arc
				Render::LineList arrowTailSegments;
				std::unique_ptr<Render::LineDrawer> tailDrawer;

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