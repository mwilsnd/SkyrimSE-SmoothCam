#pragma once

namespace Render {
	// If we don't downscale positions we will run in to precision issues on the GPU when
	// near the edges of the map. World positions and camera must all be scaled by the same amount!
	// I'm a bit surprised a game of this scale wouldn't use some kind of origin re-basing.
	constexpr auto RenderScale = 0.0142875f;

	// Convert a world position to our render scale
	__forceinline glm::vec3 ToRenderScale(const glm::vec3& position) {
		return position * RenderScale;
	}

	// And convert back
	__forceinline glm::vec3 FromRenderScale(const glm::vec3& position) {
		return position / RenderScale;
	}

	// Get the current FOV
	float GetFOV() noexcept;
	// Get the current projection matrix for 3D rendering
	glm::mat4 GetProjectionMatrix(const NiFrustum& frustum) noexcept;
	// Build a view matrix for 3D rendering
	glm::mat4 BuildViewMatrix(const glm::vec3& position, const glm::vec2& rotation) noexcept;
}