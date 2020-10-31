#pragma once

namespace Camera {
	class SmoothCamera;
}

namespace Detours {
	bool Attach();

	typedef void(__thiscall* CameraOnUpdate)(TESCameraState*, BSTSmartPointer<TESCameraState>&);
	class CameraStateDetour {
		public:
			CameraStateDetour(TESCameraState* pThis, uint16_t idx, uint64_t fnDetour, PLH::VFuncMap& origVFuncs) {
				hook = std::make_unique<PLH::VFuncSwapHook>(
					(uint64_t)pThis,
					PLH::VFuncMap{
						{ idx, fnDetour },
					},
					&origVFuncs
				);

				if (!hook->hook()) {
					_ERROR("Failed to place detour on target virtual function(TESCameraState Update), this error is fatal.");
					FatalError(L"Failed to place detour on target virtual function(TESCameraState Update), this error is fatal.");
				}
			}

		private:
			std::unique_ptr<PLH::VFuncSwapHook> hook;
	};
}