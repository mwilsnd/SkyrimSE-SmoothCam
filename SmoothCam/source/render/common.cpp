#include "render/common.h"

extern Offsets* g_Offsets;

float Render::GetFOV() noexcept {
#ifdef SKYRIM_SUPPORT_AE
	static const auto fov = REL::Relocation<float*>(g_Offsets->FOV);
	const auto deg = *fov + *REL::Relocation<float*>(g_Offsets->FOVOffset);
	return glm::radians(deg);
#else
	// The game doesn't tell us about dyanmic changes to FOV in an easy way (that I can see, but I'm also blind)
	// It does store the FOV indirectly in this global - we can just run the equations it uses in reverse.
	static const auto fac = REL::Relocation<float*>(g_Offsets->FOV);
	const auto base = *fac;
	const auto x = base / 1.30322540f;
	const auto y = glm::atan(x);
	const auto fov = y / 0.01745328f / 0.5f;
	return glm::radians(fov);
#endif
}

glm::mat4 Render::GetProjectionMatrix(const RE::NiFrustum& frustum) noexcept {
	const auto& viewSize = Render::GetContext().windowSize;
	const float aspect = viewSize.x / viewSize.y;

	auto f = frustum;
	f.fNear *= RenderScale;
	f.fFar *= RenderScale;

	return mmath::Perspective(GetFOV(), aspect, f);
}

glm::mat4 Render::BuildViewMatrix(const glm::vec3& position, const glm::vec2& rotation) noexcept {
	const auto pos = Render::ToRenderScale(position);
	constexpr auto limit = mmath::half_pi * 0.99f;
	const auto dir = mmath::GetViewVector(
		{ 0.0, 1.0, 0.0 },
		glm::clamp(rotation.x, -limit, limit),
		rotation.y
	);
	return mmath::LookAt(pos, pos + dir, { 0.0f, 0.0f, 1.0f });
}