#pragma once
#include "render/d3d_context.h"
#include "render/cbuffer.h"
#include "skyrimSE/PlayerCamera.h"

namespace Camera {
	class SmoothCamera;
}

namespace Crosshair {
	class Base {
		public:
			Base() = default;
			virtual ~Base() {};
			Base(const Base&) = delete;
			Base(Base&&) noexcept = delete;
			Base& operator=(const Base&) = delete;
			Base& operator=(Base&&) noexcept = delete;

			// Create any needed 3D assets for rendering
			virtual void Create3D(Render::D3DContext& ctx, std::shared_ptr<Render::CBuffer>& perObjectBuf) = 0;
			
			// Render the crosshair
			virtual void Render(Render::D3DContext& ctx, float curTime, float deltaTime, bool allowDepthTesting) = 0;
			
			// Set the world position
			virtual void SetPosition(const glm::vec3& pos);
			// Set the rotation
			virtual void SetRotation(const glm::vec3& rot);
			// Set the scale
			virtual void SetScale(const glm::vec3& s);
			// Update the transform matrix
			virtual void UpdateTransform();

			// Get the world position
			glm::vec3 GetPosition() const noexcept;
			// Get the rotation
			glm::vec3 GetRotation() const noexcept;
			// Get the scale
			glm::vec3 GetScale() const noexcept;
			// Get the transform matrix
			glm::mat4 GetTransform() const noexcept;

		protected:
			std::shared_ptr<Render::CBuffer> perObjectBuffer;
			glm::mat4 transform = glm::identity<glm::mat4>();
			glm::vec3 position = { 0.0f, 0.0f, 0.0f };
			glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
			glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
			bool dirty = true;
	};
}