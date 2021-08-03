#pragma once
#ifdef _DEBUG
#include "debug/ICommand.h"

namespace Debug {
	class DumpAddrLibDB : public ICommand {
		public:
			virtual ~DumpAddrLibDB() override;
			virtual void Run(const eastl::string& args) noexcept override;
			virtual const eastl::string_view GetHelpString() const noexcept override;
			virtual const eastl::string& GetName() const noexcept override {
				return commandName;
			};

		protected:
			const eastl::string commandName = "dump_addr_db";
			const eastl::string helpMsg = "Dump the offsets database for runtime 1.5.97\n";
	};
}
#endif