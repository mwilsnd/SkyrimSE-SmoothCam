#pragma once
#ifdef _DEBUG
#include "debug/ICommand.h"

namespace Debug {
	class DumpGamePerfsINI : public ICommand {
		public:
			virtual ~DumpGamePerfsINI() override;
			virtual void Run(const eastl::string& args) noexcept override;
			virtual const eastl::string_view GetHelpString() const noexcept override;
			virtual const eastl::string& GetName() const noexcept override {
				return commandName;
			};

		protected:
			const eastl::string commandName = "dump_game_perfs_ini";
			const eastl::string helpMsg = "Dump all current values in SkyrimPerfs.ini as loaded in memory\n";
	};
}
#endif