#pragma once

class StopWatch {
	public:
		StopWatch() {
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

template<size_t history>
class Profiler {
	public:
		Profiler() {}

		void Push() {
			start = GetTime();
		}

		void Pop() {
			buf.push_back(GetTime() - start);
		}

		double Avg() const {
			if (buf.size() == 0) return 0.0;
			return std::accumulate(buf.cbegin(), buf.cend(), 0.0) / buf.size();
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
		eastl::fixed_ring_buffer<double, history> buf = eastl::fixed_ring_buffer<double, history>(history);
};