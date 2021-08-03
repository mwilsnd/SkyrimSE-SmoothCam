#pragma once
#ifdef _DEBUG
#include "debug/ICommand.h"

namespace Debug {
	class GetSetting : public ICommand {
		public:
			virtual ~GetSetting() override;
			virtual void Run(const eastl::string& args) noexcept override;
			virtual const eastl::string_view GetHelpString() const noexcept override;
			virtual const eastl::string& GetName() const noexcept override {
				return commandName;
			};

		protected:
			const eastl::string commandName = "get_setting";
			const eastl::string helpMsg = "Get the value of the named setting\n";
	};
}
#endif