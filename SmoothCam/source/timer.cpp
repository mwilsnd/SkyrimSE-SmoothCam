#include "timer.h"
#include "code_analysis.h"
SILENCE_CODE_ANALYSIS;
#   include <common/ITimer.h>
RESTORE_CODE_ANALYSIS;

static ITimer timer;
static double curFrame = 0.0;
static double lastFrame = 0.0;
static double curQPC = 0.0;
static double lastQPC = 0.0;

void GameTime::Initialize() noexcept {
	timer.Start();
}

double GameTime::GetTime() noexcept {
	return timer.GetElapsedTime();
}

double GameTime::GetQPC() noexcept {
	LARGE_INTEGER f, i;
	if (QueryPerformanceCounter(&i) && QueryPerformanceFrequency(&f)) {
		auto frequency = 1.0 / static_cast<double>(f.QuadPart);
		return static_cast<double>(i.QuadPart) * frequency;
	}
	return 0.0;
}

void GameTime::StepFrameTime() noexcept {
#ifdef _DEBUG
	// @TODO: This is an awful place for this
	Debug::CommandPump();
#endif

	lastFrame = curFrame;
	curFrame = GetTime();

	lastQPC = curQPC;
	curQPC = GetQPC();
}

double GameTime::CurTime() noexcept {
	return curFrame;
}

double GameTime::CurQPC() noexcept {
	return curQPC;
}

double GameTime::GetFrameDelta() noexcept {
	return curFrame - lastFrame;
}

double GameTime::GetQPCDelta() noexcept {
	return curQPC - lastQPC;
}
