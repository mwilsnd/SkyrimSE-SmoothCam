#pragma once

namespace mmath {
	constexpr const float half_pi = 1.57079632679485f;

	typedef struct {
		float data[4][4];
	} NiMatrix44;

	bool IsInf(const float f) noexcept;
	bool IsInf(const double f) noexcept;
	bool IsInf(const glm::vec3& v) noexcept;
	bool IsInf(const glm::vec4& v) noexcept;

	bool IsNan(const float f) noexcept;
	bool IsNan(const double f) noexcept;
	bool IsNan(const glm::vec3& v) noexcept;
	bool IsNan(const glm::vec4& v) noexcept;

	bool IsValid(const float f) noexcept;
	bool IsValid(const double f) noexcept;
	bool IsValid(const glm::vec3& v) noexcept;
	bool IsValid(const glm::vec4& v) noexcept;

	// Return the forward view vector
	glm::vec3 GetViewVector(const glm::vec3& forwardRefer, float pitch, float yaw) noexcept;
	// Extracts pitch and yaw from a rotation matrix
	glm::vec3 NiMatrixToEuler(const RE::NiMatrix3& m) noexcept;
	// Decompose a position to 3 basis vectors and the coefficients, given an euler rotation
	void DecomposeToBasis(const glm::vec3& point, const glm::vec3& rotation,
		glm::vec3& forward, glm::vec3& right, glm::vec3& up, glm::vec3& coef) noexcept;
	// Construct a 3D perspective projection matrix that matches what is used by the game
	glm::mat4 Perspective(float fov, float aspect, const RE::NiFrustum& frustum) noexcept;
	// Construct a view matrix
	glm::mat4 LookAt(const glm::vec3& pos, const glm::vec3& at, const glm::vec3& up) noexcept;

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

	using OffsetTransition = TransitionGroup<glm::vec3>;
	using FloatTransition = TransitionGroup<float>;

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

	enum class BindPosition {
		Yes,
		No
	};

	// A position-only transform utility object
	// With BindPosition::Yes, any change to local will update world, and world->local
	template<BindPosition BindPositions = BindPosition::No>
	struct PositionTransform {
		// Position we base the local offset off of
		glm::vec3 ref;
		// Local space relative to refPos
		glm::vec3 local;
		// ref + local
		glm::vec3 world;
		// Forward vector
		glm::vec3 forward;
		// Right vector
		glm::vec3 right;
		// Up vector
		glm::vec3 up;

		// Set the reference for local space computation
		void SetRef(const RE::NiPoint3& pos, const RE::NiPoint3& rot) noexcept {
			SetRef(glm::vec3{ pos.x, pos.y, pos.z }, { rot.x, rot.y, rot.z });
		}

		// Set the reference for local space computation
		void SetRef(const glm::vec3& pos, const glm::vec3& rot) noexcept {
			ref = pos;

			// Compute basis vectors for the ref
			glm::vec3 coef;
			mmath::DecomposeToBasis(
				pos, rot,
				forward, right, up, coef
			);
		}

		// Set the world space position, updates the local position if BindPositions = true
		void SetWorldPosition(const RE::NiPoint3& pos) noexcept {
			SetWorldPosition(glm::vec3{ pos.x, pos.y, pos.z });
		}

		// Set the world space position, updates the local position if BindPositions = true
		void SetWorldPosition(const glm::vec3& pos) noexcept {
			world = pos;
			if constexpr (BindPositions == BindPosition::Yes)
				local = world - ref;
		}

		// Set the local space position, updates the world position if BindPositions = true
		void SetLocalPosition(const RE::NiPoint3& pos) noexcept {
			SetLocalPosition(glm::vec3{ pos.x, pos.y, pos.z });
		}

		// Set the local space position, updates the world position if BindPositions = true
		void SetLocalPosition(const glm::vec3& pos) noexcept {
			local = pos;
			if constexpr (BindPositions == BindPosition::Yes)
				world = ref + local;
		}

		// Set the local position in axis-aligned space, updates the world position if BindPositions = true
		void SetLocalPositionAxisAligned(const glm::vec3& pos, const RE::NiPoint3& refRotation) noexcept {
			SetLocalPositionAxisAligned(pos, glm::vec3{
				refRotation.x,
				refRotation.y,
				refRotation.z
			});
		}

		// Set the local position in axis-aligned space, updates the world position if BindPositions = true
		void SetLocalPositionAxisAligned(const RE::NiPoint3& pos, const RE::NiPoint3& refRotation) noexcept {
			SetLocalPositionAxisAligned(glm::vec3{ pos.x, pos.y, pos.z }, glm::vec3{
				refRotation.x,
				refRotation.y,
				refRotation.z
			});
		}

		// Set the local position in axis-aligned space, updates the world position if BindPositions = true
		void SetLocalPositionAxisAligned(const glm::vec3& pos, const glm::vec3& refRotation) noexcept {
			local =
				(forward * pos.x) +
				(right * pos.y) +
				(up * pos.z);

			if constexpr (BindPositions == BindPosition::Yes)
				world = ref + local;
		}

		// Convert the world position to axis-aligned local space
		glm::vec3 WorldToLocal(const glm::vec2& rotation) noexcept {
			return WorldToLocal({ rotation.x, 0.0f, rotation.y });
		}

		// Convert the world position to axis-aligned local space
		glm::vec3 WorldToLocal(const glm::vec3& rotation) noexcept {
			glm::vec3 f, r, u, coef;
			mmath::DecomposeToBasis(
				world - ref, rotation,
				f, r, u, coef
			);
			return coef;
		}

		// Convert world position to NiPoint3
		RE::NiPoint3 ToNiPoint3() const noexcept {
			return {
				world.x,
				world.y,
				world.z
			};
		}

		// Assign from another
		void operator=(const PositionTransform<BindPositions>& other) noexcept {
			ref = other.ref;
			local = other.local;
			world = other.world;
			forward = other.forward;
			right = other.right;
			up = other.up;
		}
	};

	using BoundPosition = mmath::PositionTransform<mmath::BindPosition::Yes>;
	using Position = mmath::PositionTransform<mmath::BindPosition::No>;

	// Pitch/Yaw rotation
	struct Rotation {
		public:
			glm::quat quat = glm::identity<glm::quat>();
			glm::vec2 euler = { 0.0f, 0.0f };

			// Set euler angles, call UpdateQuaternion to refresh the quaternion part
			void SetEuler(float pitch, float yaw) noexcept;

			// Set with a quaternion and update euler angles
			void SetQuaternion(const glm::quat& q) noexcept;
			void SetQuaternion(const RE::NiQuaternion& q) noexcept;

			// Copy rotation from a TESObjectREFR
			void CopyFrom(const RE::TESObjectREFR* ref) noexcept;

			// Compute a quaternion after setting euler angles
			void UpdateQuaternion() noexcept;

			// Get a quaternion pointing in the opposite direction (p/y inverted)
			glm::quat InverseQuat() const noexcept;
			RE::NiQuaternion InverseNiQuat() const noexcept;

			RE::NiQuaternion ToNiQuat() const noexcept;
			RE::NiPoint2 ToNiPoint2() const noexcept;
			RE::NiPoint3 ToNiPoint3() const noexcept;

			// Get a 4x4 rotation matrix
			glm::mat4 ToRotationMatrix() noexcept;

		private:
			glm::mat4 mat = glm::identity<glm::mat4>();
			bool dirty = true;
	};

	// Smooth changes in scalar overloads
	enum class Local {
		No,
		Yes
	};

	struct TweenStackEntry {
		// Scalar state to transition away from
		// If null, prev will be set and it's result should be used instead
		const Config::OffsetGroupScalar* from = nullptr;
		// Prev entry, if from is null
		const TweenStackEntry* prev = nullptr;
		// Scalar state to transition to
		const Config::OffsetGroupScalar* to = nullptr;
		// Progress in the transition, 0-1 scalar value
		float progress = 0.0f;
		// Duration in seconds a transition should take
		float duration = 1.0f;
		// Start time of the tween
		float startTime = 0.0f;
		// Blend method to use
		Config::ScalarMethods method;

		template<typename T, mmath::Local isLocal = mmath::Local::No>
		T GetBlendResult(T distance) const noexcept {
			if (from == to || progress >= 1.0f) {
				if constexpr (isLocal == mmath::Local::No)
					return RunGlobal<T>(to, distance);
				else
					return RunLocal<T>(to, distance);

			} else {
				T fromRes{};
				T toRes{};

				if constexpr (isLocal == mmath::Local::No)
					toRes = RunGlobal<T>(to, distance);
				else
					toRes = RunLocal<T>(to, distance);

				if (!from) {
					if (!prev) return toRes;
					fromRes = prev->GetBlendResult<T, isLocal>(distance);
				} else {
					if constexpr (isLocal == mmath::Local::No)
						fromRes = RunGlobal<T>(from, distance);
					else
						fromRes = RunLocal<T>(from, distance);
				}

				return mmath::Interpolate<T>(fromRes, toRes, mmath::RunScalarFunction(method, progress));
			}
		}

		template<typename T>
		T RunGlobal(const Config::OffsetGroupScalar* s, T distance) const noexcept {
			constexpr const T minZero = (T)0.000000000001;

			const auto scalar = glm::clamp(
				glm::max(
					(T)1.0 - (static_cast<T>(s->zoomMaxSmoothingDistance) - distance),
					minZero
				) / static_cast<T>(s->zoomMaxSmoothingDistance), (T)0.0, (T)1.0
			);
			auto remapped = mmath::Remap<T>(
				scalar, (T)0.0, (T)1.0,
				static_cast<T>(s->minCameraFollowRate), static_cast<T>(s->maxCameraFollowRate)
			);

			return mmath::RunScalarFunction<T>(s->currentScalar, remapped);
		}

		template<typename T>
		T RunLocal(const Config::OffsetGroupScalar* s, T distance) const noexcept {
			constexpr const T minZero = (T)0.000000000001;

			const auto scalar = glm::clamp(
				glm::max(
					(T)1.0 - (static_cast<T>(s->localMaxSmoothingDistance) - distance),
					minZero
				) / static_cast<T>(s->localMaxSmoothingDistance), (T)0.0, (T)1.0
			);
			auto remapped = mmath::Remap<T>(
				scalar, (T)0.0, (T)1.0,
				static_cast<T>(s->localMinFollowRate), static_cast<T>(s->localMaxFollowRate)
			);

			return mmath::RunScalarFunction<T>(s->separateLocalScalar, remapped);
		}
	};

	struct ScalarTweener {
		std::list<TweenStackEntry> stack = {};
		bool hasEverUpdated = false;
		static constexpr const size_t MAX_STACK = 6;

		inline const Config::OffsetGroupScalar* GetGoal() const noexcept {
			if (stack.empty()) return nullptr;
			return stack.back().to;
		}

		inline void Update(float curTime) noexcept {
			if (stack.empty()) return;
			hasEverUpdated = true;

			for (auto it = stack.begin(); it != stack.end(); ++it) {
				if (it->progress < 1.0f) {
					it->progress = glm::clamp(
						static_cast<float>(curTime - it->startTime) / glm::max(it->duration, 0.01f),
						0.0f, 1.0f
					);
				}
			}
		}

		inline void MoveTo(const Config::OffsetGroupScalar* newState, Config::ScalarMethods blendMethod,
			float curTime, float dur = 1.0f) noexcept
		{
			// Check if we are full, if so we need to evict the oldest
			if (stack.size() == MAX_STACK) {
				stack.pop_front();
				stack.front().prev = nullptr;
			}

			TweenStackEntry entry = {};
			entry.duration = dur;
			entry.startTime = curTime;
			entry.to = newState;
			entry.method = blendMethod;

			// Figure out our from state
			if (stack.empty()) {
				entry.from = entry.to;
				entry.progress = 1.0f;
			} else {
				if (stack.back().progress >= 1.0f)
					entry.from = stack.back().to;
				else {
					entry.from = nullptr; // We are already running a smoothing operation, use the blend result
					entry.prev = &stack.back();
				}
			}

			stack.push_back(eastl::move(entry));
		}

		template<typename T, mmath::Local isLocal = mmath::Local::No>
		T BlendResult(T distance, bool withDT = true) noexcept {
			if (stack.empty()) return (T)1.0;
			const auto result = stack.back().GetBlendResult<T, isLocal>(distance);

			if (withDT) {
				constexpr const T minZero = (T)0.000000000001;
				const T delta = glm::max(static_cast<T>(GameTime::GetFrameDelta()), minZero);
				const T lambda = (T)1.0 - glm::pow((T)1.0 - result, delta * (T)60.0);
				return glm::clamp(lambda, (T)0.0, (T)1.0);
			}

			return result;
		}
	};
}