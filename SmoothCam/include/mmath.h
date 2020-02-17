#pragma once
#include "pch.h"

namespace mmath {
	template<typename T, typename S>
	T Interpolate(T from, T to, S scalar) noexcept {
		if (scalar > 1.0) return to;
		if (scalar < 0.0) return from;
		return from + (to - from) * scalar;
	}

	template<typename T>
	T Remap(T value, T inMin, T inMax, T outMin, T outMax) noexcept {
		return outMin + (((value - inMin) / (inMax - inMin)) * (outMax - outMin));
	}

	bool IsInf(const float& f) noexcept;
	bool IsInf(const glm::vec3& v) noexcept;
	bool IsInf(const glm::vec4& v) noexcept;

	bool IsNan(const float& f) noexcept;
	bool IsNan(const glm::vec3& v) noexcept;
	bool IsNan(const glm::vec4& v) noexcept;

	bool IsValid(const float& f) noexcept;
	bool IsValid(const glm::vec3& v) noexcept;
	bool IsValid(const glm::vec4& v) noexcept;
}