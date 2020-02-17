#include "pch.h"

Config::UserConfig currentConfig;

void Config::ReadConfigFile() {
	Config::UserConfig cfg;

	std::ifstream is(L"Data/SKSE/Plugins/SmoothCam.json");
	if (is.good()) {
		cereal::JSONInputArchive archive(is);
		try {
			archive(cfg);
		} catch (cereal::Exception& e) {
			// Welp, something broke
			// Save the default config
			_WARNING("%s - %s", "Failed to load user config! Loading Defaults. Error message:", e.what());
			SaveCurrentConfig();
		}
	} else {
		// File not found? save our defaults
		SaveCurrentConfig();
	}

	currentConfig = cfg;
}

void Config::SaveCurrentConfig() {
	std::ofstream os(L"Data/SKSE/Plugins/SmoothCam.json");
	cereal::JSONOutputArchive archive(os);
	archive(currentConfig);
}

Config::UserConfig* Config::GetCurrentConfig() noexcept {
	return &currentConfig;
}