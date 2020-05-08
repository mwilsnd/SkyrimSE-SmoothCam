#pragma once
#include "pch.h"

namespace Config {
	enum class ScalarMethods;
}

namespace mmath {
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
	glm::vec3 GetViewVector(const glm::vec3& forwardRefer, float pitch, float yaw);

	// Decompose a position to 3 basis vectors and the coefficients, given an euler rotation
	void DecomposeToBasis(const glm::vec3& point, const glm::vec3& rotation,
		glm::vec3& forward, glm::vec3& right, glm::vec3& up, glm::vec3& coef);

	template<typename T, typename S>
	T Interpolate(T from, T to, S scalar) noexcept {
		if (scalar > 1.0) return to;
		if (scalar < 0.0) return from;
		return from + (to - from) * scalar;
	};

	template<typename T>
	T Remap(T value, T inMin, T inMax, T outMin, T outMax) noexcept {
		return outMin + (((value - inMin) / (inMax - inMin)) * (outMax - outMin));
	};

	template<typename T>
	T RunScalarFunction(Config::ScalarMethods scalarMethod, T interpValue) {
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