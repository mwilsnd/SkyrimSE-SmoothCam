#include <immintrin.h>

namespace {
	typedef NiPoint3(__thiscall TESObjectREFR::* GetBoundMin)();
	typedef NiPoint3(__thiscall TESObjectREFR::* GetBoundMax)();
}

bool mmath::IsInf(const float& f) noexcept {
	return glm::isinf(f);
}
bool mmath::IsInf(const glm::vec3& v) noexcept {
	return glm::isinf(v.x) || glm::isinf(v.y) || glm::isinf(v.z);
}
bool mmath::IsInf(const glm::vec4& v) noexcept {
	return glm::isinf(v.x) || glm::isinf(v.y) || glm::isinf(v.z) || glm::isinf(v.w);
}

bool mmath::IsNan(const float& f) noexcept {
	return glm::isnan(f);
}
bool mmath::IsNan(const glm::vec3& v) noexcept {
	return glm::isnan(v.x) || glm::isnan(v.y) || glm::isnan(v.z);
}
bool mmath::IsNan(const glm::vec4& v) noexcept {
	return glm::isnan(v.x) || glm::isnan(v.y) || glm::isnan(v.z) || glm::isnan(v.w);
}

bool mmath::IsValid(const float& f) noexcept {
	return !mmath::IsInf(f) && !mmath::IsNan(f);
}
bool mmath::IsValid(const glm::vec3& v) noexcept {
	return !mmath::IsInf(v) && !mmath::IsNan(v);
}
bool mmath::IsValid(const glm::vec4& v) noexcept {
	return !mmath::IsInf(v) && !mmath::IsNan(v);
}

// Return the forward view vector
glm::vec3 mmath::GetViewVector(const glm::vec3& forwardRefer, float pitch, float yaw) noexcept {
	auto aproxNormal = glm::vec4(forwardRefer.x, forwardRefer.y, forwardRefer.z, 1.0);

	auto m = glm::identity<glm::mat4>();
	m = glm::rotate(m, -pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	aproxNormal = m * aproxNormal;
	
	m = glm::identity<glm::mat4>();
	m = glm::rotate(m, -yaw, glm::vec3(0.0f, 0.0f, 1.0f));
	aproxNormal = m * aproxNormal;

	return static_cast<glm::vec3>(aproxNormal);
}

glm::vec3 mmath::NiMatrixToEuler(const NiMatrix33& m) noexcept {
	const float pitch = glm::asin(glm::clamp(-m.data[2][1], -1.0f, 1.0f));
	float yaw = 0.0f;
	if (m.data[0][0] <= 0.0000001f || m.data[2][2] <= 0.0000001f) {
		const auto ab = glm::atan(m.data[0][2], m.data[1][2]);
		yaw = ab - half_pi;
	} else {
		yaw = glm::atan(m.data[0][0], m.data[1][0]);
	}

	return {
		pitch,
		yaw,
		0.0f
	};
}

NiMatrix33 mmath::ToddHowardTransform(const float pitch, const float yaw) noexcept {
	// Create a matrix to flip coords from D3D NDC space to scaleform
	// ¯\_(ツ)_/¯
	auto m = glm::identity<glm::mat4>();
	m = glm::rotate(m, -half_pi, { 1.0f, 0.0f, 0.0f });
	m = glm::rotate(m, -pitch, { 0.0f, 1.0f, 0.0f });
	m = glm::rotate(m, yaw - half_pi, { 0.0f, 0.0f, 1.0f });

	NiMatrix33 mat;
	mat.data[0][0] = m[0][0] * -1.0f;
	mat.data[0][1] = m[0][1] * -1.0f;
	mat.data[0][2] = m[0][2];
	mat.data[1][0] = m[1][0] * -1.0f;
	mat.data[1][1] = m[1][1] * -1.0f;
	mat.data[1][2] = m[1][2];
	mat.data[2][0] = m[2][0] * -1.0f;
	mat.data[2][1] = m[2][1] * -1.0f;
	mat.data[2][2] = m[2][2];

	return mat;
}

// Decompose a position to 3 basis vectors and the coefficients, given an euler rotation
void mmath::DecomposeToBasis(const glm::vec3& point, const glm::vec3& rotation,
	glm::vec3& forward, glm::vec3& right, glm::vec3& up, glm::vec3& coef) noexcept
{
	// @Note: This assumes an XYZ rotation order
	const auto sx = glm::sin(rotation.x);
	const auto sy = glm::sin(rotation.y);
	const auto sz = glm::sin(rotation.z);
	const auto cx = glm::cos(rotation.x);
	const auto cy = glm::cos(rotation.y);
	const auto cz = glm::cos(rotation.z);

	forward = {
		cy * cz,
		-cy * sz,
		sy
	};
	right = {
		cz * sx * sy + cx * sz,
		cx * cz - sx * sy * sz,
		-cy * sx
	};
	up = {
		-cx * cz * sy + sx * sz,
		cz * sx + cx * sy * sz,
		cx * cy
	};
	coef = {
		glm::dot(point, forward),
		glm::dot(point, right),
		glm::dot(point, up)
	};
}

glm::vec2 mmath::PointToScreen(const glm::vec3& point) {
	auto port = NiRect<float>();
	port.m_left = -1.0f;
	port.m_right = 1.0f;
	port.m_top = 1.0f;
	port.m_bottom = -1.0f;

	glm::vec3 screen = {};
	auto niPt = NiPoint3(point.x, point.y, point.z);
	(*WorldPtToScreenPt3_Internal)(
		g_worldToCamMatrix,
		&port, &niPt,
		&screen.x, &screen.y, &screen.z, 9.99999975e-06
	);

	if (screen.z < -1.0f)
		return { -100.0f, -100.0f };

	return { screen.x, screen.y };
}