#ifdef _DEBUG
#include "debug/registry.h"
#include "debug/ICommand.h"

Debug::CommandRegistry* Debug::CommandRegistry::Get() noexcept {
	static CommandRegistry registry;
	return &registry;
}

void Debug::CommandRegistry::Register(eastl::unique_ptr<ICommand>&& command) noexcept {
	registry.insert({ command->GetName(), eastl::move(command) });
}

const Debug::CommandRegistry::CommandTable& Debug::CommandRegistry::GetCommands() const noexcept {
	return registry;
}

Debug::ICommand* Debug::CommandRegistry::Find(const eastl::string& name) const noexcept {
	const auto it = registry.find(name);
	if (it == registry.end()) return nullptr;
	return it->second.get();
}
#endif