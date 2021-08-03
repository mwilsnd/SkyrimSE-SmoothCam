#ifdef _DEBUG
#include "debug/commands/help.h"
#include "debug/registry.h"

Debug::Help::~Help() {}

void Debug::Help::Run(const eastl::string& args) noexcept {
	puts("All commands:");
	for (const auto& it : Debug::CommandRegistry::Get()->GetCommands()) {
		printf("\t%s - %s\n", it.first.data(), it.second->GetHelpString().data());
	}
	puts("");
}

const eastl::string_view Debug::Help::GetHelpString() const noexcept {
	return helpMsg;
}
#endif