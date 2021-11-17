#pragma once

#ifdef EMIT_MINIDUMPS
#pragma comment(lib, "Dbghelp.lib")

namespace Debug {
	// Create a MiniDumpScope at the start of a scope where you want to catch
	// fatal crashes - a mdmp will be written if a crash does happen.
	class MiniDumpScope {
		public:
			explicit MiniDumpScope() noexcept;
			~MiniDumpScope() noexcept;
			MiniDumpScope(const MiniDumpScope&) = delete;
			MiniDumpScope(MiniDumpScope&&) noexcept = delete;
			MiniDumpScope& operator=(const MiniDumpScope&) = delete;
			MiniDumpScope& operator=(MiniDumpScope&&) noexcept = delete;
	};

	// Install the vectored exception handler which will handle crashes in our code section
	// and execute a mini dump
	void InstallMiniDumpHandler() noexcept;
	// Remove the exception handler
	void RemoveMiniDumpHandler() noexcept;
}
#else
namespace Debug {
	class MiniDumpScope {};
}
#endif