#ifdef EMIT_MINIDUMPS
#include "debug/eh.h"
#include <DbgHelp.h>
#include <Psapi.h>

constexpr auto dmpMsg = LR"(An unrecoverable error occurred during execution of SmoothCam code. Sorry! :(
SmoothCam has created a mini-dump file which the developer can use to fix this problem.

To report this issue:
1). Locate the mini-dump file in the root of your Skyrim directory (The same folder as SkyrimSE.exe). This file is called "SmoothCam_AppCrash.mdmp".

2). Visit the Github issues page (https://github.com/mwilsnd/SkyrimSE-SmoothCam/issues).

3). Create a new issue and try to describe what you were doing when the crash occurred if you can. Upload and link the mini-dump file in your new issue.

3a). If possible, also provide the version of SkyrimSE you are running, your SKSE version and a list of other mods installed (A Net Script Framework crash log also works).)";

constexpr auto dmpFailMsg = LR"(If this issue persist and SmoothCam is unable to write a mini-dump, you can try the following:
1). Run Skyrim as an administrator - Permissions may be preventing the dump file from being written.

2). Ensure there is no locking handle on the file "SmoothCam_AppCrash.mdmp", if such file exists (It would be next to SkyrimSE.exe).

3). Install the latest Visual C++ 2019 Runtime Redistributable.

4). Deactivate other SKSE or .NET mods.)";

static bool dumpRunning = false;
static volatile std::atomic<size_t> vehRC = { 0 };
Debug::MiniDumpScope::MiniDumpScope() noexcept {
	++vehRC;
}

Debug::MiniDumpScope::~MiniDumpScope() noexcept {
	assert(vehRC > 0);
	--vehRC;
}

void WriteMiniDump(EXCEPTION_POINTERS* exceptionInfo) {
	dumpRunning = true;

	constexpr auto flags = MiniDumpNormal | MiniDumpWithDataSegs |
		MiniDumpWithIndirectlyReferencedMemory;

	HANDLE hFile = CreateFile(L"SmoothCam_AppCrash.mdmp", GENERIC_READ | GENERIC_WRITE, 
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 

	if (hFile != NULL && hFile != INVALID_HANDLE_VALUE) {
		MINIDUMP_EXCEPTION_INFORMATION info;
		info.ExceptionPointers = exceptionInfo;
		info.ThreadId = GetCurrentThreadId();
		info.ClientPointers = FALSE;

		if (MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			static_cast<MINIDUMP_TYPE>(flags),
			&info, NULL, NULL
		)) {
			ShowCursor(TRUE);
			MessageBox(nullptr, dmpMsg, L"SmoothCam: Skyrim has crashed!", MB_OK);
		} else {
			ShowCursor(TRUE);
			MessageBox(nullptr, dmpFailMsg, L"SmoothCam: Skyrim has crashed!", MB_OK);
		}

		CloseHandle(hFile); 
	}

	dumpRunning = false;
}

HMODULE GetCurrentModule() {
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurrentModule,
		&hModule
	);
	return hModule;
}

LONG WINAPI VectorHandler(struct _EXCEPTION_POINTERS* pointers) {
	if (!IS_ERROR(pointers->ExceptionRecord->ExceptionCode) || vehRC == 0 || dumpRunning ||
		IS_UNWINDING(pointers->ExceptionRecord->ExceptionFlags))
		return EXCEPTION_CONTINUE_SEARCH;

	// Figure out if we are in the stack
	// If so, run the mini dump and fastfail
	STACKFRAME64 stack;
	stack.AddrPC.Offset = pointers->ContextRecord->Rip;
	stack.AddrFrame.Offset = pointers->ContextRecord->Rbp;
	stack.AddrStack.Offset = pointers->ContextRecord->Rsp;
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrFrame.Mode = AddrModeFlat;
	stack.AddrStack.Mode = AddrModeFlat;

	HANDLE hProcess = GetCurrentProcess();
	HMODULE hModule = GetCurrentModule();
	HANDLE hThread = GetCurrentThread();

	MODULEINFO mi;
	GetModuleInformation(hProcess, hModule, &mi, sizeof(mi));

	// @Note: We could at some point have enough code (in theroy) to generate more than one .text section.
	// 4 should be plenty
	struct TextRegion {
		uintptr_t start = 0;
		uintptr_t end = 0;
	};
	TextRegion codeSegs[4] = {};
	uint8_t numSegs = 0;
	
	const auto moduleBaseAddr = (uintptr_t)hModule;
	const auto ntHeader = ImageNtHeader(hModule);
	auto sec = IMAGE_FIRST_SECTION(ntHeader);
	for (auto i = 0; i < ntHeader->FileHeader.NumberOfSections; i++) {
		if (memcmp(sec->Name, ".text\x00\x00\x00", 8) == 0) {
			codeSegs[numSegs++] = { moduleBaseAddr+sec->VirtualAddress, moduleBaseAddr+sec->Misc.VirtualSize };
			if (numSegs >= 4) break;
		}
		sec++;
	}

	auto ctx = *pointers->ContextRecord;

	do {
		if (stack.AddrPC.Offset != 0) {
			// If PC is anywhere in the range of our .text section(s), invoke the mini dump
			for (auto seg = 0; seg < numSegs; seg++) {
				if (stack.AddrPC.Offset >= codeSegs[seg].start && stack.AddrPC.Offset <= codeSegs[seg].end) {
					WriteMiniDump(pointers);
					__fastfail(FAST_FAIL_FATAL_APP_EXIT);
					break;
				}
			}
		}

	} while (StackWalk64(
		IMAGE_FILE_MACHINE_AMD64,
		hProcess,
		hThread,
		&stack,
		&ctx,
		NULL,
		SymFunctionTableAccess64,
		SymGetModuleBase64,
		NULL
	));

	return EXCEPTION_CONTINUE_SEARCH;
}

static HANDLE vehHandle = nullptr;
void Debug::InstallMiniDumpHandler() noexcept {
	assert(!vehHandle);
	// @Note: A process that calls SymInitialize should not call it again unless it calls SymCleanup first.
	// I don't see a way to query if the symbol handler has already been initialized.
	// In testing, calling it twice and then causing an exception seems to still function, and the only other mod
	// which uses dbghelp as far as i'm aware is the net script framework's crash log, which seems to function normally.
	SymInitialize(GetCurrentProcess(), NULL, TRUE);
	SymSetOptions(SYMOPT_LOAD_LINES);
	vehHandle = AddVectoredExceptionHandler(1, VectorHandler);
}

void Debug::RemoveMiniDumpHandler() noexcept {
	if (vehHandle)
		RemoveVectoredExceptionHandler(vehHandle);
	vehHandle = nullptr;
}
#endif