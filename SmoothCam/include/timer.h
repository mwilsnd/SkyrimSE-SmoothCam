#pragma once

namespace GameTime {
	// Get the exact current time
	double GetTime() noexcept;
	// Get the exact current QPC time
	double GetQPC() noexcept;
	// Set the time stamp forward for the current fame
	void StepFrameTime() noexcept;
	// Get the current frame time
	double CurTime() noexcept;
	// Get the current frame QPC time
	double CurQPC() noexcept;
	// Get the current delta time for the frame
	double GetFrameDelta() noexcept;
	// Get the current delta QPC time for the frame
	double GetQPCDelta() noexcept;
}