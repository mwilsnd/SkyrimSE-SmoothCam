#ifdef _DEBUG
#include "debug/commands/get_setting.h"
#include "debug/registry.h"

Debug::GetSetting::~GetSetting() {}

void Debug::GetSetting::Run(const eastl::string& args) noexcept {
	auto setting = (*g_iniSettingCollection)->Get(args.c_str());

	if (!setting) {
		setting = (*g_iniPrefSettingCollection)->Get(args.c_str());
		if (!setting) {
			puts("Setting not found");
			return;
		}
	}

	switch (setting->GetType()) {
		case Setting::kType_Bool: {
			printf("%s :: %s\n", setting->name, setting->data.u8 ? "true" : "false");
			break;
		}
		case Setting::kType_Float: {
			printf("%s :: %f\n", setting->name, setting->data.f32);
			break;
		}
		case Setting::kType_Integer: {
			printf("%s :: %d\n", setting->name, setting->data.s32);
			break;
		}
		case Setting::kType_String: {
			printf("%s :: %s\n", setting->name, setting->data.s);
			break;
		}
		case Setting::kType_ID: {
			printf("%s :: %d\n", setting->name, setting->data.u32);
			break;
		}
		default: {
			printf("%s :: unhandled value type\n",setting->name);
			break;
		}
	}
}

const eastl::string_view Debug::GetSetting::GetHelpString() const noexcept {
	return helpMsg;
}
#endif