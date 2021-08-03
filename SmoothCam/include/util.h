#pragma once

namespace Util {
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
		eastl::hash<T> h;
		seed ^= h(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	inline eastl::string UpperCase(const eastl::string& str) noexcept {
		eastl::string result;
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

	inline eastl::string UpperCase(const BSFixedString& str) noexcept {
		eastl::string result;
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
}