#include "main.h"
#include "trackir/trackir.h"
#include "render/shader_cache.h"
#include "compat.h"
#include "debug/eh.h"

#ifdef DEBUG
#include "arrow_fixes.h"
#endif

PluginHandle g_pluginHandle = kPluginHandle_Invalid;
const SKSEInterface* g_skse = nullptr;
const SKSEMessagingInterface* g_messaging = nullptr;
const SKSEPapyrusInterface* g_papyrus = nullptr;
static bool hooked = false;
static bool d3dHooked = false;

eastl::unique_ptr<Camera::Camera> g_theCamera = nullptr;
#ifdef WITH_D2D
eastl::unique_ptr<Render::D2D> g_D2D = nullptr;
#endif

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
				_MESSAGE("Looking for compatible mods\n");
				Compat::Initialize();
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
		case SKSEMessagingInterface::kMessage_PostLoad: {
			g_messaging->RegisterListener(g_pluginHandle, nullptr, Messaging::HandleInterfaceRequest);
			break;
		}
		default:
			break;
	}
}
#pragma warning( pop )

extern "C" {
	__declspec(dllexport) bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info) {
		g_skse = skse;

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
		info->version = 15;

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

		_MESSAGE("SmoothCam loaded!");
		return true;
	}
}