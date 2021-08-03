#include "pch.h"

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line) {
    return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags,
    unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

extern "C" {
    int __cdecl Vsnprintf8(char* p, size_t n, const char* pFormat, va_list arguments) {
        return vsnprintf_s(p, n, _TRUNCATE, pFormat, arguments);
    }
}