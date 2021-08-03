#pragma once
#ifdef _DEBUG
namespace Debug {
	class ICommand;

	class CommandRegistry {
		public:
			static CommandRegistry* Get() noexcept;
			void Register(eastl::unique_ptr<ICommand>&& command) noexcept;
			ICommand* Find(const eastl::string& name) const noexcept;

			using CommandTable = eastl::unordered_map<eastl::string, eastl::unique_ptr<ICommand>>;
			const CommandTable& GetCommands() const noexcept;

		private:
			CommandRegistry() = default;
			~CommandRegistry() = default;
			CommandRegistry(const CommandRegistry&) = delete;
			CommandRegistry(CommandRegistry&&) noexcept = delete;
			CommandRegistry& operator=(const CommandRegistry&) = delete;
			CommandRegistry& operator=(CommandRegistry&&) noexcept = delete;

			CommandTable registry;
	};
}
#endif