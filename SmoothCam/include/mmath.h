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

	glm::vec3 PointToScreen(const glm::vec3& point);

	// Construct a 3D perspective projection matrix that matches what is used by the game
	glm::mat4 Perspective(float fov, float aspect, const NiFrustum& frustum);
	// Construct a view matrix
	glm::mat4 LookAt(const glm::vec3& pos, const glm::vec3& at, const glm::vec3& up);
	
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

	// A transition state
	template<typename T>
	struct TransitionGroup {
		T lastPosition = {};
		T targetPosition = {};
		T currentPosition = {};
		bool running = false;
		double startTime = 0.0;
	};

	// Run a transition state
	template<typename T, typename S>
	void UpdateTransitionState(double curTime, bool enabled, float duration, Config::ScalarMethods method,
		S& transitionState, const T& currentValue)
	{
		// Check our current offset and see if we need to run a transition 
		if (enabled) {
			if (currentValue != transitionState.targetPosition) {
				// Start the task
				if (transitionState.running)
					transitionState.lastPosition = transitionState.currentPosition;

				transitionState.running = true;
				transitionState.startTime = curTime;
				transitionState.targetPosition = currentValue;
			}

			if (transitionState.running) {
				// Update the transition smoothing
				const auto scalar = glm::clamp(
					static_cast<float>(curTime - transitionState.startTime) / glm::max(duration, 0.01f),
					0.0f, 1.0f
				);

				if (scalar < 1.0f) {
					transitionState.currentPosition = mmath::Interpolate<T, float>(
						transitionState.lastPosition,
						transitionState.targetPosition,
						mmath::RunScalarFunction<float>(method, scalar)
						);
				} else {
					transitionState.currentPosition = transitionState.targetPosition;
					transitionState.running = false;
					transitionState.lastPosition = transitionState.currentPosition;
				}
			} else {
				transitionState.lastPosition = transitionState.targetPosition =
					transitionState.currentPosition = currentValue;
			}
		} else {
			// Disabled
			transitionState.running = false;
			transitionState.lastPosition = transitionState.targetPosition =
				transitionState.currentPosition = currentValue;
		}
	}

	// A fixed-goal transition (from a start position to an end position)
	template<typename T>
	struct FixedTransitionGoal {
		T lastPosition = {};
		bool running = false;
		double startTime = 0.0;
	};
	
	template<typename T, typename S>
	T UpdateFixedTransitionGoal(double curTime, float duration, Config::ScalarMethods method,
		S& fixedGoalState, const T& currentValue)
	{
		if (!fixedGoalState.running) return currentValue;

		const auto scalar = glm::clamp(
			static_cast<float>(curTime - fixedGoalState.startTime) / glm::max(duration, 0.01f),
			0.0f, 1.0f
		);
		fixedGoalState.lastPosition = mmath::Interpolate<T, float>(
			fixedGoalState.lastPosition,
			currentValue,
			mmath::RunScalarFunction<float>(method, scalar)
		);

		if (scalar >= 1.0f) fixedGoalState.running = false;
		return fixedGoalState.lastPosition;
	}
}