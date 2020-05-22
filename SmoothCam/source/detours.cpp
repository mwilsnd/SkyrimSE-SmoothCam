#include "detours.h"
#include "camera.h"
#include "arrow_fixes.h"

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

#ifdef _DEBUG
	ArrowFixes::Draw();
#endif
}

double GetFrameDelta() noexcept {
	return static_cast<float>(curFrame - lastFrame);
}

#define CAMERA_UPDATE_DETOUR_IMPL(name)															\
static PLH::VFuncMap origVFuncs_##name##;														\
void __fastcall mCameraStateUpdate##name##(TESCameraState* pThis, void* unk) {					\
	reinterpret_cast<Detours::CameraOnUpdate>(origVFuncs_##name##.at(3))(pThis, unk);			\
	StepFrameTime();																			\
	auto player = *g_thePlayer;																	\
	auto camera = CorrectedPlayerCamera::GetSingleton();										\
	std::shared_ptr<Camera::SmoothCamera> lockedPtr;											\
	if ((camera && player); lockedPtr = g_theCamera.lock(), lockedPtr != nullptr)				\
		lockedPtr->UpdateCamera(player, camera);												\
}

#define DO_CAMERA_UPDATE_DETOUR_IMPL(name, state)				\
auto detour_##name## = Detours::CameraStateDetour(				\
	CorrectedPlayerCamera::GetSingleton()->cameraStates[state],	\
	static_cast<uint16_t>(3),									\
	reinterpret_cast<uint64_t>(&mCameraStateUpdate##name##),	\
	origVFuncs_##name##											\
);

CAMERA_UPDATE_DETOUR_IMPL(FPS);
CAMERA_UPDATE_DETOUR_IMPL(TPS);
CAMERA_UPDATE_DETOUR_IMPL(Dragon);
CAMERA_UPDATE_DETOUR_IMPL(Horse);
CAMERA_UPDATE_DETOUR_IMPL(Tween);
CAMERA_UPDATE_DETOUR_IMPL(VATS);
CAMERA_UPDATE_DETOUR_IMPL(Free);
CAMERA_UPDATE_DETOUR_IMPL(Vanity);
CAMERA_UPDATE_DETOUR_IMPL(Furniture);
CAMERA_UPDATE_DETOUR_IMPL(Bleedout);
CAMERA_UPDATE_DETOUR_IMPL(Transition);

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
	if (pThis == (uintptr_t)&(*g_thePlayer)->menuOpenCloseEvent) {
		if (ev->menuName != nullptr && strcmp(ev->menuName, "Dialogue Menu") == 0) {
			std::shared_ptr<Camera::SmoothCamera> lockedPtr;
			if (!g_theCamera.expired() && (lockedPtr = g_theCamera.lock(), lockedPtr != nullptr)) {
				lockedPtr->OnDialogMenuChanged(ev);
			}
		}
	}
	return reinterpret_cast<MenuOpenCloseHandler>(origVFuncs_MenuOpenClose[1])(pThis, ev, dispatcher);
}

bool Detours::Attach(std::shared_ptr<Camera::SmoothCamera> theCamera) {
	g_theCamera = theCamera;

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
		// Intercept menu open/close events
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

	DO_CAMERA_UPDATE_DETOUR_IMPL(FPS, CorrectedPlayerCamera::kCameraState_FirstPerson);
	DO_CAMERA_UPDATE_DETOUR_IMPL(TPS, CorrectedPlayerCamera::kCameraState_ThirdPerson2);
	DO_CAMERA_UPDATE_DETOUR_IMPL(Dragon, CorrectedPlayerCamera::kCameraState_Dragon);
	DO_CAMERA_UPDATE_DETOUR_IMPL(Horse, CorrectedPlayerCamera::kCameraState_Horse);
	DO_CAMERA_UPDATE_DETOUR_IMPL(Tween, CorrectedPlayerCamera::kCameraState_TweenMenu);
	DO_CAMERA_UPDATE_DETOUR_IMPL(VATS, CorrectedPlayerCamera::kCameraState_VATS);
	DO_CAMERA_UPDATE_DETOUR_IMPL(Free, CorrectedPlayerCamera::kCameraState_Free);
	DO_CAMERA_UPDATE_DETOUR_IMPL(Vanity, CorrectedPlayerCamera::kCameraState_AutoVanity);
	DO_CAMERA_UPDATE_DETOUR_IMPL(Furniture, CorrectedPlayerCamera::kCameraState_Furniture);
	DO_CAMERA_UPDATE_DETOUR_IMPL(Bleedout, CorrectedPlayerCamera::kCameraState_Bleedout);
	DO_CAMERA_UPDATE_DETOUR_IMPL(Transition, CorrectedPlayerCamera::kCameraState_Transition);

	return ArrowFixes::Attach();
}