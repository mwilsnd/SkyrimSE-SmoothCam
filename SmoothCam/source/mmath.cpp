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
	__m128 cosValues, sineValues;
	if constexpr (alignof(decltype(rotation)) == 16) {
		__m128 rot = _mm_load_ps(rotation.data.data);
		sineValues = _mm_sincos_ps(&cosValues, rot);
	} else {
		__m128 rot = _mm_loadu_ps(rotation.data.data);
		sineValues = _mm_sincos_ps(&cosValues, rot);
	}

	const auto cZsX = cosValues.m128_f32[2] * sineValues.m128_f32[0];
	const auto sXsZ = sineValues.m128_f32[0] * sineValues.m128_f32[2];
	
	forward = {
		cosValues.m128_f32[1] * cosValues.m128_f32[2],
		-cosValues.m128_f32[1] * sineValues.m128_f32[2],
		sineValues.m128_f32[1]
	};
	right = {
		cZsX * sineValues.m128_f32[1] + cosValues.m128_f32[0] * sineValues.m128_f32[2],
		cosValues.m128_f32[0] * cosValues.m128_f32[2] - sXsZ * sineValues.m128_f32[1],
		-cosValues.m128_f32[1] * sineValues.m128_f32[0]
	};
	up = {
		-cosValues.m128_f32[0] * cosValues.m128_f32[2] * sineValues.m128_f32[1] + sXsZ,
		cZsX + cosValues.m128_f32[0] * sineValues.m128_f32[1] * sineValues.m128_f32[2],
		cosValues.m128_f32[0] * cosValues.m128_f32[1]
	};

	coef = {
		glm::dot(point, forward),
		glm::dot(point, right),
		glm::dot(point, up)
	};
}

glm::vec3 mmath::PointToScreen(const glm::vec3& point) noexcept {
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

glm::mat4 mmath::Perspective(float fov, float aspect, const NiFrustum& frustum) noexcept {
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

glm::mat4 mmath::LookAt(const glm::vec3& pos, const glm::vec3& at, const glm::vec3& up) noexcept {
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


void mmath::Rotation::SetEuler(float pitch, float yaw) noexcept {
	euler.x = pitch;
	euler.y = yaw;
	dirty = true;
}

void mmath::Rotation::SetQuaternion(const glm::quat& q) noexcept {
	quat = q;
	euler.x = glm::pitch(q) * -1.0f;
	euler.y = glm::roll(q) * -1.0f; // The game stores yaw in the Z axis
	dirty = true;
}

void mmath::Rotation::SetQuaternion(const NiQuaternion& q) noexcept {
	SetQuaternion(glm::quat{ q.m_fW, q.m_fX, q.m_fY, q.m_fZ });
}

void mmath::Rotation::CopyFrom(const TESObjectREFR* ref) noexcept {
	SetEuler(ref->rot.x, ref->rot.z);
}

void mmath::Rotation::UpdateQuaternion() noexcept {
	quat = glm::quat(glm::vec3{ -euler.x, 0.0f, -euler.y });
}

glm::quat mmath::Rotation::InverseQuat() const noexcept {
	return glm::quat(glm::vec3{ euler.x, 0.0f, euler.y });
}

NiQuaternion mmath::Rotation::InverseNiQuat() const noexcept {
	const auto q = InverseQuat();
	return { q.w, q.x, q.y, q.z };
}

NiQuaternion mmath::Rotation::ToNiQuat() const noexcept {
	return { quat.w, quat.x, quat.y, quat.z };
}

NiPoint2 mmath::Rotation::ToNiPoint2() const noexcept {
	return { euler.x, euler.y };
}

NiPoint3 mmath::Rotation::ToNiPoint3() const noexcept {
	return { euler.x, 0.0f, euler.y };
}

glm::mat4 mmath::Rotation::ToRotationMatrix() noexcept {
	if (dirty) {
		mat = glm::identity<glm::mat4>();
		mat = glm::rotate(mat, -euler.y, { 0.0f, 0.0f, 1.0f }); // yaw
		mat = glm::rotate(mat, -euler.x, { 1.0f, 0.0f, 0.0f }); // pitch
		dirty = false;
	}
	return mat;
}

NiMatrix33 mmath::Rotation::THT() const noexcept {
	return mmath::ToddHowardTransform(euler.x, euler.y);
}