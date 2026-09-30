#pragma once

#ifdef _DEBUG
#include "debug/ICommand.h"

namespace Debug {
	class Help : public ICommand {
		public:
			virtual ~Help() override;
			virtual void Run(const std::string& args) noexcept override;
			virtual const std::string_view GetHelpString() const noexcept override;
			virtual const std::string& GetName() const noexcept override {
				return commandName;
			};

		protected:
			const std::string commandName = "help";
			const std::string helpMsg = "Displays a list of all commands\n";
	};
}
#endif