#ifdef _DEBUG
#include "debug/console.h"
#include "debug/ICommand.h"
#include "debug/registry.h"

#include "debug/commands/dump_addrlib_db.h"
#include "debug/commands/dump_game_ini.h"
#include "debug/commands/dump_game_perfs_ini.h"
#include "debug/commands/get_setting.h"
#include "debug/commands/set_setting.h"
#include "debug/commands/help.h"

static std::mutex consoleLock;
static std::thread repl;

static std::mutex commandListLock;
static eastl::vector<eastl::tuple<eastl::string, eastl::string>> commandList;

std::mutex& Debug::GetTerminalLock() noexcept {
	return consoleLock;
}

void Debug::StartREPL(FILE* outStream) noexcept {
	CommandRegistry::Get()->Register(eastl::make_unique<Debug::DumpAddrLibDB>());
	CommandRegistry::Get()->Register(eastl::make_unique<Debug::DumpGameINI>());
	CommandRegistry::Get()->Register(eastl::make_unique<Debug::DumpGamePerfsINI>());
	CommandRegistry::Get()->Register(eastl::make_unique<Debug::GetSetting>());
	CommandRegistry::Get()->Register(eastl::make_unique<Debug::SetSetting>());
	CommandRegistry::Get()->Register(eastl::make_unique<Debug::Help>());

	repl = std::thread([](FILE* outStream) {
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		FILE* streamOut;
		FILE* streamIn;
		freopen_s(&streamIn, "CONIN$", "r", stdin);
		freopen_s(&streamOut, "CONOUT$", "w", outStream);

		while (true) {
			std::string command;
			std::getline(std::cin, command);

			std::lock_guard<std::mutex> listLock(commandListLock);

			const auto pos = command.find_first_of(' ', 0);
			auto cmd = pos != std::string::npos ? command.substr(0, pos) : command;
			auto args = pos != std::string::npos ? command.substr(glm::min(pos+1, command.length())) : "";
			commandList.push_back(eastl::tie(eastl::string(cmd.c_str()), eastl::string(args.c_str())));
		}
	}, outStream);
	repl.detach();
}

void Debug::CommandPump() noexcept {
	std::lock_guard<std::mutex> listLock(commandListLock);

	for (const auto& [cmd, args] : commandList) {
		auto command = Debug::CommandRegistry::Get()->Find(cmd);
		if (!command) {
			puts("Unknown command");
			continue;
		}
		command->Run(args);
	}

	commandList.clear();
}
#endif