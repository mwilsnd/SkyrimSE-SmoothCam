#include "detours.h"
#include "camera.h"
#include "arrow_fixes.h"

static PLH::VFuncMap origVFuncs_PlayerInput;
static PLH::VFuncMap origVFuncs_MenuOpenClose;

static std::unique_ptr<PLH::VFuncSwapHook> playerInputHooks;
static std::unique_ptr<PLH::VFuncSwapHook> menuOpenCloseHooks;

extern std::shared_ptr<Camera::SmoothCamera> g_theCamera;

#define CAMERA_UPDATE_DETOUR_IMPL(name)															\
static PLH::VFuncMap origVFuncs_##name##;														\
void __fastcall mCameraStateUpdate##name##(														\
	TESCameraState* pThis, BSTSmartPointer<TESCameraState>& nextState)							\
{																								\
	GameTime::StepFrameTime();																	\
	auto player = *g_thePlayer;																	\
	auto camera = CorrectedPlayerCamera::GetSingleton();										\
	player->IncRef();																			\
	if (g_theCamera != nullptr)	{																\
		if (g_theCamera->PreGameUpdate(player, camera, nextState)) {							\
			player->DecRef();																	\
			return;																				\
		}																						\
	}																							\
	reinterpret_cast<Detours::CameraOnUpdate>(origVFuncs_##name##.at(3))(pThis, nextState);		\
	if (g_theCamera != nullptr)	{																\
		g_theCamera->UpdateCamera(player, camera, nextState);									\
	}																							\
	player->DecRef();																			\
}

#define DO_CAMERA_UPDATE_DETOUR_IMPL(name, state)				\
static auto detour_##name## = Detours::CameraStateDetour(		\
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
		switch (input->eventType) {
			case InputEvent::kEventType_Button: {
				const auto ev = reinterpret_cast<ButtonEvent*>(input);
				const BSFixedString* const id = ev->GetControlID();
				if (id && id->data) {
					if (strcmp(id->data, "Toggle POV") == 0) {
						if (g_theCamera)
							g_theCamera->OnTogglePOV(ev);
						break;
					}
				}

				if (g_theCamera)
					g_theCamera->OnKeyPress(ev);
				
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
		if (ev->menuName.data && g_theCamera) {
			auto id = Camera::MenuID::None;

			if (strcmp(ev->menuName.data, "Dialogue Menu") == 0)
				id = Camera::MenuID::DialogMenu;
			else if (strcmp(ev->menuName.data, "Loading Menu") == 0)
				id = Camera::MenuID::LoadingMenu;
			else if (strcmp(ev->menuName.data, "Mist Menu") == 0)
				id = Camera::MenuID::MistMenu;
			else if (strcmp(ev->menuName.data, "Fader Menu") == 0)
				id = Camera::MenuID::FaderMenu;
			else if (strcmp(ev->menuName.data, "LoadWaitSpinner") == 0)
				id = Camera::MenuID::LoadWaitSpinner;

			if (id != Camera::MenuID::None)
				g_theCamera->OnMenuOpenClose(id, ev);
		}
	}
	return reinterpret_cast<MenuOpenCloseHandler>(origVFuncs_MenuOpenClose[1])(pThis, ev, dispatcher);
}

bool Detours::Attach() {
	GameTime::Initialize();

	{
		playerInputHooks = std::make_unique<PLH::VFuncSwapHook>(
			(uint64_t)PlayerControls::GetSingleton()->togglePOVHandler,
			PLH::VFuncMap{
				{ static_cast<uint16_t>(1), reinterpret_cast<uint64_t>(&mOnInput) },
			},
			&origVFuncs_PlayerInput
		);

		if (!playerInputHooks->hook()) {
			_ERROR("Failed to place detour on target virtual function(togglePOVHandler), this error is fatal.");
			FatalError(L"Failed to place detour on target virtual function(togglePOVHandler), this error is fatal.");
		}
	}

	{
		// Intercept menu open/close events
		menuOpenCloseHooks = std::make_unique<PLH::VFuncSwapHook>(
			(uint64_t)&(*g_thePlayer)->menuOpenCloseEvent,
			PLH::VFuncMap{
				{ static_cast<uint16_t>(1), reinterpret_cast<uint64_t>(&mMenuOpenCloseHandler) },
			},
			&origVFuncs_MenuOpenClose
		);

		if (!menuOpenCloseHooks->hook()) {
			_ERROR("Failed to place detour on target virtual function(menuOpenCloseEvent), this error is fatal.");
			FatalError(L"Failed to place detour on target virtual function(menuOpenCloseEvent), this error is fatal.");
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