#include "compat.h"
#include "modapi.h"
#include <Psapi.h>
#include <DbgHelp.h>

extern Offsets* g_Offsets;
Compat::ModDetectionFlags modDetectionFlags = {};

static struct {
	const RE::TESFile* ifpvDetector = nullptr;
	const RE::TESGlobal* ifpvIsFirstPerson = nullptr;
	const RE::TESFile *ago = nullptr;

	Compat::ICCheckResult icDetectResult = Compat::ICCheckResult::NOT_FOUND;
	HMODULE hImprovedCamera = NULL;
} detectedMods;

Compat::ICCheckResult loadImprovedCameraHandle(HMODULE& mod) noexcept {
	auto hMod = GetModuleHandle(L"ImprovedCamera.dll");
	if (hMod == NULL) return Compat::ICCheckResult::NOT_FOUND;
	MODULEINFO mi;
	GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));
	if (mi.SizeOfImage != Compat::ICSignatures::SizeOfImage) return Compat::ICCheckResult::VERSION_MISMATCH;

	const auto ntHeader = ImageNtHeader(hMod);
	if (ntHeader->Signature != Compat::ICSignatures::Signature ||
		ntHeader->OptionalHeader.AddressOfEntryPoint != Compat::ICSignatures::AddressOfEntryPoint ||
		ntHeader->FileHeader.TimeDateStamp != Compat::ICSignatures::TimeDateStamp)
		return Compat::ICCheckResult::VERSION_MISMATCH;

	DWORD dwHandle = 0;
	const auto sz = GetFileVersionInfoSize(L"ImprovedCamera.dll", &dwHandle);
	if (sz != 0) {
		LPSTR verData = (LPSTR)malloc(sizeof(char) * sz);

		if (GetFileVersionInfo(L"ImprovedCamera.dll", dwHandle, sz, verData)) {
			LPBYTE lpBuffer = NULL;
			UINT size = 0;
			if (VerQueryValue(verData, L"\\", reinterpret_cast<void**>(&lpBuffer), &size) && size) {
				const VS_FIXEDFILEINFO* verInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(lpBuffer);
				if (verInfo->dwSignature == 0xfeef04bd) {
					const auto v0 = (verInfo->dwFileVersionMS >> 16) & 0xffff;
					const auto v1 = (verInfo->dwFileVersionMS >> 0) & 0xffff;
					const auto v2 = (verInfo->dwFileVersionLS >> 16) & 0xffff;
					const auto v3 = (verInfo->dwFileVersionLS >> 0) & 0xffff;

					// Now check for our match
					if (v0 == Compat::ICSignatures::FileVersion[0] && v1 == Compat::ICSignatures::FileVersion[1] &&
						v2 == Compat::ICSignatures::FileVersion[2] && v3 == Compat::ICSignatures::FileVersion[3])
						mod = hMod;
				}
			}
		}

		free(verData);
	}

	return hMod != NULL ? Compat::ICCheckResult::OK : Compat::ICCheckResult::VERSION_MISMATCH;
}

void Compat::Initialize() noexcept {
	auto handler = RE::TESDataHandler::GetSingleton();
	detectedMods.ago = handler->LookupModByName("DSerArcheryGameplayOverhaul.esp");
	detectedMods.icDetectResult = loadImprovedCameraHandle(detectedMods.hImprovedCamera);

	detectedMods.ifpvDetector = handler->LookupModByName("IFPVDetector.esl");
	if (detectedMods.ifpvDetector) {
		detectedMods.ifpvIsFirstPerson = reinterpret_cast<const RE::TESGlobal*>(handler->LookupForm(
			0x801, "IFPVDetector.esl"
		));
	}

	switch (detectedMods.icDetectResult) {
		case ICCheckResult::OK: {
			modDetectionFlags.bImprovedCamera = true;
			logger::info("Found ImprovedCamera.dll beta 1.0.0.4");
			break;
		}
		case ICCheckResult::NOT_FOUND: {
			logger::info("ImprovedCamera.dll beta 1.0.0.4 not found, running without compatibility.");
			break;
		}
		case ICCheckResult::VERSION_MISMATCH: {
			logger::warn("Found ImprovedCamera.dll but unable to validate as beta 1.0.0.4, please install the offical release module for compatibility support.");
			break;
		}
	}

	if (detectedMods.ifpvDetector) {
		modDetectionFlags.bIFPV = true;
		logger::info("Found IFPVDetector for Immersive First Person View compatibility");
	}
	if (detectedMods.ago) {
		modDetectionFlags.bAGO = true;
		logger::info("Detected Archery Gameplay Overhaul");
	}

	const auto& consumers = Messaging::SmoothCamInterface::GetInstance()->GetConsumers();
	for (const auto& mod : consumers) {
		if (mod == "TrueDirectionalMovement") {
			modDetectionFlags.bTDM = true;
			break;
		}
	}
}

bool Compat::IsPresent(Compat::Mod mod) noexcept {
	switch (mod) {
		case Compat::Mod::ImprovedCamera:
			return detectedMods.hImprovedCamera != NULL && detectedMods.icDetectResult == ICCheckResult::OK;
		case Compat::Mod::ImmersiveFirstPersonView:
			return detectedMods.ifpvDetector != nullptr;
		case Compat::Mod::ArcheryGameplayOverhaul:
			return detectedMods.ago != nullptr;
		case Compat::Mod::TrueDirectionalMovement:
			return modDetectionFlags.bTDM;
		default: return false;
	}
}

Compat::ICCheckResult Compat::GetICDetectReason() noexcept {
	return detectedMods.icDetectResult;
}

bool Compat::IC_IsFirstPerson() noexcept {
	if (!detectedMods.hImprovedCamera || detectedMods.icDetectResult != ICCheckResult::OK) return false;
	return *reinterpret_cast<bool*>(
		reinterpret_cast<uintptr_t>(detectedMods.hImprovedCamera) + 0x4d510
	);
}

bool Compat::IFPV_IsFirstPerson() noexcept {
	if (!detectedMods.ifpvIsFirstPerson) return false;
	return detectedMods.ifpvIsFirstPerson->value != 0;
}

Compat::ModDetectionFlags& Compat::GetDetectedMods() noexcept {
	return modDetectionFlags;
}

bool Compat::IsConsumerPresent(const char* name) noexcept {
	auto& consumers = Messaging::SmoothCamInterface::GetInstance()->GetConsumers();
	for (auto it = consumers.cbegin(); it != consumers.cend(); ++it) {
		if (it->compare(name) == 0) return true;
	}
	return false;
}