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

glm::vec3 mmath::PointToScreen(const glm::vec3& point) {
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

	return { screen.x, screen.y, screen.z };
};

glm::mat4 mmath::Perspective(float fov, float aspect, const NiFrustum& frustum) {
	const auto range = frustum.m_fFar / (frustum.m_fNear - frustum.m_fFar);
	const auto height = 1.0f / glm::tan(fov * 0.5f);

	glm::mat4 proj;
	proj[0][0] = height;
	proj[0][1] = 0.0f;
	proj[0][2] = 0.0f;
	proj[0][3] = 0.0f;

	proj[1][0] = 0.0f;
	proj[1][1] = height * aspect;
	proj[1][2] = 0.0f;
	proj[1][3] = 0.0f;

	proj[2][0] = 0.0f;
	proj[2][1] = 0.0f;
	proj[2][2] = range * -1.0f;
	proj[2][3] = 1.0f;

	proj[3][0] = 0.0f;
	proj[3][1] = 0.0f;
	proj[3][2] = range * frustum.m_fNear;
	proj[3][3] = 0.0f;
	
	// exact match, save for 2,0 2,1 - looks like XMMatrixPerspectiveOffCenterLH with a slightly
	// different frustum or something. whatever, close enough.
	return proj; 
}

glm::mat4 mmath::LookAt(const glm::vec3& pos, const glm::vec3& at, const glm::vec3& up) {
	const auto forward = glm::normalize(at - pos);
	const auto side = glm::normalize(glm::cross(up, forward));
	const auto u = glm::cross(forward, side);

	const auto negEyePos = pos * -1.0f;
	const auto sDotEye = glm::dot(side, negEyePos);
	const auto uDotEye = glm::dot(u, negEyePos);
	const auto fDotEye = glm::dot(forward, negEyePos);

	glm::mat4 result;
	result[0][0] = side.x * -1.0f;
	result[0][1] = side.y * -1.0f;
	result[0][2] = side.z * -1.0f;
	result[0][3] = sDotEye *-1.0f;

	result[1][0] = u.x;
	result[1][1] = u.y;
	result[1][2] = u.z;
	result[1][3] = uDotEye;

	result[2][0] = forward.x;
	result[2][1] = forward.y;
	result[2][2] = forward.z;
	result[2][3] = fDotEye;

	result[3][0] = 0.0f;
	result[3][1] = 0.0f;
	result[3][2] = 0.0f;
	result[3][3] = 1.0f;

	return glm::transpose(result);
}