#pragma once
#ifdef _DEBUG
namespace Debug {
	class ICommand {
		public:
			virtual ~ICommand() = 0;
			virtual void Run(const eastl::string& args) noexcept = 0;
			virtual const eastl::string_view GetHelpString() const noexcept = 0;
			virtual const eastl::string& GetName() const noexcept = 0;
	};
}
#endif