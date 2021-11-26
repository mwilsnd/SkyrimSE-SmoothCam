#include "timer.h"

static double curFrame = 0.0;
static double lastFrame = 0.0;
static double curQPC = 0.0;
static double lastQPC = 0.0;

constexpr const size_t DTAccumBufferSize = 40;
static eastl::fixed_ring_buffer<double, DTAccumBufferSize> deltaHistory(DTAccumBufferSize);
static double deltaSmooth = 0.0;

double GameTime::GetTime() noexcept {
	return GetQPC();
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

	deltaHistory.push_back(GetFrameDelta());
	if (lastFrame <= 0.00001) {
		deltaSmooth = 0.00001;
	} else {
		deltaSmooth = 0.0000001;
		for (auto v : deltaHistory) {
			deltaSmooth += v;
		}
		deltaSmooth /= static_cast<double>(deltaHistory.size());
	}
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

double GameTime::GetSmoothFrameDelta() noexcept {
	return deltaSmooth;
}