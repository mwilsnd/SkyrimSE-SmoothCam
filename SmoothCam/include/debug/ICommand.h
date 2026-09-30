#pragma once

#ifdef _DEBUG
namespace Debug {
	class ICommand {
		public:
			virtual ~ICommand() = 0;
			virtual void Run(const std::string& args) noexcept = 0;
			virtual const std::string_view GetHelpString() const noexcept = 0;
			virtual const std::string& GetName() const noexcept = 0;
	};
}
#endif