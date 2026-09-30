#pragma once

#ifdef _DEBUG
#include "debug/ICommand.h"

namespace Debug {
	class DumpGameINI : public ICommand {
		public:
			virtual ~DumpGameINI() override;
			virtual void Run(const std::string& args) noexcept override;
			virtual const std::string_view GetHelpString() const noexcept override;
			virtual const std::string& GetName() const noexcept override {
				return commandName;
			};

		protected:
			const std::string commandName = "dump_game_ini";
			const std::string helpMsg = "Dump all current values in Skyrim.ini as loaded in memory\n";
	};
}
#endif