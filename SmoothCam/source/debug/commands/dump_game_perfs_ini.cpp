#ifdef _DEBUG
#include "debug/commands/dump_game_perfs_ini.h"

Debug::DumpGamePerfsINI::~DumpGamePerfsINI() {}

void Debug::DumpGamePerfsINI::Run(const eastl::string& args) noexcept {
	auto settings = *g_iniPrefSettingCollection;
	auto node = &settings->items;

	puts("Current INI values:");

	while (node) {
		switch (node->setting->GetType()) {
			case Setting::kType_Bool: {
				printf("\t%s :: %s\n", node->setting->name, node->setting->data.u8 ? "true" : "false");
				break;
			}
			case Setting::kType_Float: {
				printf("\t%s :: %f\n", node->setting->name, node->setting->data.f32);
				break;
			}
			case Setting::kType_Integer: {
				printf("\t%s :: %d\n", node->setting->name, node->setting->data.s32);
				break;
			}
			case Setting::kType_String: {
				printf("\t%s :: %s\n", node->setting->name, node->setting->data.s);
				break;
			}
			case Setting::kType_ID: {
				printf("\t%s :: %d\n", node->setting->name, node->setting->data.u32);
				break;
			}
			default: {
				printf("\t%s :: unhandled value type\n", node->setting->name);
				break;
			}
		}
		node = node->next;
	}

	puts("");
}

const eastl::string_view Debug::DumpGamePerfsINI::GetHelpString() const noexcept {
	return helpMsg;
}
#endif