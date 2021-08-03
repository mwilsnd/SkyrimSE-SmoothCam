#pragma once
#ifdef _DEBUG
#include "debug/ICommand.h"

namespace Debug {
	class SetSetting : public ICommand {
		public:
			virtual ~SetSetting() override;
			virtual void Run(const eastl::string& args) noexcept override;
			virtual const eastl::string_view GetHelpString() const noexcept override;
			virtual const eastl::string& GetName() const noexcept override {
				return commandName;
			};

		protected:
			const eastl::string commandName = "set_setting";
			const eastl::string helpMsg = "Set the value of the named setting\n";
	};
}
#endif