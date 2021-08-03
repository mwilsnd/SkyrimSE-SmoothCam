#pragma once

#ifdef _DEBUG
#include <mutex>
#define DebugPrint(...)                                          \
    {std::lock_guard<std::mutex> lock(Debug::GetTerminalLock()); \
    printf(__VA_ARGS__);}

namespace Debug {
    // Locking for stdout may or may not be required, but better to be on the safe side
    std::mutex& GetTerminalLock() noexcept;

    void StartREPL(FILE* outSteam = stdout) noexcept;
    void CommandPump() noexcept;
}

#else
#define DebugPrint(...)
#endif