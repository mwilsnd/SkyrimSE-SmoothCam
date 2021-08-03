#include "detours.h"
#include "camera.h"
#include "arrow_fixes.h"
#include "debug/eh.h"

extern eastl::unique_ptr<Camera::Camera> g_theCamera;

static eastl::unique_ptr<PolymorphicVTableDetour<TESCameraState, 13>> cameraUpdateHooks;
static eastl::unique_ptr<VTableDetour<PlayerInputHandler>> playerInputHook;
static eastl::unique_ptr<VTableDetour<BSTEventSink<void*>>> menuOpenCloseHook;

// Camera update
typedef void(__thiscall* CameraOnUpdate)(TESCameraState*, BSTSmartPointer<TESCameraState>&);
void mCameraUpdate(TESCameraState* state, BSTSmartPointer<TESCameraState>& nextState) {
	const auto mdmp = Debug::MiniDumpScope();

	GameTime::StepFrameTime();

	auto player = *g_thePlayer;
	player->IncRef();
	auto camera = CorrectedPlayerCamera::GetSingleton();

	if (g_theCamera) {
		if (g_theCamera->PreGameUpdate(player, camera, nextState)) {
			player->DecRef();
			return;
		}
	}

	// TPS1&2 share the same vtable
	TESCameraState* selector;
	if (state == camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson1])
		selector = camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2];
	else
		selector = state;

	cameraUpdateHooks->GetBase<CameraOnUpdate>(selector, 3)(state, nextState);

	if (g_theCamera)
		g_theCamera->UpdateCamera(player, camera, nextState);

	player->DecRef();
}

// POV Handler
typedef uintptr_t(__thiscall* OnInput)(PlayerInputHandler*, InputEvent*);
uintptr_t mOnInput(PlayerInputHandler* pThis, InputEvent* input) {
	if (input) {
		const auto mdmp = Debug::MiniDumpScope();

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
	return playerInputHook->GetBase<OnInput>(1)(pThis, input);
}

typedef EventResult(__thiscall* MenuOpenCloseHandler)(uintptr_t pThis, MenuOpenCloseEvent* ev,
	EventDispatcher<MenuOpenCloseEvent>* dispatcher);
EventResult mMenuOpenCloseHandler(uintptr_t pThis, MenuOpenCloseEvent* ev,
	EventDispatcher<MenuOpenCloseEvent>* dispatcher)
{
	if (pThis == (uintptr_t)&(*g_thePlayer)->menuOpenCloseEvent) {
		const auto mdmp = Debug::MiniDumpScope();

		if (ev->menuName.data && g_theCamera) {
			DebugPrint("Menu %s is %s\n", ev->menuName.data, ev->opening ? "opening" : "closing");
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
			else if (strcmp(ev->menuName.data, "MapMenu") == 0)
				id = Camera::MenuID::MapMenu;
			else if (strcmp(ev->menuName.data, "InventoryMenu") == 0)
				id = Camera::MenuID::InventoryMenu;

			if (id != Camera::MenuID::None)
				g_theCamera->OnMenuOpenClose(id, ev);
		}
	}
	return menuOpenCloseHook->GetBase<MenuOpenCloseHandler>(1)(pThis, ev, dispatcher);
}

bool Detours::Attach() {
	GameTime::Initialize();

	playerInputHook = eastl::make_unique<VTableDetour<PlayerInputHandler>>(PlayerControls::GetSingleton()->togglePOVHandler);
	playerInputHook->Add(1, mOnInput);
	if (!playerInputHook->Attach()) {
		_ERROR("Failed to place detour on target virtual function(togglePOVHandler), this error is fatal.");
		FatalError(L"Failed to place detour on target virtual function(togglePOVHandler), this error is fatal.");
	}

	menuOpenCloseHook = eastl::make_unique<VTableDetour<BSTEventSink<void*>>>(&(*g_thePlayer)->menuOpenCloseEvent);
	menuOpenCloseHook->Add(1, mMenuOpenCloseHandler);
	if (!menuOpenCloseHook->Attach()) {
		_ERROR("Failed to place detour on target virtual function(togglePOVHandler), this error is fatal.");
		FatalError(L"Failed to place detour on target virtual function(togglePOVHandler), this error is fatal.");
	}

	auto states = CorrectedPlayerCamera::GetSingleton()->cameraStates;
	cameraUpdateHooks = eastl::make_unique<PolymorphicVTableDetour<TESCameraState, 13>>();
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_FirstPerson], 3, mCameraUpdate);
	//cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_ThirdPerson1], 3, mCameraUpdate); // 1 and 2 share the same vtbl
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_ThirdPerson2], 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_Dragon], 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_Horse], 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_TweenMenu], 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_VATS], 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_Free], 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_AutoVanity], 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_Furniture], 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_Bleedout], 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_Transition], 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[CorrectedPlayerCamera::kCameraState_IronSights], 3, mCameraUpdate);

	DebugPrint("Hooking camera state update methods\n");
	if (!cameraUpdateHooks->Attach()) {
		_ERROR("Failed to place detour on target virtual function(TESCameraState Update), this error is fatal.");
		FatalError(L"Failed to place detour on target virtual function(TESCameraState Update), this error is fatal.");
	}

	return ArrowFixes::Attach();
}