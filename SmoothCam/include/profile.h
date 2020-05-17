#pragma once

class Profiler {
	public:
		Profiler() {
			start = GetTime();
		}

		double Snap() const {
			return GetTime() - start;
		}
		
	private:
		double GetTime() const {
			LARGE_INTEGER f, i;
			if (QueryPerformanceCounter(&i) && QueryPerformanceFrequency(&f)) {
				auto frequency = 1.0 / static_cast<double>(f.QuadPart);
				return static_cast<double>(i.QuadPart) * frequency;
			}
			return 0.0;
		}

		double start = 0.0;
};