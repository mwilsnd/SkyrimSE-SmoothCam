#pragma once

namespace Camera {
	class SmoothCamera;
}

namespace Detours {
	bool Attach(std::shared_ptr<Camera::SmoothCamera> theCamera);

	typedef void(__thiscall* CameraOnUpdate)(TESCameraState*, void*);
	class CameraStateDetour {
		public:
		CameraStateDetour(TESCameraState* pThis, uint16_t idx, uint64_t fnDetour, PLH::VFuncMap& origVFuncs) {
			PLH::VFuncSwapHook hook(
				(uint64_t)pThis,
				{
					{ idx, fnDetour },
				},
				&origVFuncs
				);

			if (!hook.hook()) {
				_ERROR("Failed to place detour on target virtual function, this error is fatal.");
				FatalError(L"Failed to place detour on target virtual function, this error is fatal.");
			}
		}
	};
}