#include "hooks.h"
#include "camera.h"
#include "thirdperson.h"
#include "crosshair.h"
#include "arrow_fixes.h"
#include "debug/eh.h"

extern eastl::unique_ptr<Camera::Camera> g_theCamera;
extern Offsets* g_Offsets;

using PlayerMenuOpenCloseEvent = void; // RE::BSTEventSink<RE::MenuOpenCloseEvent>
using PlayerMenuModeChangeEvent = void; // RE::BSTEventSink<MenuModeChangeEvent>
static eastl::unique_ptr<PolymorphicVTableDetour<RE::TESCameraState, 13>> cameraUpdateHooks;
static eastl::unique_ptr<VTableDetour<RE::PlayerInputHandler>> playerInputHook;
static eastl::unique_ptr<VTableDetour<PlayerMenuOpenCloseEvent>> menuOpenCloseHook;
static eastl::unique_ptr<VTableDetour<PlayerMenuModeChangeEvent>> menuModeChangeHook;
static eastl::unique_ptr<VTableDetour<RE::TESCameraState>> inputHandler_ThirdPersonState;
static eastl::unique_ptr<VTableDetour<RE::TESCameraState>> inputHandler_DragonState;
static eastl::unique_ptr<VTableDetour<RE::TESCameraState>> inputHandler_HorseState;
static eastl::unique_ptr<VTableDetour<RE::TESCameraState>> inputHandler_BleedoutState;

// Camera update
typedef void(__thiscall* CameraOnUpdate)(RE::TESCameraState*, RE::BSTSmartPointer<RE::TESCameraState>&);
static void mCameraUpdate(RE::TESCameraState* state, RE::BSTSmartPointer<RE::TESCameraState>& nextState) {
	auto ply = RE::PlayerCharacter::GetSingleton();
	auto cam = RE::PlayerCamera::GetSingleton();

	{
		const auto mdmp = Debug::MiniDumpScope();
		GameTime::StepFrameTime();

		if (g_theCamera)
			if (g_theCamera->PreGameUpdate(ply, cam, nextState))
				return;
	}

	// TPS1&2 share the same vtable
	RE::TESCameraState* selector;
	if (state->id == RE::CameraState::kAnimated)
		selector = cam->cameraStates[RE::CameraState::kThirdPerson].get();
	else
		selector = state;

	// Make sure we have a valid selector
	if (!selector) return;

	// And a valid base
	const auto base = cameraUpdateHooks->GetBase<CameraOnUpdate>(selector, 3);
	if (!base) return;

	cameraUpdateHooks->GetBase<CameraOnUpdate>(selector, 3)(state, nextState);

	{
		const auto mdmp = Debug::MiniDumpScope();
		if (g_theCamera)
			g_theCamera->UpdateCamera(ply, cam, nextState);
	}
}

// Camera look input
typedef void(__thiscall* CameraHandleLookInput)(RE::TESCameraState*, const RE::NiPoint2&);
static void mCameraHandleLookInput(RE::TESCameraState* state, const RE::NiPoint2& input) {
	if (!Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken() && !Config::GetCurrentConfig()->modDisabled)
	{
		const auto mdmp = Debug::MiniDumpScope();
		if (g_theCamera && g_theCamera->IsInputLocked(state)) return;
	}

	// TPS1&2 share the same vtable
	RE::TESCameraState* selector;
	if (state->id == RE::CameraState::kAnimated)
		selector = RE::PlayerCamera::GetSingleton()->cameraStates[RE::CameraState::kThirdPerson].get();
	else
		selector = state;
	cameraUpdateHooks->GetBase<CameraHandleLookInput>(selector, 0xF)(selector, input);
}

// POV Handler
typedef void(__thiscall* ProcessButton)(RE::TESCameraState* state, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_movementData);
static void mProcessButtonTPS(RE::TESCameraState* state, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_movementData) {
	{
		const auto mdmp = Debug::MiniDumpScope();
		const auto id = a_event->QUserEvent();
		if (id.size() > 0)
			if (strcmp(id.c_str(), "Toggle POV") == 0)
				if (g_theCamera)
					g_theCamera->OnTogglePOV(a_event);
	}

	inputHandler_ThirdPersonState->GetBase<ProcessButton>(4)(state, a_event, a_movementData);
}

static void mProcessButtonDragon(RE::TESCameraState* state, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_movementData) {
	{
		const auto mdmp = Debug::MiniDumpScope();
		const auto id = a_event->QUserEvent();
		if (id.size() > 0)
			if (strcmp(id.c_str(), "Toggle POV") == 0)
				if (g_theCamera)
					g_theCamera->OnTogglePOV(a_event);
	}

	inputHandler_DragonState->GetBase<ProcessButton>(4)(state, a_event, a_movementData);
}

static void mProcessButtonHorse(RE::TESCameraState* state, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_movementData) {
	{
		const auto mdmp = Debug::MiniDumpScope();
		const auto id = a_event->QUserEvent();
		if (id.size() > 0)
			if (strcmp(id.c_str(), "Toggle POV") == 0)
				if (g_theCamera)
					g_theCamera->OnTogglePOV(a_event);
	}

	inputHandler_HorseState->GetBase<ProcessButton>(4)(state, a_event, a_movementData);
}

static void mProcessButtonBleedout(RE::TESCameraState* state, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_movementData) {
	{
		const auto mdmp = Debug::MiniDumpScope();
		const auto id = a_event->QUserEvent();
		if (id.size() > 0)
			if (strcmp(id.c_str(), "Toggle POV") == 0)
				if (g_theCamera)
					g_theCamera->OnTogglePOV(a_event);
	}

	inputHandler_BleedoutState->GetBase<ProcessButton>(4)(state, a_event, a_movementData);
}

// Key pressed
typedef uintptr_t(__thiscall* OnInput)(RE::PlayerInputHandler*, RE::InputEvent*);
static uintptr_t mOnInput(RE::PlayerInputHandler* pThis, RE::InputEvent* input) {
	if (input) {
		const auto mdmp = Debug::MiniDumpScope();

		switch (input->eventType.get()) {
			case RE::INPUT_EVENT_TYPE::kButton: {
				if (g_theCamera)
					g_theCamera->OnKeyPress(reinterpret_cast<RE::ButtonEvent*>(input));
				break;
			}
			default:
				break;
		}
	}
	return playerInputHook->GetBase<OnInput>(1)(pThis, input);
}

// Menu open/close
typedef RE::BSEventNotifyControl(__thiscall* MenuOpenCloseHandler)(uintptr_t pThis, RE::MenuOpenCloseEvent* ev,
	RE::BSTEventSource<RE::MenuOpenCloseEvent>* dispatcher);
static RE::BSEventNotifyControl mMenuOpenCloseHandler(uintptr_t pThis, RE::MenuOpenCloseEvent* ev,
	RE::BSTEventSource<RE::MenuOpenCloseEvent>* dispatcher)
{
	const auto menuOpenCloseHandler = reinterpret_cast<uintptr_t>(RE::PlayerCharacter::GetSingleton()) + 0x2B0;
	if (pThis == menuOpenCloseHandler) {
		const auto mdmp = Debug::MiniDumpScope();

		if (ev->menuName.size() > 0 && g_theCamera) {
			DebugPrint("Menu %s is %s\n", ev->menuName.c_str(), ev->opening ? "opening" : "closing");
			auto id = Camera::MenuID::None;

			if (strcmp(ev->menuName.c_str(), "Dialogue Menu") == 0)
				id = Camera::MenuID::DialogMenu;
			else if (strcmp(ev->menuName.c_str(), "Loading Menu") == 0)
				id = Camera::MenuID::LoadingMenu;
			else if (strcmp(ev->menuName.c_str(), "Mist Menu") == 0)
				id = Camera::MenuID::MistMenu;
			else if (strcmp(ev->menuName.c_str(), "Fader Menu") == 0)
				id = Camera::MenuID::FaderMenu;
			else if (strcmp(ev->menuName.c_str(), "LoadWaitSpinner") == 0)
				id = Camera::MenuID::LoadWaitSpinner;
			else if (strcmp(ev->menuName.c_str(), "MapMenu") == 0)
				id = Camera::MenuID::MapMenu;
			else if (strcmp(ev->menuName.c_str(), "InventoryMenu") == 0)
				id = Camera::MenuID::InventoryMenu;

			if (id != Camera::MenuID::None)
				g_theCamera->OnMenuOpenClose(id, ev);
		}
	}
	return menuOpenCloseHook->GetBase<MenuOpenCloseHandler>(1)(pThis, ev, dispatcher);
}

// MenuMode changed
struct MenuModeChangeEvent { RE::BSFixedString name; };
typedef RE::BSEventNotifyControl(__thiscall* MenuModeChangeHandler)(uintptr_t pThis, MenuModeChangeEvent* ev,
	RE::BSTEventSource<MenuModeChangeEvent>* dispatcher);
static RE::BSEventNotifyControl mMenuModeChangeHandler(uintptr_t pThis, MenuModeChangeEvent* ev,
	RE::BSTEventSource<MenuModeChangeEvent>* dispatcher)
{
	const auto menuModeChangeHandler = reinterpret_cast<uintptr_t>(RE::PlayerCharacter::GetSingleton()) + 0x2B8;
	if (pThis == menuModeChangeHandler) {
		const auto mdmp = Debug::MiniDumpScope();
		if (!ev->name.empty())
			g_theCamera->OnMenuModeChange(RE::UI::GetSingleton()->IsMenuOpen(ev->name));
	}
	return menuModeChangeHook->GetBase<MenuModeChangeHandler>(1)(pThis, ev, dispatcher);
}


// 80230 : gotoAndStop
typedef uintptr_t(*CrosshairInvoke)(RE::GFxMovieView** param1, uint64_t* param2, const char* name, uint64_t param4);
static eastl::unique_ptr<TypedDetour<CrosshairInvoke>> detCrosshairInvoke;
static uintptr_t mCrosshairInvoke(RE::GFxMovieView** param1, uint64_t* param2, const char* name, uint64_t param4) {
	const auto ret = detCrosshairInvoke->GetBase()(param1, param2, name, param4);
	if (!name || !g_theCamera) return ret;

	{
		const auto mdmp = Debug::MiniDumpScope();
		if (strcmp(name, "Alert") == 0) {
			g_theCamera->GetThirdpersonCamera()->GetCrosshairManager()->SetAlertMode(true);
		} else if (strcmp(name, "Normal") == 0) {
			g_theCamera->GetThirdpersonCamera()->GetCrosshairManager()->SetAlertMode(false);
		}
	}

	return ret;
}

//{ 0x00ECA860, 80233 }
typedef bool(*GFxInvoke)(void* pThis, void* obj, RE::GFxValue* result, const char* name, RE::GFxValue* args, uint32_t numArgs, bool isDisplayObj);
static eastl::unique_ptr<TypedDetour<GFxInvoke>> detGFxInvoke;
static bool mGFxInvoke(void* pThis, void* obj, RE::GFxValue* result, const char* name, RE::GFxValue* args, uint32_t numArgs, bool isDisplayObj) {
	const auto ret = detGFxInvoke->GetBase()(pThis, obj, result, name, args, numArgs, isDisplayObj);

	{
		const auto mdmp = Debug::MiniDumpScope();
		const auto camera = RE::PlayerCamera::GetSingleton();
		if (GameState::IsFirstPerson(camera))
			return ret;

		if (g_theCamera && !Messaging::SmoothCamInterface::GetInstance()->IsCrosshairTaken()
			&& name && strcmp(name, "ValidateCrosshair") == 0)
		{
			// Getting spammed on by conjuration magic mode - (ノಠ益ಠ)ノ彡┻━┻
			// @Note: I really don't like these more invasive detours - finding a way around this should be a priority
			// We need this currently because ValidateCrosshair will desync our crosshair state and mess things up
			g_theCamera->GetThirdpersonCamera()->GetCrosshairManager()->ValidateCrosshair();
		}
	}

	return ret;
}

//FUN_14084b430:49866
typedef void(*FactorCameraOffset)(RE::PlayerCamera* camera, RE::NiPoint3& pos, bool fac);
using FactorCameraOffsetDetour = TypedDetour<FactorCameraOffset>;
static eastl::unique_ptr<FactorCameraOffsetDetour> detFactorCameraOffset;
static void mFactorCameraOffset(RE::PlayerCamera* camera, RE::NiPoint3& pos, bool fac) {
	// SSE Engine Fixes will call GetEyeVector with factorCameraOffset
	// We appear to screw this computation up as a side effect of correcting the interaction crosshair
	// So, yeah. Just fix it here.
	if (fac || Config::GetCurrentConfig()->modDisabled) {
		detFactorCameraOffset->GetBase()(camera, pos, fac);
		return;
	}

	// Only run in states we care about
	{
		//const auto mdmp = Debug::MiniDumpScope();
		if ((Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken() || Config::GetCurrentConfig()->modDisabled) ||
			(!GameState::IsThirdPerson(camera) && !GameState::IsInHorseCamera(camera) && !GameState::IsInDragonCamera(camera)))
		{
			detFactorCameraOffset->GetBase()(camera, pos, fac);
			return;
		}

		// Just return an offset of zero
		pos = { 0, 0, 0 };
	}
}

// Render tear-down, before CoUninitialize
static eastl::vector<Hooks::ShutdownCallback> shutdownEVs;
void Hooks::RegisterGameShutdownEvent(ShutdownCallback&& cb) noexcept {
	shutdownEVs.push_back(eastl::move(cb));
}

typedef uintptr_t(*CalledDuringRenderShutdown)();
static eastl::unique_ptr<TypedDetour<CalledDuringRenderShutdown>> detCalledDuringRenderShutdown;
static uintptr_t mCalledDuringRenderShutdown() {
	DebugPrint("Shutting down...\n");

	// Process shutdown callbacks first
	{
		const auto scope = Debug::MiniDumpScope();
		for (const auto& ev : shutdownEVs)
			ev();
	}

#ifdef EMIT_MINIDUMPS
	DebugPrint("Removing minidump handler\n");
	Debug::RemoveMiniDumpHandler();
#endif

#ifdef DEVELOPER
	if (TrackIR::IsRunning()) {
		DebugPrint("Shutting down TrackIR\n");
		TrackIR::Shutdown();
	}
#endif

	DebugPrint("Shutting down the rendering subsystem\n");
	Render::Shutdown();

	DebugPrint("SmoothCam shutdown, continue with game shutdown...\n");
	return detCalledDuringRenderShutdown->GetBase()();
}

// Render startup
// Hook here after D3DStartup so we don't step on SSEDisplayTweaks :>
typedef void(*CalledDuringRenderStartup)();
static eastl::unique_ptr<TypedDetour<CalledDuringRenderStartup>> detCalledDuringRenderStartup;
static void mCalledDuringRenderStartup() {
	if (Render::HasAttemptedHook()) return;

	logger::info("Hooking D3D11\n");
	Render::InstallHooks();
	if (!Render::HasContext())
		WarningPopup(L"SmoothCam: Failed to hook DirectX, Rendering features will be disabled. Try running with overlay software disabled if this warning keeps occurring.");

#ifdef DEBUG
	if (Render::HasContext())
		Util::InitializeDebugDrawing(Render::GetContext());
#endif

#ifdef WITH_D2D
	if (Render::HasContext()) {
		const auto mdmp = Debug::MiniDumpScope();
		Render::InitD2D();
	}
#endif

#ifdef DEVELOPER
	if (Render::HasContext()) {
		const auto mdmp = Debug::MiniDumpScope();
		const auto result = TrackIR::Initialize(Render::GetContext().hWnd);
		if (result != TrackIR::NPResult::OK)
			logger::info("Failed to load TrackIR interface");
		else
			logger::info("TrackIR is running.");
	}
#endif

	detCalledDuringRenderStartup->GetBase()();
}

bool Hooks::DeferredAttach() {
	playerInputHook = eastl::make_unique<VTableDetour<RE::PlayerInputHandler>>(RE::PlayerControls::GetSingleton()->togglePOVHandler);
	playerInputHook->Add(1, mOnInput);
	if (!playerInputHook->Attach())
		FatalError(L"Failed to place detour on target virtual function(togglePOVHandler), this error is fatal.");

	menuOpenCloseHook = eastl::make_unique<VTableDetour<PlayerMenuOpenCloseEvent>>(
		reinterpret_cast<PlayerMenuOpenCloseEvent*>(
			reinterpret_cast<uintptr_t>(RE::PlayerCharacter::GetSingleton()) + 0x2B8
		)
	);
	menuOpenCloseHook->Add(1, mMenuOpenCloseHandler);
	if (!menuOpenCloseHook->Attach())
		FatalError(L"Failed to place detour on target virtual function(menuOpenCloseHandler), this error is fatal.");

	menuModeChangeHook = eastl::make_unique<VTableDetour<PlayerMenuModeChangeEvent>>(
		reinterpret_cast<PlayerMenuModeChangeEvent*>(
			reinterpret_cast<uintptr_t>(RE::PlayerCharacter::GetSingleton()) + 0x2C0
		)
	);
	menuModeChangeHook->Add(1, mMenuModeChangeHandler);
	if (!menuModeChangeHook->Attach())
		FatalError(L"Failed to place detour on target virtual function(menuModeChangeHandler), this error is fatal.");

	auto states = RE::PlayerCamera::GetSingleton()->cameraStates;
	cameraUpdateHooks = eastl::make_unique<PolymorphicVTableDetour<RE::TESCameraState, 13>>();
	cameraUpdateHooks->Add(states[RE::CameraState::kFirstPerson].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kThirdPerson].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kDragon].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kMount].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kTween].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kVATS].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kFree].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kAutoVanity].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kFurniture].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kBleedout].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kPCTransition].get(), 3, mCameraUpdate);
	cameraUpdateHooks->Add(states[RE::CameraState::kIronSights].get(), 3, mCameraUpdate);

	cameraUpdateHooks->Add(states[RE::CameraState::kThirdPerson].get(), 0xF, mCameraHandleLookInput);
	cameraUpdateHooks->Add(states[RE::CameraState::kDragon].get(), 0xF, mCameraHandleLookInput);
	cameraUpdateHooks->Add(states[RE::CameraState::kMount].get(), 0xF, mCameraHandleLookInput);
	cameraUpdateHooks->Add(states[RE::CameraState::kBleedout].get(), 0xF, mCameraHandleLookInput);

	DebugPrint("Hooking camera state update methods\n");
	if (!cameraUpdateHooks->Attach())
		FatalError(L"Failed to place detour on target virtual function(TESCameraState::Update), this error is fatal.");

	DebugPrint("Hooking camera state input methods\n");
	auto tps_1 = REL::Relocation<RE::TESCameraState*>(g_Offsets->vtable_ThirdPersonState_1).get();
	auto drag_1 = REL::Relocation<RE::TESCameraState*>(g_Offsets->vtable_DragonCameraState_1).get();
	auto horse_1 = REL::Relocation<RE::TESCameraState*>(g_Offsets->vtable_HorseCameraState_1).get();
	auto bleed_1 = REL::Relocation<RE::TESCameraState*>(g_Offsets->vtable_BleedoutCameraState_1).get();
	inputHandler_ThirdPersonState = eastl::make_unique<VTableDetour<RE::TESCameraState>>(
		reinterpret_cast<RE::TESCameraState*>(&tps_1)
	);
	inputHandler_ThirdPersonState->Add(4, mProcessButtonTPS);

	inputHandler_DragonState = eastl::make_unique<VTableDetour<RE::TESCameraState>>(
		reinterpret_cast<RE::TESCameraState*>(&drag_1)
	);
	inputHandler_DragonState->Add(4, mProcessButtonDragon);

	inputHandler_HorseState = eastl::make_unique<VTableDetour<RE::TESCameraState>>(
		reinterpret_cast<RE::TESCameraState*>(&horse_1)
	);
	inputHandler_HorseState->Add(4, mProcessButtonHorse);

	inputHandler_BleedoutState = eastl::make_unique<VTableDetour<RE::TESCameraState>>(
		reinterpret_cast<RE::TESCameraState*>(&bleed_1)
	);
	inputHandler_BleedoutState->Add(4, mProcessButtonBleedout);

	if (!inputHandler_ThirdPersonState->Attach() || !inputHandler_DragonState->Attach() ||
		!inputHandler_HorseState->Attach() || !inputHandler_BleedoutState->Attach())
		FatalError(L"Failed to place detour on target functions(TESCameraState::ProcessButton), this error is fatal.");

	// FactorCameraOffset
	detFactorCameraOffset = eastl::make_unique<FactorCameraOffsetDetour>(g_Offsets->FactorCameraOffset, mFactorCameraOffset);
	if (!detFactorCameraOffset->Attach())
		FatalError(L"Failed to place detour on target function(Hooks::FactorCameraOffset), this error is fatal.");

	// Crosshair UI
	detGFxInvoke = eastl::make_unique<TypedDetour<GFxInvoke>>(g_Offsets->GFxInvoke, mGFxInvoke);
	if (!detGFxInvoke->Attach())
		FatalError(L"Failed to place detour on target function(80,233), this error is fatal.");

	detCrosshairInvoke = eastl::make_unique<TypedDetour<CrosshairInvoke>>(g_Offsets->GFxGotoAndStop, mCrosshairInvoke);
	if (!detCrosshairInvoke->Attach())
		FatalError(L"Failed to place detour on target function(80,230), this error is fatal.");

	return ArrowFixes::Attach();
}

bool Hooks::AttachD3D() {
	logger::info("Hooking render startup method");
	detCalledDuringRenderStartup = eastl::make_unique<TypedDetour<CalledDuringRenderStartup>>(
		g_Offsets->RenderStartup,
		mCalledDuringRenderStartup
	);
	if (!detCalledDuringRenderStartup->Attach()) {
		logger::critical("Failed to place detour on target function(Render Startup), this error is fatal.");
		return false;
	}

	logger::info("Hooking render shutdown method");
	detCalledDuringRenderShutdown = eastl::make_unique<TypedDetour<CalledDuringRenderShutdown>>(
		g_Offsets->RenderShutdown,
		mCalledDuringRenderShutdown
	);
	if (!detCalledDuringRenderShutdown->Attach()) {
		logger::critical("Failed to place detour on target function(Render Shutdown), this error is fatal.");
		return false;
	}

	return true;
}

#ifdef SKYRIM_SUPPORT_AE
void Hooks::ApplyPatches() {
	SKSE::AllocTrampoline(512);

	auto hundered = 100.0f;
	auto hunderedK = 100000.0f;
	auto one = 1.0f;

	static auto initBestDistance = *reinterpret_cast<uint32_t*>(&hundered);
	static auto bestDistance = *reinterpret_cast<uint32_t*>(&hundered);
	static auto badDistance = *reinterpret_cast<uint32_t*>(&hunderedK);
	static auto okDistance = *reinterpret_cast<uint32_t*>(&one);

	struct CrosshairPickerTerrain_Patch0 : public Xbyak::CodeGenerator {	
		CrosshairPickerTerrain_Patch0(uintptr_t target) {
			Xbyak::Label returnL;
			Xbyak::Label initBestL;
			Xbyak::Label bestDistanceL;

			mov(rax, ptr[rip + bestDistanceL]);
			movss(xmm0, ptr[rip + initBestL]);
			movss(ptr[rax], xmm0);
			mov(rax, rsp);
			mov(ptr[rax + 0x20], r9);
			jmp(ptr[rip + returnL]);
			L(returnL);
			dq(target);

			L(initBestL);
			dd(initBestDistance);
			L(bestDistanceL);
			dq(reinterpret_cast<uintptr_t>(&bestDistance));
		}
	};

	struct CrosshairPickerTerrain_Patch1 : public Xbyak::CodeGenerator {
		CrosshairPickerTerrain_Patch1(uintptr_t target) {
			Xbyak::Label returnL;
			Xbyak::Label compEnd;
			Xbyak::Label bestDistanceL;

			mov(r15, ptr[rip + bestDistanceL]);
			add(rax, ptr[rbp + 0x5B0]);
			movss(xmm0, ptr[rax + 0x1C]);
			comiss(xmm0, ptr[r15]);
			ja(compEnd);
			movss(ptr[r15], xmm0);
			L(compEnd);
			jmp(ptr[rip + returnL]);

			L(returnL);
			dq(target);
			L(bestDistanceL);
			dq(reinterpret_cast<uintptr_t>(&bestDistance));
		}
	};

	struct CrosshairPickerTerrain_Patch2 : public Xbyak::CodeGenerator {
		CrosshairPickerTerrain_Patch2(uintptr_t target) {
			Xbyak::Label returnL;
			Xbyak::Label compEnd2;
			Xbyak::Label bestDistanceL;
			Xbyak::Label badDistanceL;
			Xbyak::Label okDistanceL;

			mov(rax, ptr[rip + bestDistanceL]);
			comiss(xmm10, ptr[rip + badDistanceL]);
			jb(compEnd2);
			movss(xmm1, ptr[rax]);
			comiss(xmm1, ptr[rip + okDistanceL]);
			ja(compEnd2);
			movss(xmm10, ptr[rax]);
			mulss(xmm10, xmm15);
			L(compEnd2);
			movaps(xmm1, xmm10);
			mov(rax, ptr[rbp + 0x8C8]);
			jmp(ptr[rip + returnL]);

			L(returnL);
			dq(target);
			L(bestDistanceL);
			dq(reinterpret_cast<uintptr_t>(&bestDistance));
			L(badDistanceL);
			dd(badDistance);
			L(okDistanceL);
			dd(okDistance);
		}
	};


	auto targetFunc = g_Offsets->CrosshairData_pick;
	const auto returnTarget0 = targetFunc + 7;
	const auto returnTarget1 = targetFunc + 0x3B4 + 7;
	const auto returnTarget2 = targetFunc + 0xEAC + 11;
	CrosshairPickerTerrain_Patch0 code0(returnTarget0); code0.ready();
	CrosshairPickerTerrain_Patch1 code1(returnTarget1); code1.ready();
	CrosshairPickerTerrain_Patch2 code2(returnTarget2); code2.ready();

	auto& trampoline = SKSE::GetTrampoline();
	trampoline.write_branch<6>(targetFunc, trampoline.allocate(code0));
	trampoline.write_branch<6>(targetFunc + 0x3B4, trampoline.allocate(code1));
	trampoline.write_branch<6>(targetFunc + 0xEAC, trampoline.allocate(code2));
}
#else
void Hooks::ApplyPatches() {
	SKSE::AllocTrampoline(512);

	auto hundered = 100.0f;
	auto hunderedK = 100000.0f;
	auto one = 1.0f;

	static auto initBestDistance = *reinterpret_cast<uint32_t*>(&hundered);
	static auto bestDistance = *reinterpret_cast<uint32_t*>(&hundered);
	static auto badDistance = *reinterpret_cast<uint32_t*>(&hunderedK);
	static auto okDistance = *reinterpret_cast<uint32_t*>(&one);

	struct CrosshairPickerTerrain_Patch0 : public Xbyak::CodeGenerator {	
		CrosshairPickerTerrain_Patch0(uintptr_t target) {
			Xbyak::Label returnL;
			Xbyak::Label initBestL;
			Xbyak::Label bestDistanceL;

			mov(rax, ptr[rip + bestDistanceL]);
			movss(xmm0, ptr[rip + initBestL]);
			movss(ptr[rax], xmm0);
			mov(rax, rsp);
			mov(ptr[rax + 0x20], r9);
			jmp(ptr[rip + returnL]);
			L(returnL);
			dq(target);

			L(initBestL);
			dd(initBestDistance);
			L(bestDistanceL);
			dq(reinterpret_cast<uintptr_t>(&bestDistance));
		}
	};

	struct CrosshairPickerTerrain_Patch1 : public Xbyak::CodeGenerator {
		CrosshairPickerTerrain_Patch1(uintptr_t target) {
			Xbyak::Label returnL;
			Xbyak::Label compEnd;
			Xbyak::Label bestDistanceL;

			mov(r14, ptr[rip + bestDistanceL]);
			add(rax, ptr[rbp + 0x290]);
			movss(xmm0, ptr[rax + 0x1C]);
			comiss(xmm0, ptr[r14]);
			ja(compEnd);
			movss(ptr[r14], xmm0);
			L(compEnd);
			jmp(ptr[rip + returnL]);

			L(returnL);
			dq(target);
			L(bestDistanceL);
			dq(reinterpret_cast<uintptr_t>(&bestDistance));
		}
	};

	struct CrosshairPickerTerrain_Patch2 : public Xbyak::CodeGenerator {
		CrosshairPickerTerrain_Patch2(uintptr_t target) {
			Xbyak::Label returnL;
			Xbyak::Label compEnd2;
			Xbyak::Label bestDistanceL;
			Xbyak::Label badDistanceL;
			Xbyak::Label okDistanceL;
			
			mov(rax, ptr[rip + bestDistanceL]);
			comiss(xmm10, ptr[rip + badDistanceL]);
			jb(compEnd2);
			movss(xmm1, ptr[rax]);
			comiss(xmm1, ptr[rip + okDistanceL]);
			ja(compEnd2);
			movss(xmm10, ptr[rax]);
			mulss(xmm10, xmm14);
			L(compEnd2);
			movaps(xmm1, xmm10);
			mov(rax, ptr[rbp + 0x8C8]);
			jmp(ptr[rip + returnL]);

			L(returnL);
			dq(target);
			L(bestDistanceL);
			dq(reinterpret_cast<uintptr_t>(&bestDistance));
			L(badDistanceL);
			dd(badDistance);
			L(okDistanceL);
			dd(okDistance);
		}
	};


	auto targetFunc = g_Offsets->CrosshairData_pick;
	const auto returnTarget0 = targetFunc + 7;
	const auto returnTarget1 = targetFunc + 0x3B4 + 7;
	const auto returnTarget2 = targetFunc + 0xD37 + 11;
	CrosshairPickerTerrain_Patch0 code0(returnTarget0); code0.ready();
	CrosshairPickerTerrain_Patch1 code1(returnTarget1); code1.ready();
	CrosshairPickerTerrain_Patch2 code2(returnTarget2); code2.ready();
	
	auto& trampoline = SKSE::GetTrampoline();
	trampoline.write_branch<6>(targetFunc, trampoline.allocate(code0));
	trampoline.write_branch<6>(targetFunc + 0x3B4, trampoline.allocate(code1));
	trampoline.write_branch<6>(targetFunc + 0xD37, trampoline.allocate(code2));
}
#endif