#pragma once

#ifdef _DEBUG
namespace Debug {
	class ICommand;

	class CommandRegistry {
		public:
			static CommandRegistry* Get() noexcept;
			void Register(std::unique_ptr<ICommand>&& command) noexcept;
			ICommand* Find(const std::string& name) const noexcept;

			using CommandTable = std::unordered_map<std::string, std::unique_ptr<ICommand>>;
			const CommandTable& GetCommands() const noexcept;

		private:
			explicit CommandRegistry() noexcept = default;
			~CommandRegistry() = default;
			CommandRegistry(const CommandRegistry&) = delete;
			CommandRegistry(CommandRegistry&&) noexcept = delete;
			CommandRegistry& operator=(const CommandRegistry&) = delete;
			CommandRegistry& operator=(CommandRegistry&&) noexcept = delete;

			CommandTable registry;
	};
}
#endif