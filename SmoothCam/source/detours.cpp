#include "pch.h"
#include "detours.h"
#include "camera.h"

static PLH::VFuncMap origVFuncs;
std::weak_ptr<Camera::SmoothCamera> g_theCamera;

// This detour was placed just so I could follow code execution through the debugger
// 0x18 -> vfun 3
// CameraUpdate
/*typedef int(__thiscall* TPSV3)(ThirdPersonState*, void*, uintptr_t, uintptr_t, uintptr_t);
void __fastcall mTPSV3(ThirdPersonState* state, void* local_res8, uintptr_t unk1, uintptr_t unk2, uintptr_t unk3) {
	reinterpret_cast<TPSV3>(origVFuncs[3])(state, local_res8, unk1, unk2, unk3);
}*/

typedef uintptr_t(__thiscall* POV1)(PlayerInputHandler*, InputEvent*);
uintptr_t __fastcall mPOV1(PlayerInputHandler* pThis, InputEvent* input) {
	if (input) {
		switch (input->deviceType) {
			case InputEvent::kEventType_Button: {
				const auto ev = reinterpret_cast<ButtonEvent*>(input);
				const BSFixedString* const id = ev->GetControlID();
				if (!id || !id->data) break;

				if (strcmp(id->data, "Toggle POV") == 0) {
					std::shared_ptr<Camera::SmoothCamera> lockedPtr;
					if (!g_theCamera.expired() && (lockedPtr = g_theCamera.lock(), lockedPtr != nullptr)) {
						lockedPtr->OnTogglePOV(ev);
						break;
					}
				}
				break;
			}
			default:
				break;
		}
	}
	return reinterpret_cast<POV1>(origVFuncs[1])(pThis, input);
}

// This method calls the method that then calls update camera
// We detour the caller of the caller of update camera to be better compatible with ImprovedCamera,
// which patches a call instruction in the caller of update camera
RelocAddr<uintptr_t> UpdateCameraCallerSig(0x084ab90);

typedef uintptr_t(*UpdateCameraCallerType)(CorrectedPlayerCamera* pCamera, uint8_t param_2, uint8_t param_3, uint8_t param_4);
UpdateCameraCallerType UpdateCameraCallerFn;
std::unique_ptr<BasicDetour> FindUpdateCameraCaller = nullptr;

__declspec(noinline) volatile uintptr_t __fastcall Detour_UpdateCameraCallerFn(
	CorrectedPlayerCamera* pCamera, uint8_t param_2, uint8_t param_3, uint8_t param_4)
{
	// Allow the camera update code to run first, so we can run after (Improved mod compat) 
	const auto ret = UpdateCameraCallerFn(pCamera, param_2, param_3, param_4);

	// In method disasm, camera appears to only be valid and updatecamera invoked if this is 1
	if (ret == 1) {
		auto player = *g_thePlayer;
		std::shared_ptr<Camera::SmoothCamera> lockedPtr;
		if ((pCamera && player); lockedPtr = g_theCamera.lock(), lockedPtr != nullptr)
			lockedPtr->UpdateCamera(player, pCamera);
	}

	return ret;
}

bool Detours::Attach(std::shared_ptr<Camera::SmoothCamera> theCamera) {
	g_theCamera = theCamera;

	UpdateCameraCallerFn = reinterpret_cast<UpdateCameraCallerType>(UpdateCameraCallerSig.GetUIntPtr());
	FindUpdateCameraCaller = std::make_unique<BasicDetour>(
		reinterpret_cast<void**>(&UpdateCameraCallerFn),
		Detour_UpdateCameraCallerFn
	);

	if (!FindUpdateCameraCaller->Attach()) {
		_ERROR("Failed to place detour on target update camera function, this error is fatal.");
		FatalError(L"Failed to place detour on target update camera function, this error is fatal.");
	}
	
	PLH::VFuncMap redirect = {
		{ static_cast<uint16_t>(1), reinterpret_cast<uint64_t>(&mPOV1) },
	};

	// Any PlayerInputHandler instance will do
	// Main reason for doing a vtable detour is just because its easier (to me anyways),
	// and because we only get called when the game would normally call this.
	PLH::VFuncSwapHook hook(
		(uint64_t)PlayerControls::GetSingleton()->togglePOVHandler,
		redirect,
		&origVFuncs
	);

	if (!hook.hook()) {
		_ERROR("Failed to place detour on target virtual function, this error is fatal.");
		FatalError(L"Failed to place detour on target virtual function, this error is fatal."); 
	}

	return true;
}