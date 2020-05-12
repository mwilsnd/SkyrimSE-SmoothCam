#include "addrlib/offsets.h"
#include "addrlib/skse_macros.h"
#include <string>

VersionDb& Offsets::GetDB() {
	static VersionDb db;
	return db;
}

bool Offsets::Initialize() {
	return GetDB().Load();
}

#ifdef _DEBUG
void Offsets::DumpDatabaseTextFile() {
	if (!GetDB().Load(1, 5, 97, 0)) {
		FatalError(L"Failed to load offset database.");
	}

	GetDB().Dump("offsets.txt");
}
#endif

constexpr uintptr_t Offsets::GetByVersionAddr(uintptr_t addr) {
	return addrMap.at(addr);
}

uintptr_t Offsets::GetVersionAddress(uintptr_t addr) {
	return GetOffset(addrMap.at(addr));
}

uintptr_t Offsets::GetOffset(uintptr_t id) {
	uintptr_t ret;
	GetDB().FindOffsetById(id, ret);
	return ret;
}