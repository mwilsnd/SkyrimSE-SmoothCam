#ifdef _DEBUG
#include "debug/commands/dump_game_perfs_ini.h"

Debug::DumpGamePerfsINI::~DumpGamePerfsINI() {}

void Debug::DumpGamePerfsINI::Run(const eastl::string&) noexcept {
	auto settings = RE::INIPrefSettingCollection::GetSingleton();

	puts("Current INI values:");
	for (auto& setting : settings->settings) {
		switch (setting->GetType()) {
			case RE::Setting::Type::kBool: {
				printf("\t%s :: %s\n", setting->name, setting->GetBool() ? "true" : "false");
				break;
			}
			case RE::Setting::Type::kFloat: {
				printf("\t%s :: %f\n", setting->name, setting->GetFloat());
				break;
			}
			case RE::Setting::Type::kSignedInteger: {
				printf("\t%s :: %d\n", setting->name, setting->GetSInt());
				break;
			}
			case RE::Setting::Type::kString: {
				printf("\t%s :: %s\n", setting->name, setting->GetString());
				break;
			}
			case RE::Setting::Type::kUnsignedInteger: {
				printf("\t%s :: %d\n", setting->name, setting->GetUInt());
				break;
			}
			default: {
				printf("\t%s :: unhandled value type\n", setting->name);
				break;
			}
		}
	}

	puts("");
}

const eastl::string_view Debug::DumpGamePerfsINI::GetHelpString() const noexcept {
	return helpMsg;
}
#endif