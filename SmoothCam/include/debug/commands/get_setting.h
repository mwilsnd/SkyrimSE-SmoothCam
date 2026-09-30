#pragma once

#ifdef _DEBUG
#include "debug/ICommand.h"

namespace Debug {
	class GetSetting : public ICommand {
		public:
			virtual ~GetSetting() override;
			virtual void Run(const std::string& args) noexcept override;
			virtual const std::string_view GetHelpString() const noexcept override;
			virtual const std::string& GetName() const noexcept override {
				return commandName;
			};

		protected:
			const std::string commandName = "get_setting";
			const std::string helpMsg = "Get the value of the named setting\n";
	};
}
#endif