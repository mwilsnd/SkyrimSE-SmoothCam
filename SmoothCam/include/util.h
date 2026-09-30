#pragma once
#ifdef DEBUG
namespace Render {
	struct D3DContext;
}
#endif

namespace Util {
	template <typename T, size_t Size>
	class RingBuffer {
		public:
			RingBuffer() = default;

			void push_back(const T& value) {
				if (size_ < Size) {
					data[size_++] = value;
				} else {
					for (size_t i = 0; i + 1 < Size; ++i) {
						data[i] = data[i + 1];
					}
					data[Size - 1] = value;
				}
			}

			size_t size() const noexcept { return size_; }
			bool empty() const noexcept { return size_ == 0; }

			T& front() noexcept { return data[0]; }
			const T& front() const noexcept { return data[0]; }
			T& back() noexcept { return data[size_ - 1]; }
			const T& back() const noexcept { return data[size_ - 1]; }
			T& operator[](size_t i) { return data[i]; }
			const T& operator[](size_t i) const { return data[i]; }

			T* begin() noexcept { return data; }
			const T* begin() const noexcept { return data; }
			const T* cbegin() const noexcept { return data; }
			T* end() noexcept { return data + size_; }
			const T* end() const noexcept { return data + size_; }
			const T* cend() const noexcept { return data + size_; }

		private:
			T data[Size] = {};
			size_t size_ = 0;
	};

	enum
	{
		// first 256 for keyboard, then 8 mouse buttons, then mouse wheel up, wheel down, then 16 gamepad buttons
		kMacro_KeyboardOffset = 0,		// not actually used, just for self-documentation
		kMacro_NumKeyboardKeys = 256,

		kMacro_MouseButtonOffset = kMacro_NumKeyboardKeys,	// 256
		kMacro_NumMouseButtons = 8,

		kMacro_MouseWheelOffset = kMacro_MouseButtonOffset + kMacro_NumMouseButtons,	// 264
		kMacro_MouseWheelDirections = 2,

		kMacro_GamepadOffset = kMacro_MouseWheelOffset + kMacro_MouseWheelDirections,	// 266
		kMacro_NumGamepadButtons = 16,

		kMaxMacros = kMacro_GamepadOffset + kMacro_NumGamepadButtons	// 282
	};

	enum
	{
		kGamepadButtonOffset_DPAD_UP = kMacro_GamepadOffset,	// 266
		kGamepadButtonOffset_DPAD_DOWN,
		kGamepadButtonOffset_DPAD_LEFT,
		kGamepadButtonOffset_DPAD_RIGHT,
		kGamepadButtonOffset_START,
		kGamepadButtonOffset_BACK,
		kGamepadButtonOffset_LEFT_THUMB,
		kGamepadButtonOffset_RIGHT_THUMB,
		kGamepadButtonOffset_LEFT_SHOULDER,
		kGamepadButtonOffset_RIGHT_SHOULDER,
		kGamepadButtonOffset_A,
		kGamepadButtonOffset_B,
		kGamepadButtonOffset_X,
		kGamepadButtonOffset_Y,
		kGamepadButtonOffset_LT,
		kGamepadButtonOffset_RT	// 281
	};

	uint32_t GamepadMaskToKeycode(uint32_t keyMask);
	uint32_t GamepadKeycodeToMask(uint32_t keyCode);

	// @Note: We are tagging the 3 0 bits (caused by pointer alignment rules)
	// Tags must be cleared before being dereferenced.
	template<typename T>
	T* TagPointer(T* ptr, uint8_t tag) noexcept {
		constexpr uintptr_t mask = 0xFFFFFFFFFFFFFFFC;
		const auto addr = reinterpret_cast<uintptr_t>(ptr);
		return reinterpret_cast<T*>((addr & mask) | tag);
	}

	template<typename T>
	T* ClearPointerTag(T* ptr) noexcept {
		constexpr uintptr_t mask = 0xFFFFFFFFFFFFFFFC;
		const auto addr = reinterpret_cast<uintptr_t>(ptr);
		return reinterpret_cast<T*>(addr & mask);
	}

	template<typename T>
	uint8_t GetPointerTag(T* ptr) noexcept {
		constexpr uintptr_t tagMask = 0x0000000000000003;
		const auto addr = reinterpret_cast<uintptr_t>(ptr);
		return static_cast<uint8_t>(addr & tagMask);
	}

	template<typename T>
	inline void HashCombine(size_t& seed, const T& v) noexcept {
		std::hash<T> h;
		seed ^= h(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	inline std::string UpperCase(const std::string& str) noexcept {
		std::string result;
		result.reserve(str.length());

		for (auto it = str.cbegin(); it != str.cend(); it++) {
			const auto ch = *it;
			if (ch >= 0x61 && ch < 0x7B)
				result.push_back(ch - 0x20);
			else
				result.push_back(ch);
		}

		return result;
	}

	inline std::string UpperCase(const RE::BSFixedString& str) noexcept {
		std::string result;
		const auto len = std::strlen(str.c_str());
		result.reserve(len);

		for (auto i = 0; i < len; i++) {
			const auto ch = str.c_str()[i];
			if (ch >= 0x61 && ch < 0x7B)
				result.push_back(ch - 0x20);
			else
				result.push_back(ch);
		}

		return result;
	}

	template <typename T>
	std::string IntToHexStr(T w, size_t hex_len = sizeof(T) << 1) {
		static const char* digits = "0123456789ABCDEF";
		auto ret = std::string(hex_len, '0');

		for (size_t i = 0, j = (hex_len - 1) * 4 ; i < hex_len; ++i, j-=4)
			ret[i] = digits[(w >> j) & 0x0f];

		return ret;
	}

#ifdef DEBUG
	void InitializeDebugDrawing(Render::D3DContext& ctx) noexcept;
	void DrawCross3D(const glm::vec3& pos, float extents, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }) noexcept;
	void DrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }) noexcept;
#endif
}