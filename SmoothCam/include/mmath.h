#pragma once

namespace Config {
	enum class ScalarMethods;
}

namespace mmath {
	constexpr const float half_pi = 1.57079632679485f;

	typedef struct {
		float data[4][4];
	} NiMatrix44;

	typedef struct aabb {
		glm::vec3 mins;
		glm::vec3 maxs;

		aabb aabb::operator+ (const glm::vec3& rhs) {
			return {
				mins + rhs,
				maxs + rhs
			};
		}

		aabb aabb::operator+ (const NiPoint3& rhs) {
			return {
				mins + glm::vec3{ rhs.x, rhs.y, rhs.z },
				maxs + glm::vec3{ rhs.x, rhs.y, rhs.z }
			};
		}
	} AABB;

	bool IsInf(const float& f) noexcept;
	bool IsInf(const glm::vec3& v) noexcept;
	bool IsInf(const glm::vec4& v) noexcept;

	bool IsNan(const float& f) noexcept;
	bool IsNan(const glm::vec3& v) noexcept;
	bool IsNan(const glm::vec4& v) noexcept;

	bool IsValid(const float& f) noexcept;
	bool IsValid(const glm::vec3& v) noexcept;
	bool IsValid(const glm::vec4& v) noexcept;

	// Return the forward view vector
	glm::vec3 GetViewVector(const glm::vec3& forwardRefer, float pitch, float yaw) noexcept;
	// Extracts pitch and yaw from a rotation matrix
	glm::vec3 NiMatrixToEuler(const NiMatrix33& m) noexcept;
	// Creates a rotation matrix for NiCameras to compute a proper world to screen matrix for scaleform
	NiMatrix33 ToddHowardTransform(const float pitch, const float yaw) noexcept;
	// Decompose a position to 3 basis vectors and the coefficients, given an euler rotation
	void DecomposeToBasis(const glm::vec3& point, const glm::vec3& rotation,
		glm::vec3& forward, glm::vec3& right, glm::vec3& up, glm::vec3& coef) noexcept;

	// Construct an AABB for an actor
	AABB GetReferAABB(TESObjectREFR* ref);
	AABB RotateAABB(const AABB& axisAligned, const NiMatrix33& mat) noexcept;
	AABB GetActorAABB(Actor* actor);

	// Ray-AABB intersect
	bool IntersectRayAABB(const glm::vec3& start, const glm::vec3& dir, const AABB& aabb,
		glm::vec3& hitPos) noexcept;

	glm::vec2 PointToScreen(const glm::vec3& point);

	template<typename T, typename S>
	T Interpolate(const T from, const T to, const S scalar) noexcept {
		if (scalar > 1.0) return to;
		if (scalar < 0.0) return from;
		return from + (to - from) * scalar;
	};

	template<typename T>
	T Remap(T value, T inMin, T inMax, T outMin, T outMax) noexcept {
		return outMin + (((value - inMin) / (inMax - inMin)) * (outMax - outMin));
	};

	template<typename T>
	T RunScalarFunction(Config::ScalarMethods scalarMethod, T interpValue) noexcept {
		switch (scalarMethod) {
			case Config::ScalarMethods::LINEAR:
				return glm::linearInterpolation(interpValue);
			case Config::ScalarMethods::QUAD_IN:
				return glm::quadraticEaseIn(interpValue);
			case Config::ScalarMethods::QUAD_OUT:
				return glm::quadraticEaseOut(interpValue);
			case Config::ScalarMethods::QUAD_INOUT:
				return glm::quadraticEaseInOut(interpValue);
			case Config::ScalarMethods::CUBIC_IN:
				return glm::cubicEaseIn(interpValue);
			case Config::ScalarMethods::CUBIC_OUT:
				return glm::cubicEaseOut(interpValue);
			case Config::ScalarMethods::CUBIC_INOUT:
				return glm::cubicEaseInOut(interpValue);
			case Config::ScalarMethods::QUART_IN:
				return glm::quarticEaseIn(interpValue);
			case Config::ScalarMethods::QUART_OUT:
				return glm::quarticEaseOut(interpValue);
			case Config::ScalarMethods::QUART_INOUT:
				return glm::quarticEaseInOut(interpValue);
			case Config::ScalarMethods::QUINT_IN:
				return glm::quinticEaseIn(interpValue);
			case Config::ScalarMethods::QUINT_OUT:
				return glm::quinticEaseOut(interpValue);
			case Config::ScalarMethods::QUINT_INOUT:
				return glm::quinticEaseInOut(interpValue);
			case Config::ScalarMethods::SINE_IN:
				return glm::sineEaseIn(interpValue);
			case Config::ScalarMethods::SINE_OUT:
				return glm::sineEaseOut(interpValue);
			case Config::ScalarMethods::SINE_INOUT:
				return glm::sineEaseInOut(interpValue);
			case Config::ScalarMethods::CIRC_IN:
				return glm::circularEaseIn(interpValue);
			case Config::ScalarMethods::CIRC_OUT:
				return glm::circularEaseOut(interpValue);
			case Config::ScalarMethods::CIRC_INOUT:
				return glm::circularEaseInOut(interpValue);
			case Config::ScalarMethods::EXP_IN:
				return glm::exponentialEaseIn(interpValue);
			case Config::ScalarMethods::EXP_OUT:
				return glm::exponentialEaseOut(interpValue);
			case Config::ScalarMethods::EXP_INOUT:
				return glm::exponentialEaseInOut(interpValue);
			default:
				return glm::linearInterpolation(interpValue);
		}
	};
}