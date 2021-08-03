#ifdef _DEBUG
#include "debug/commands/dump_addrlib_db.h"

Debug::DumpAddrLibDB::~DumpAddrLibDB() {}

void Debug::DumpAddrLibDB::Run(const eastl::string& args) noexcept {
	Offsets::DumpDatabaseTextFile();
	puts("Dumped offsets database to offsets.txt");
}

const eastl::string_view Debug::DumpAddrLibDB::GetHelpString() const noexcept {
	return helpMsg;
}
#endif