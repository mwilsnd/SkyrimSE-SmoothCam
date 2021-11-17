#pragma once

namespace Compat {
	enum class Mod : uint8_t {
		ImprovedCamera,
		ImmersiveFirstPersonView,
		ArcheryGameplayOverhaul,
		TrueDirectionalMovement,
	};

	enum class ICCheckResult : uint8_t {
		OK = 0,
		NOT_FOUND = 1,
		VERSION_MISMATCH = 2
	};

#ifdef IS_SKYRIM_AE
	namespace ICSignatures {
		constexpr const DWORD SizeOfImage = 0xFFFFFFFF;
		constexpr const DWORD Signature = 0xFFFFFFFF;
		constexpr const DWORD AddressOfEntryPoint = 0xFFFFFFFF;
		constexpr const DWORD TimeDateStamp = 0xFFFFFFFF;
		constexpr const DWORD FileVersion[4] = { 0, 0, 0, 0 };
	};
#else
	namespace ICSignatures {
		constexpr const DWORD SizeOfImage = 0x00054000;
		constexpr const DWORD Signature = 0x00004550;
		constexpr const DWORD AddressOfEntryPoint = 0x0001b0a4;
		constexpr const DWORD TimeDateStamp = 0x5d3e15f0;
		constexpr const DWORD FileVersion[4] = { 1, 0, 0, 4 };
	};
#endif

	struct ModDetectionFlags {
		bool bImprovedCamera = false;
		bool bIFPV = false;
		bool bAGO = false;
		bool bTDM = false;
	};

	void Initialize() noexcept;
	bool IsPresent(Mod mod) noexcept;
	ICCheckResult GetICDetectReason() noexcept;
	bool IC_IsFirstPerson() noexcept;
	bool IFPV_IsFirstPerson() noexcept;
	ModDetectionFlags& GetDetectedMods() noexcept;
	bool IsConsumerPresent(const char* name) noexcept;
}