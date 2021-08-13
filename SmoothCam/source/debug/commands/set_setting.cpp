#ifdef _DEBUG
#include "debug/commands/set_setting.h"
#include "debug/registry.h"

Debug::SetSetting::~SetSetting() {}

void Debug::SetSetting::Run(const eastl::string& args) noexcept {
	const auto pos = args.find_first_of(' ', 0);
	auto name = pos != std::string::npos ? args.substr(0, pos) : args;
	auto value = pos != std::string::npos ? args.substr(glm::min(pos+1, args.length())) : "";

	auto setting = (*g_iniSettingCollection)->Get(name.c_str());

	if (!setting) {
		setting = (*g_iniPrefSettingCollection)->Get(name.c_str());
		if (!setting) {
			puts("Setting not found");
			return;
		}
	}

	switch (setting->GetType()) {
		case Setting::kType_Bool: {
			setting->data.u8 = value.compare("true") == 0;
			break;
		}
		case Setting::kType_Float: {
			setting->data.f32 = std::stof(value.c_str());
			break;
		}
		case Setting::kType_Integer: {
			setting->data.s32 = std::stoi(value.c_str());
			break;
		}
		case Setting::kType_ID: {
			setting->data.u32 = std::stoul(value.c_str());
			break;
		}
		default: {
			printf("%s :: unhandled value type\n",setting->name);
			break;
		}
	}
}

const eastl::string_view Debug::SetSetting::GetHelpString() const noexcept {
	return helpMsg;
}
#endif