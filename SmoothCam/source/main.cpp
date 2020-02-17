#include "pch.h"
#include "main.h"
#include "detours.h"
#include "papyrus.h"

PluginHandle g_pluginHandle = kPluginHandle_Invalid;
const SKSEMessagingInterface* g_messaging = nullptr;
const SKSEPapyrusInterface* g_papyrus = nullptr;
std::shared_ptr<Camera::SmoothCamera> g_theCamera = nullptr;
bool hooked = false;

#pragma warning( push )
#pragma warning( disable : 26461 ) // skse function pointer is not const
void SKSEMessageHandler(SKSEMessagingInterface::Message* message) {
	switch (message->type) {
		case SKSEMessagingInterface::kMessage_PostLoadGame: {
			// The game has loaded, go ahead and hook the camera now
			if (!hooked && g_theCamera)
				hooked = Detours::Attach(g_theCamera);
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

#ifdef _DEBUG
		_DMESSAGE("SmoothCam plugin query begin");
#endif

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "SmoothCam";
		info->version = 1;

		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor) {
			return false;
		}

		if (skse->runtimeVersion != RUNTIME_VERSION_1_5_97) {
			_DMESSAGE("This module was compiled for skse 1.5.97, you are running an unsupported verion.");
			return false;
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