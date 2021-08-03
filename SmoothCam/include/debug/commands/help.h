#pragma once
#ifdef _DEBUG
#include "debug/ICommand.h"

namespace Debug {
	class Help : public ICommand {
		public:
			virtual ~Help() override;
			virtual void Run(const eastl::string& args) noexcept override;
			virtual const eastl::string_view GetHelpString() const noexcept override;
			virtual const eastl::string& GetName() const noexcept override {
				return commandName;
			};

		protected:
			const eastl::string commandName = "help";
			const eastl::string helpMsg = "Displays a list of all commands\n";
	};
}
#endif