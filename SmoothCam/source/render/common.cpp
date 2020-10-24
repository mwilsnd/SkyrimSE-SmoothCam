#include "render/common.h"

float Render::GetFOV() noexcept {
	// The game doesn't tell us about dyanmic changes to FOV in an easy way (that I can see, but I'm also blind)
	// It does store the FOV indirectly in this global - we can just run the equations it uses in reverse.
	const auto fac = Offsets::Get<float*>(513786);
	const auto x = *fac / 1.30322540;
	const auto y = glm::atan(x);
	const auto fov = y / 0.01745328f / 0.5f;
	return glm::radians(fov);
}

glm::mat4 Render::GetProjectionMatrix(const NiFrustum& frustum) noexcept {
	const auto& viewSize = Render::GetContext().windowSize;
	const float aspect = viewSize.x / viewSize.y;

	auto f = frustum;
	f.m_fNear *= RenderScale;
	f.m_fFar *= RenderScale;

	return mmath::Perspective(GetFOV(), aspect, f);
}

glm::mat4 Render::BuildViewMatrix(const glm::vec3& position, const glm::vec2& rotation) noexcept {
	const auto pos = Render::ToRenderScale(position);
	const auto dir = mmath::GetViewVector(
		{ 0.0, 1.0, 0.0 },
		rotation.x,
		rotation.y
	);
	return mmath::LookAt(pos, pos + dir, { 0.0f, 0.0f, 1.0f });
}