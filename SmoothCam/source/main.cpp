#include "main.h"
#include "detours.h"
#include "papyrus.h"

#ifdef _DEBUG
#   include "debug_drawing.h"
#endif

PluginHandle g_pluginHandle = kPluginHandle_Invalid;
const SKSEMessagingInterface* g_messaging = nullptr;
const SKSEPapyrusInterface* g_papyrus = nullptr;
std::shared_ptr<Camera::SmoothCamera> g_theCamera = nullptr;
bool hooked = false;

#pragma warning( push )
#pragma warning( disable : 26461 ) // skse function pointer is not const
void SKSEMessageHandler(SKSEMessagingInterface::Message* message) {
	switch (message->type) {
		case SKSEMessagingInterface::kMessage_NewGame:
		case SKSEMessagingInterface::kMessage_PostLoadGame: {
			// The game has loaded, go ahead and hook the camera now
			if (!hooked && g_theCamera) {
				hooked = Detours::Attach(g_theCamera);
			}
			break;
		}
#ifdef _DEBUG
		case SKSEMessagingInterface::kMessage_InputLoaded: {
			DebugDrawing::DetourD3D11();
		}
#endif
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
		info->version = 7;

		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor) {
			return false;
		}

		if (skse->runtimeVersion != RUNTIME_VERSION_1_5_97) {
			_WARNING("This module was compiled for skse 1.5.97, you are running an unsupported verion. You may experience crashes or other strange issues.");
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
		g_theCamera = std::make_shared<Camera::SmoothCamera>();

		_MESSAGE("SmoothCam loaded!");
		return true;
	}
}