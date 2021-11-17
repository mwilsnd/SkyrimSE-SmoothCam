#ifdef _DEBUG
#include "debug/commands/get_setting.h"

Debug::GetSetting::~GetSetting() {}

void Debug::GetSetting::Run(const eastl::string& args) noexcept {
	auto setting = RE::INISettingCollection::GetSingleton()->GetSetting(args.c_str());

	if (!setting) {
		setting = RE::INIPrefSettingCollection::GetSingleton()->GetSetting(args.c_str());
		if (!setting) {
			puts("Setting not found");
			return;
		}
	}

	switch (setting->GetType()) {
		case RE::Setting::Type::kBool: {
			printf("%s :: %s\n", setting->name, setting->GetBool() ? "true" : "false");
			break;
		}
		case RE::Setting::Type::kFloat: {
			printf("%s :: %f\n", setting->name, setting->GetFloat());
			break;
		}
		case RE::Setting::Type::kSignedInteger: {
			printf("%s :: %d\n", setting->name, setting->GetSInt());
			break;
		}
		case RE::Setting::Type::kString: {
			printf("%s :: %s\n", setting->name, setting->GetString());
			break;
		}
		case RE::Setting::Type::kUnsignedInteger: {
			printf("%s :: %d\n", setting->name, setting->GetUInt());
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