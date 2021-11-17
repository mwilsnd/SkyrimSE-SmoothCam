#ifdef _DEBUG
#include "debug/commands/set_setting.h"

Debug::SetSetting::~SetSetting() {}

void Debug::SetSetting::Run(const eastl::string& args) noexcept {
	const auto pos = args.find_first_of(' ', 0);
	auto name = pos != eastl::string::npos ? args.substr(0, pos) : args;
	auto value = pos != eastl::string::npos ? args.substr(glm::min(pos+1, args.length())) : "";

	auto setting = RE::INISettingCollection::GetSingleton()->GetSetting(name.c_str());

	if (!setting) {
		setting = RE::INIPrefSettingCollection::GetSingleton()->GetSetting(name.c_str());
		if (!setting) {
			puts("Setting not found");
			return;
		}
	}

	switch (setting->GetType()) {
		case RE::Setting::Type::kBool: {
			setting->data.b = value.compare("true") == 0;
			break;
		}
		case RE::Setting::Type::kFloat: {
			setting->data.f = std::stof(value.c_str());
			break;
		}
		case RE::Setting::Type::kSignedInteger: {
			setting->data.i = std::stoi(value.c_str());
			break;
		}
		case RE::Setting::Type::kUnsignedInteger: {
			setting->data.u = std::stoul(value.c_str());
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