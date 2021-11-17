#pragma once

namespace Camera {
	class SmoothCamera;
}

namespace Crosshair {
	class Base {
		public:
			explicit Base() = default;
			virtual ~Base() noexcept {};
			Base(const Base&) = delete;
			Base(Base&&) noexcept = delete;
			Base& operator=(const Base&) = delete;
			Base& operator=(Base&&) noexcept = delete;

			// Create any needed 3D assets for rendering
			virtual void Create3D(Render::D3DContext& ctx, eastl::shared_ptr<Render::CBuffer>& perObjectBuf) noexcept = 0;
			
			// Render the crosshair
			virtual void Render(Render::D3DContext& ctx, bool allowDepthTesting) noexcept = 0;
			
			// Set the world position
			virtual void SetPosition(const glm::vec3& pos) noexcept;
			// Set the rotation
			virtual void SetRotation(const glm::vec3& rot) noexcept;
			// Set the scale
			virtual void SetScale(const glm::vec3& s) noexcept;
			// Update the transform matrix
			virtual void UpdateTransform() noexcept;

			// Get the world position
			glm::vec3 GetPosition() const noexcept;
			// Get the rotation
			glm::vec3 GetRotation() const noexcept;
			// Get the scale
			glm::vec3 GetScale() const noexcept;
			// Get the transform matrix
			glm::mat4 GetTransform() const noexcept;

		protected:
			eastl::shared_ptr<Render::CBuffer> perObjectBuffer;
			glm::mat4 transform = glm::identity<glm::mat4>();
			glm::vec3 position = { 0.0f, 0.0f, 0.0f };
			glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
			glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
			bool dirty = true;
	};
}