#include "main.h"

const SKSE::MessagingInterface* g_messaging = nullptr;
SKSE::PluginHandle g_pluginHandle = SKSE::kInvalidPluginHandle;
Offsets* g_Offsets = nullptr;
eastl::unique_ptr<Camera::Camera> g_theCamera = nullptr;

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
	switch (message->type) {
		case SKSE::MessagingInterface::kNewGame:
		case SKSE::MessagingInterface::kPostLoadGame: {
			// @Note: coc from the main menu does not fire post load game
			// The game has loaded, go ahead and hook the camera now
			if (!g_theCamera) {
				const auto mdmp = Debug::MiniDumpScope();
				logger::info("Looking for compatible mods");
				Compat::Initialize();
				logger::info("Creating the camera");
				g_theCamera = eastl::make_unique<Camera::Camera>();
				logger::info("Attaching deferred detours");
				if (!Hooks::DeferredAttach())
					logger::critical("Failed to attach deferred detours.");
			}

			break;
		}
		case SKSE::MessagingInterface::kPostLoad: {
			g_messaging->RegisterListener(nullptr, Messaging::HandleInterfaceRequest);
			break;
		}
		default:
			break;
	}
}

#ifdef SKYRIM_SUPPORT_AE
extern "C" __declspec(dllexport) constexpr auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v{};
	v.pluginVersion = 16;
	v.PluginName("SmoothCam"sv);
	v.AuthorName("mwilsnd"sv);
#ifdef SKYRIM_IS_PRE629
	v.CompatibleVersions({ SKSE::RUNTIME_1_6_318 });
	v.UsesAddressLibrary(true);
#else
	v.CompatibleVersions({ SKSE::RUNTIME_1_6_640 });
	v.UsesUpdatedStructs();
	v.UsesAddressLibrary();
#endif
	return v;
}();
#endif

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
#ifdef DEBUG
	Debug::StartREPL();
#endif

#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= "SmoothCam";
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "SmoothCam";
	a_info->version = 16;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39 || ver > SKSE::RUNTIME_1_5_97) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
#ifdef DEBUG
#ifdef SKYRIM_SUPPORT_AE
	Debug::StartREPL();
#endif
	while (!IsDebuggerPresent()) {}
#endif

	SKSE::Init(a_skse);
	g_Offsets = &Offsets::Get();

	logger::info("Reading config file");
	Config::Initialize();

#ifdef EMIT_MINIDUMPS
	if (Config::GetCurrentConfig()->enableCrashDumps) {
		logger::info("Installing vectored minidump exception handler");
		Debug::InstallMiniDumpHandler();
	}
#endif
	const auto mdmp = Debug::MiniDumpScope();

	logger::info("Registering plugin interfaces");
	g_messaging = reinterpret_cast<SKSE::MessagingInterface*>(a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));
	if (!g_messaging) {
		logger::critical("Failed to load messaging interface! This error is fatal, will not load.");
		return false;
	}

	auto papyrus = reinterpret_cast<SKSE::PapyrusInterface*>(a_skse->QueryInterface(SKSE::LoadInterface::kPapyrus));
	if (!papyrus) {
		logger::critical("Failed to load scripting interface! This error is fatal, will not load.");
		return false;
	}

	papyrus->Register(PapyrusBindings::Bind);
	g_messaging->RegisterListener("SKSE", SKSEMessageHandler);

	logger::info("Attaching render subsystem hooks");
	if (!Hooks::AttachD3D()) return false;

	Hooks::RegisterGameShutdownEvent([] {
		if (g_theCamera) {
			DebugPrint("Freeing the camera\n");
			g_theCamera.reset();
		}
	});

	logger::info("loaded");
	return true;
}