#include "pch.h"
#include "detours.h"
#include "camera.h"

static PLH::VFuncMap origVFuncs_PlayerInput;
static PLH::VFuncMap origVFuncs_MenuOpenClose;
std::weak_ptr<Camera::SmoothCamera> g_theCamera;

double curFrame = 0.0;
double lastFrame = 0.0;
double GetTime() noexcept {
	LARGE_INTEGER f, i;
	if (QueryPerformanceCounter(&i) && QueryPerformanceFrequency(&f)) {
		auto frequency = 1.0 / static_cast<double>(f.QuadPart);
		return static_cast<double>(i.QuadPart) * frequency;
	}
	return 0.0;
}

void StepFrameTime() noexcept {
	lastFrame = curFrame;
	curFrame = GetTime();
}

float GetFrameDelta() noexcept {
	return glm::max(static_cast<float>(curFrame - lastFrame), 0.000001f);
}

// POV Handler
typedef uintptr_t(__thiscall* OnInput)(PlayerInputHandler*, InputEvent*);
uintptr_t __fastcall mOnInput(PlayerInputHandler* pThis, InputEvent* input) {
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
					}
				}
				
				break;
			}
			default:
				break;
		}
	}
	return reinterpret_cast<OnInput>(origVFuncs_PlayerInput[1])(pThis, input);
}

typedef EventResult(__thiscall* MenuOpenCloseHandler)(uintptr_t pThis, MenuOpenCloseEvent* ev, EventDispatcher<MenuOpenCloseEvent>* dispatcher);
EventResult __fastcall mMenuOpenCloseHandler(uintptr_t pThis, MenuOpenCloseEvent* ev, EventDispatcher<MenuOpenCloseEvent>* dispatcher) {
	if (pThis == (uintptr_t) & (*g_thePlayer)->menuOpenCloseEvent) {
		if (ev->menuName != nullptr && strcmp(ev->menuName, "Dialogue Menu") == 0) {
			std::shared_ptr<Camera::SmoothCamera> lockedPtr;
			if (!g_theCamera.expired() && (lockedPtr = g_theCamera.lock(), lockedPtr != nullptr)) {
				lockedPtr->OnDialogMenuChanged(ev);
			}
		}
	}
	return reinterpret_cast<MenuOpenCloseHandler>(origVFuncs_MenuOpenClose[1])(pThis, ev, dispatcher);
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
	StepFrameTime();

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

// Detour WorldToScreenPt and apply a calculated offset to correct for our modified camera position
RelocAddr<uintptr_t> RunWorldToScreenPt(0x0c66580);
typedef uint64_t(*RunWorldToScreenPtType)(float* worldToCamMatrix, float* viewport, float* p_in, float* x_out, float* y_out, float* z_out,
	float zeroTolerance);
RunWorldToScreenPtType RunWorldToScreenPtFn;
std::unique_ptr<BasicDetour> FindRunWorldToScreenPt = nullptr;

__declspec(noinline) volatile uint64_t __fastcall Detour_RunWorldToScreenPt(
	float* worldToCamMatrix, float* viewport, float* p_in, float* x_out, float* y_out, float* z_out, float zeroTolerance)
{
	std::shared_ptr<Camera::SmoothCamera> lockedPtr;
	if (!g_theCamera.expired() && (lockedPtr = g_theCamera.lock(), lockedPtr != nullptr)) {
		const auto localSpace = lockedPtr->GetGameExpectedPosition() - lockedPtr->GetCurrentPosition();
		p_in[0] = p_in[0] + localSpace.x;
		p_in[1] = p_in[1] + localSpace.y;
		p_in[2] = p_in[2] + localSpace.z;
	}
	
	return RunWorldToScreenPtFn(worldToCamMatrix, viewport, p_in, x_out, y_out, z_out, zeroTolerance);
}

bool Detours::Attach(std::shared_ptr<Camera::SmoothCamera> theCamera) {
	g_theCamera = theCamera;

	{
		UpdateCameraCallerFn = reinterpret_cast<UpdateCameraCallerType>(UpdateCameraCallerSig.GetUIntPtr());
		FindUpdateCameraCaller = std::make_unique<BasicDetour>(
			reinterpret_cast<void**>(&UpdateCameraCallerFn),
			Detour_UpdateCameraCallerFn
		);

		if (!FindUpdateCameraCaller->Attach()) {
			_ERROR("Failed to place detour on target update camera function, this error is fatal.");
			FatalError(L"Failed to place detour on target update camera function, this error is fatal.");
		}
	}

	if (Config::GetCurrentConfig()->patchWorldToScreenMatrix) {
		RunWorldToScreenPtFn = reinterpret_cast<RunWorldToScreenPtType>(RunWorldToScreenPt.GetUIntPtr());
		FindRunWorldToScreenPt = std::make_unique<BasicDetour>(
			reinterpret_cast<void**>(&RunWorldToScreenPtFn),
			Detour_RunWorldToScreenPt
		);

		if (!FindRunWorldToScreenPt->Attach()) {
			_ERROR("Failed to place detour on target function, this error is fatal.");
			FatalError(L"Failed to place detour on target function, this error is fatal.");
		}
	}

	{
		PLH::VFuncSwapHook playerInputHooks(
			(uint64_t)PlayerControls::GetSingleton()->togglePOVHandler,
			{
				{ static_cast<uint16_t>(1), reinterpret_cast<uint64_t>(&mOnInput) },
			},
			&origVFuncs_PlayerInput
		);

		if (!playerInputHooks.hook()) {
			_ERROR("Failed to place detour on target virtual function, this error is fatal.");
			FatalError(L"Failed to place detour on target virtual function, this error is fatal.");
		}
	}

	{
		// Intercept menu open/close events so we can see when the player is talking to another NPC
		PLH::VFuncSwapHook menuOpenCloseHooks(
			(uint64_t)&(*g_thePlayer)->menuOpenCloseEvent,
			{
				{ static_cast<uint16_t>(1), reinterpret_cast<uint64_t>(&mMenuOpenCloseHandler) },
			},
			&origVFuncs_MenuOpenClose
		);

		if (!menuOpenCloseHooks.hook()) {
			_ERROR("Failed to place detour on target virtual function, this error is fatal.");
			FatalError(L"Failed to place detour on target virtual function, this error is fatal.");
		}
	}

	return true;
}