#pragma once
#include "pch.h"

namespace Camera {
	class SmoothCamera;
}

namespace Detours {
	bool Attach(std::shared_ptr<Camera::SmoothCamera> theCamera);
}