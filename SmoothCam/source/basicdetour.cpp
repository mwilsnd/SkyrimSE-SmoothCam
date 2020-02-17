#include "pch.h"
#include "basicdetour.h"

BasicDetour::BasicDetour(void** old, void* replacement) noexcept : attached(false) {
	fnOrig = old;
	fnDetour = replacement;
}

bool BasicDetour::Attach() noexcept {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	const auto ok = DetourAttach(fnOrig, fnDetour) == NO_ERROR;
	if (ok)
		DetourTransactionCommit();
	else
		DetourTransactionAbort();

	attached = ok;
	return ok;
}

void BasicDetour::Detach() noexcept {
	if (!attached)
		return;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourDetach(fnOrig, fnDetour);

	DetourTransactionCommit();
}