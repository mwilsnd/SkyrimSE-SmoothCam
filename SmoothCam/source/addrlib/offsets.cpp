#include "addrlib/offsets.h"
#include "addrlib/skse_macros.h"

static bool dbLoaded = false;
static std::unique_ptr<VersionDb> db;
static eastl::unordered_map<uintptr_t, uintptr_t> cache;

std::unique_ptr<VersionDb>& Offsets::GetDB() {
	return db;
}

bool Offsets::Initialize() {
	db = std::make_unique<VersionDb>();
	const auto status = db->Load();
	if (status) {
		dbLoaded = true;
		for (const auto& it : addrMap)
			CacheID(it.second);
	}
	return status;
}

void Offsets::ReleaseDB() noexcept {
	dbLoaded = false;
	db.reset();
}

void Offsets::CacheID(uintptr_t ofsID) noexcept {
	cache.insert({ ofsID, reinterpret_cast<uintptr_t>(db->FindAddressById(ofsID)) });
}

#ifdef _DEBUG
void Offsets::DumpDatabaseTextFile() {
	if (!db->Load(1, 5, 97, 0)) {
		FatalError(L"Failed to load offset database.");
	}

	db->Dump("offsets.txt");
}
#endif

constexpr uintptr_t Offsets::GetByVersionAddr(uintptr_t addr) {
	return addrMap.at(addr);
}

uintptr_t Offsets::GetVersionAddress(uintptr_t addr) noexcept {
	return GetOffset(addrMap.at(addr));
}

uintptr_t Offsets::GetOffset(uintptr_t id) noexcept {
	if (dbLoaded) {
		return reinterpret_cast<uintptr_t>(db->FindAddressById(id));
	} else {
		return cache.find(id)->second;
	}
}