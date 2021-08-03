#include "main.h"
#include "trackir/trackir.h"
#include "render/shader_cache.h"
#include "debug/eh.h"
#include <Psapi.h>
#include <DbgHelp.h>

#ifdef DEBUG
#include "arrow_fixes.h"
#endif

PluginHandle g_pluginHandle = kPluginHandle_Invalid;
const SKSEMessagingInterface* g_messaging = nullptr;
const SKSEPapyrusInterface* g_papyrus = nullptr;
static bool hooked = false;
static bool d3dHooked = false;

eastl::unique_ptr<Camera::Camera> g_theCamera = nullptr;
#ifdef WITH_D2D
eastl::unique_ptr<Render::D2D> g_D2D = nullptr;
#endif

// Module handle to improved camera, if it is loaded
HMODULE hImprovedCamera = NULL;
uint8_t improvedCameraStatus = 1;

// Let's be nice and (try to) cleanly release our resources
// Do this here before the game nukes com
typedef uintptr_t(*CalledDuringRenderShutdown)();
static eastl::unique_ptr<TypedDetour<CalledDuringRenderShutdown>> detCalledDuringRenderShutdown;
static uintptr_t mCalledDuringRenderShutdown() {
	DebugPrint("Shutting down...\n");

#ifdef EMIT_MINIDUMPS
	DebugPrint("Removing minidump handler\n");
	Debug::RemoveMiniDumpHandler();
#endif

	if (hooked) {
		DebugPrint("Freeing the camera\n");
		g_theCamera.reset();
	}

#ifdef WITH_D2D
	DebugPrint("Freeing Direct2D\n");
	g_D2D.reset();
#endif

#ifdef DEVELOPER
	if (TrackIR::IsRunning()) {
		DebugPrint("Shutting down TrackIR\n");
		TrackIR::Shutdown();
	}
#endif

#ifdef DEBUG
	ArrowFixes::Shutdown();
#endif

	DebugPrint("Shutting down the rendering subsystem\n");
	Render::ShaderCache::Get().Release();
	Render::Shutdown();

	DebugPrint("SmoothCam shutdown, continue with game shutdown...\n");
	return detCalledDuringRenderShutdown->GetBase()();
}

void attachD3DHooks() {
	if (!d3dHooked) {
		// Hook the shutdown function first
		{
			const auto mdmp = Debug::MiniDumpScope();
			_MESSAGE("Hooking render shutdown method\n");
			detCalledDuringRenderShutdown = eastl::make_unique<TypedDetour<CalledDuringRenderShutdown>>(
				75446,
				mCalledDuringRenderShutdown
			);
			if (!detCalledDuringRenderShutdown->Attach()) {
				_ERROR("Failed to place detour on target function(Render Shutdown), this error is fatal.");
				FatalError(L"Failed to place detour on target function(Render Shutdown), this error is fatal.");
			}
		}

		// Wait until now to ensure D3D is loaded and we aren't racing it
		_MESSAGE("Hooking D3D11\n");
		Render::InstallHooks();
		if (!Render::HasContext()) {
			WarningPopup(L"SmoothCam: Failed to hook DirectX, Rendering features will be disabled. Try running with overlay software disabled if this warning keeps occurring.");
		}
		d3dHooked = true;

#ifdef WITH_D2D
		if (Render::HasContext()) {
			const auto mdmp = Debug::MiniDumpScope();
			_MESSAGE("Initializing Direct2D\n");
			g_D2D = eastl::make_unique<Render::D2D>(Render::GetContext());
		}
#endif

#ifdef DEVELOPER
		if (Render::HasContext()) {
			const auto mdmp = Debug::MiniDumpScope();
			const auto result = TrackIR::Initialize(Render::GetContext().hWnd);
			if (result != TrackIR::NPResult::OK) {
				_MESSAGE("Failed to load TrackIR interface");
			} else {
				_MESSAGE("TrackIR is running.");
			}
		}
#endif
	}
}

// Try to locate ImprovedCamera in memory and validate the exact file
// We are looking for the beta 4 release version posted on reddit, no
// other version should pass this check. Kind of upset I have to read
// a memory address for full compatibility but whatever.
enum class ICCheckResult {
	OK = 0,
	NOT_FOUND = 1,
	VERSION_MISMATCH = 2
};

namespace ICSignatures {
	constexpr const DWORD SizeOfImage = 0x00054000;
	constexpr const DWORD Signature = 0x00004550;
	constexpr const DWORD AddressOfEntryPoint = 0x0001b0a4;
	constexpr const DWORD TimeDateStamp = 0x5d3e15f0;
	constexpr const DWORD FileVersion[4] = { 1, 0, 0, 4 };
};

ICCheckResult loadImprovedCameraHandle() {
	auto hMod = GetModuleHandle(L"ImprovedCamera.dll");
	if (hMod == NULL) return ICCheckResult::NOT_FOUND;
	MODULEINFO mi;
	GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));
	if (mi.SizeOfImage != ICSignatures::SizeOfImage) return ICCheckResult::VERSION_MISMATCH;

	const auto ntHeader = ImageNtHeader(hMod);
	if (ntHeader->Signature != ICSignatures::Signature ||
		ntHeader->OptionalHeader.AddressOfEntryPoint != ICSignatures::AddressOfEntryPoint ||
		ntHeader->FileHeader.TimeDateStamp != ICSignatures::TimeDateStamp)
		return ICCheckResult::VERSION_MISMATCH;

	DWORD dwHandle;
	auto sz = GetFileVersionInfoSize(L"ImprovedCamera.dll", &dwHandle);
	if (sz != 0) {
		LPSTR verData = (LPSTR)malloc(sizeof(char) * sz);

		if (GetFileVersionInfo(L"ImprovedCamera.dll", dwHandle, sz, verData)) {
			LPBYTE lpBuffer = NULL;
			UINT size = 0;
			if (VerQueryValue(verData, L"\\", reinterpret_cast<void**>(&lpBuffer), &size) && size) {
				VS_FIXEDFILEINFO* verInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(lpBuffer);
				if (verInfo->dwSignature == 0xfeef04bd) {
					auto v0 = (verInfo->dwFileVersionMS >> 16) & 0xffff;
					auto v1 = (verInfo->dwFileVersionMS >> 0) & 0xffff;
					auto v2 = (verInfo->dwFileVersionLS >> 16) & 0xffff;
					auto v3 = (verInfo->dwFileVersionLS >> 0) & 0xffff;

					// Now check for our match
					if (v0 == ICSignatures::FileVersion[0] && v1 == ICSignatures::FileVersion[1] &&
						v2 == ICSignatures::FileVersion[2] && v3 == ICSignatures::FileVersion[3])
						hImprovedCamera = hMod;
				}
			}
		}

		free(verData);
	}

	return hImprovedCamera != NULL ? ICCheckResult::OK : ICCheckResult::VERSION_MISMATCH;
}

#pragma warning( push )
#pragma warning( disable : 26461 ) // skse function pointer is not const
void SKSEMessageHandler(SKSEMessagingInterface::Message* message) {
	switch (message->type) {
		case SKSEMessagingInterface::kMessage_NewGame:
		case SKSEMessagingInterface::kMessage_PostLoadGame: {
			// @Note: coc from the main menu does not fire post load game

			// @Note: New game does not fire preloadgame
			if (message->type == SKSEMessagingInterface::kMessage_NewGame)
				attachD3DHooks();

			// The game has loaded, go ahead and hook the camera now
			if (!hooked) {
				const auto mdmp = Debug::MiniDumpScope();

				_MESSAGE("Creating the camera\n");
				g_theCamera = eastl::make_unique<Camera::Camera>();
				_MESSAGE("Attaching detours\n");
				hooked = Detours::Attach();
			}

			break;
		}
		case SKSEMessagingInterface::kMessage_PreLoadGame: {
			attachD3DHooks();
			break;
		}
		default:
			break;
	}
}
#pragma warning( pop )

extern "C" {
	__declspec(dllexport) bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info) {
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\SmoothCam.log");
		gLog.SetPrintLevel(IDebugLog::kLevel_DebugMessage);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

		_MESSAGE("SKSEPlugin_Query begin");

		if (!Offsets::Initialize()) {
			_ERROR("Failed to load game offset database. Visit https://www.nexusmods.com/skyrimspecialedition/mods/32444 to aquire the correct database file.");
			FatalError(L"Failed to load game offset database. Visit https://www.nexusmods.com/skyrimspecialedition/mods/32444 to aquire the correct database file.");
			return false;
		}

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "SmoothCam";
		info->version = 13;

		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor) {
			return false;
		}

		if (skse->runtimeVersion != RUNTIME_VERSION_1_5_97) {
			_WARNING("This module was compiled for skse 1.5.97, you are running an unsupported version. You may experience crashes or other strange issues.");
		}

		_MESSAGE("Allowing module to load");
		return true;
	}

	__declspec(dllexport) bool SKSEPlugin_Load(const SKSEInterface* skse) {
		_MESSAGE("SKSEPlugin_Load begin");

		_MESSAGE("Reading config file");
		Config::ReadConfigFile();

#ifdef EMIT_MINIDUMPS
		if (Config::GetCurrentConfig()->enableCrashDumps) {
			_MESSAGE("Installing vectored minidump exception handler");
			Debug::InstallMiniDumpHandler();
		}
#endif
		const auto mdmp = Debug::MiniDumpScope();

#ifdef _DEBUG
		Debug::StartREPL();
#endif

		_MESSAGE("Caching offset IDs");
		for (const auto id : idsToCache)
			Offsets::CacheID(id);

#ifndef DEBUG
		_MESSAGE("Releasing the address library database from memory, desired IDs are cached");
		Offsets::ReleaseDB();
#endif

		_MESSAGE("Registering plugin interfaces");
		g_messaging = reinterpret_cast<SKSEMessagingInterface*>(skse->QueryInterface(kInterface_Messaging));
		if (!g_messaging) {
			_ERROR("Failed to load messaging interface! This error is fatal, will not load.");
			FatalError(L"Failed to load messaging interface! This error is fatal, will not load.");
			return false;
		}

		g_papyrus = reinterpret_cast<SKSEPapyrusInterface*>(skse->QueryInterface(kInterface_Papyrus));
		if (!g_papyrus) {
			_ERROR("Failed to load scripting interface! This error is fatal, will not load.");
			FatalError(L"Failed to load scripting interface! This error is fatal, will not load.");
			return false;
		}
		
		g_messaging->RegisterListener(g_pluginHandle, "SKSE", SKSEMessageHandler);
		
		g_papyrus->Register([](VMClassRegistry* registry) {
			PapyrusBindings::Bind(registry);
			_MESSAGE("Papyrus native function registration completed.");
			return true;
		});

		switch (loadImprovedCameraHandle()) {
			case ICCheckResult::OK: {
				_MESSAGE("Found ImprovedCamera.dll beta 1.0.0.4");
				improvedCameraStatus = 0;
				break;
			}
			case ICCheckResult::NOT_FOUND: {
				_MESSAGE("ImprovedCamera.dll beta 1.0.0.4 not found, running without compatibility.");
				improvedCameraStatus = 1;
				break;
			}
			case ICCheckResult::VERSION_MISMATCH: {
				_WARNING("Found ImprovedCamera.dll but unable to validate as beta 1.0.0.4, please install the offical release module for compatibility support.");
				improvedCameraStatus = 2;
				break;
			}
		}

		_MESSAGE("SmoothCam loaded!");
		return true;
	}
}