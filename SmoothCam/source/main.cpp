#include "main.h"

PluginHandle g_pluginHandle = kPluginHandle_Invalid;
const SKSEMessagingInterface* g_messaging = nullptr;
const SKSEPapyrusInterface* g_papyrus = nullptr;
static bool hooked = false;
static bool d3dHooked = false;

std::shared_ptr<Camera::SmoothCamera> g_theCamera = nullptr;
#ifdef WITH_D2D
std::unique_ptr<Render::D2D> g_D2D = nullptr;
#endif

#pragma warning( push )
#pragma warning( disable : 26461 ) // skse function pointer is not const
void SKSEMessageHandler(SKSEMessagingInterface::Message* message) {
	switch (message->type) {
		case SKSEMessagingInterface::kMessage_NewGame:
		case SKSEMessagingInterface::kMessage_PostLoadGame: {
			// The game has loaded, go ahead and hook the camera now
			if (!hooked) {
				g_theCamera = std::make_shared<Camera::SmoothCamera>();
				hooked = Detours::Attach();
			}
			break;
		}
		case SKSEMessagingInterface::kMessage_PreLoadGame: {
			if (!d3dHooked) {
				// Wait until now to ensure D3D is loaded and we aren't racing it
				Render::InstallHooks();
#ifdef WITH_D2D
				if (Render::HasContext())
					g_D2D = std::make_unique<Render::D2D>(Render::GetContext());
#endif
				d3dHooked = true;
			}
		}
		default:
			break;
	}
}
#pragma warning( pop )

// Let's be nice and (try to) cleanly release our resources
// Do this here before the game nukes com
typedef uintptr_t(*CalledDuringRenderShutdown)();
static CalledDuringRenderShutdown fnCalledDuringRenderShutdown;
static std::unique_ptr<BasicDetour> detCalledDuringRenderShutdown;
static uintptr_t mCalledDuringRenderShutdown() {
	if (hooked)
		g_theCamera.reset();

#ifdef WITH_D2D
	g_D2D.reset();
#endif
	Render::Shutdown();

	return fnCalledDuringRenderShutdown();
}

extern "C" {
	__declspec(dllexport) bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info) {
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\SmoothCam.log");
		gLog.SetPrintLevel(IDebugLog::kLevel_DebugMessage);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

#ifdef _DEBUG
		_DMESSAGE("SmoothCam plugin query begin");
#endif

		if (!Offsets::Initialize()) {
			_ERROR("Failed to load game offset database. Visit https://www.nexusmods.com/skyrimspecialedition/mods/32444 to aquire the correct database file.");
			FatalError(L"Failed to load game offset database. Visit https://www.nexusmods.com/skyrimspecialedition/mods/32444 to aquire the correct database file.");
			return false;
		}

//#ifdef _DEBUG
//		Offsets::DumpDatabaseTextFile();
//#endif

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "SmoothCam";
		info->version = 11;

		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor) {
			return false;
		}

		if (skse->runtimeVersion != RUNTIME_VERSION_1_5_97) {
			_WARNING("This module was compiled for skse 1.5.97, you are running an unsupported version. You may experience crashes or other strange issues.");
		}
		
		return true;
	}

	__declspec(dllexport) bool SKSEPlugin_Load(const SKSEInterface* skse) {
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

		Config::ReadConfigFile();


		{
			fnCalledDuringRenderShutdown = Offsets::Get<CalledDuringRenderShutdown>(75446);
			detCalledDuringRenderShutdown = std::make_unique<BasicDetour>(
				reinterpret_cast<void**>(&fnCalledDuringRenderShutdown),
				mCalledDuringRenderShutdown
			);

			if (!detCalledDuringRenderShutdown->Attach()) {
				_ERROR("Failed to place detour on target function, this error is fatal.");
				FatalError(L"Failed to place detour on target function, this error is fatal.");
			}
		}

		_MESSAGE("SmoothCam loaded!");
		return true;
	}
}