#include "modapi.h"
#include "camera.h"
#include "thirdperson.h"
#include "crosshair.h"
#include "debug/eh.h"

extern eastl::unique_ptr<Camera::Camera> g_theCamera;
extern const SKSE::MessagingInterface* g_messaging;

Messaging::SmoothCamInterface::SmoothCamInterface() noexcept {
	apiTID = GetCurrentThreadId();
}

Messaging::SmoothCamInterface::~SmoothCamInterface() noexcept {}

DWORD Messaging::SmoothCamInterface::GetSmoothCamThreadId() const noexcept {
	return apiTID;
}

Messaging::APIResult Messaging::SmoothCamInterface::RequestCameraControl(SKSE::PluginHandle modHandle) noexcept {
	const auto owner = cameraOwner.load(std::memory_order::memory_order_acquire);
	if (owner != SKSE::kInvalidPluginHandle)
		if (owner == modHandle)
			return APIResult::AlreadyGiven;
		else
			return APIResult::AlreadyTaken;

	if (needsCameraControl) return APIResult::MustKeep;
	auto expected = static_cast<SKSE::PluginHandle>(SKSE::kInvalidPluginHandle);
	if (!cameraOwner.compare_exchange_strong(expected, modHandle, std::memory_order::memory_order_acq_rel))
		return APIResult::AlreadyTaken;

	wantsInterpolatorUpdates = false;
	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamInterface::RequestCrosshairControl(SKSE::PluginHandle modHandle,
	bool restoreDefaults) noexcept
{
	const auto owner = crosshairOwner.load(std::memory_order::memory_order_acquire);
	if (owner != SKSE::kInvalidPluginHandle)
		if (owner == modHandle)
			return APIResult::AlreadyGiven;
		else
			return APIResult::AlreadyTaken;

	if (needsCrosshairControl) return APIResult::MustKeep;
	auto expected = static_cast<SKSE::PluginHandle>(SKSE::kInvalidPluginHandle);
	if (!crosshairOwner.compare_exchange_strong(expected, modHandle, std::memory_order::memory_order_acq_rel))
		return APIResult::AlreadyTaken;

	if (restoreDefaults && crosshairMgr) {
		const auto mdmp = Debug::MiniDumpScope();
		crosshairMgr->ResetCrosshair();
	}

	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamInterface::RequestStealthMeterControl(SKSE::PluginHandle modHandle,
	bool restoreDefaults) noexcept
{
	const auto owner = stealthMeterOwner.load(std::memory_order::memory_order_acquire);
	if (owner != SKSE::kInvalidPluginHandle)
		if (owner == modHandle)
			return APIResult::AlreadyGiven;
		else
			return APIResult::AlreadyTaken;

	if (needsStealthMeterControl) return APIResult::MustKeep;
	auto expected = static_cast<SKSE::PluginHandle>(SKSE::kInvalidPluginHandle);
	if (!stealthMeterOwner.compare_exchange_strong(expected, modHandle, std::memory_order::memory_order_acq_rel))
		return APIResult::AlreadyTaken;

	if (restoreDefaults && crosshairMgr) {
		const auto mdmp = Debug::MiniDumpScope();
		crosshairMgr->ResetStealthMeter();
	}

	return APIResult::OK;
}

SKSE::PluginHandle Messaging::SmoothCamInterface::GetCameraOwner() const noexcept {
	return cameraOwner;
}

SKSE::PluginHandle Messaging::SmoothCamInterface::GetCrosshairOwner() const noexcept {
	return crosshairOwner;
}

SKSE::PluginHandle Messaging::SmoothCamInterface::GetStealthMeterOwner() const noexcept {
	return stealthMeterOwner;
}

Messaging::APIResult Messaging::SmoothCamInterface::ReleaseCameraControl(SKSE::PluginHandle modHandle) noexcept {
	if (cameraOwner != modHandle) return APIResult::NotOwner;
	cameraOwner.store(SKSE::kInvalidPluginHandle, std::memory_order::memory_order_release);
	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamInterface::ReleaseCrosshairControl(SKSE::PluginHandle modHandle) noexcept {
	if (crosshairOwner != modHandle) return APIResult::NotOwner;
	crosshairOwner.store(SKSE::kInvalidPluginHandle, std::memory_order::memory_order_release);
	wantCrosshairReset = true;
	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamInterface::ReleaseStealthMeterControl(SKSE::PluginHandle modHandle) noexcept {
	if (stealthMeterOwner != modHandle) return APIResult::NotOwner;
	stealthMeterOwner.store(SKSE::kInvalidPluginHandle, std::memory_order::memory_order_release);
	wantStealthMeterReset = true;
	return APIResult::OK;
}

RE::NiPoint3 Messaging::SmoothCamInterface::GetLastCameraPosition() const noexcept {
	const auto mdmp = Debug::MiniDumpScope();
	if (g_theCamera)
		return g_theCamera->GetThirdpersonCamera()->GetPosition().ToNiPoint3();
	return {};
}

Messaging::APIResult Messaging::SmoothCamInterface::RequestInterpolatorUpdates(SKSE::PluginHandle modHandle, bool allowUpdates) noexcept {
	if (cameraOwner != modHandle) return APIResult::NotOwner;
	wantsInterpolatorUpdates = allowUpdates;
	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamInterface::SendToGoalPosition(SKSE::PluginHandle modHandle, bool shouldMoveToGoal,
	bool moveNow, const RE::Actor* ref) noexcept
{
	if (cameraOwner != modHandle) return APIResult::NotOwner;
	if (moveNow) {
		const auto mdmp = Debug::MiniDumpScope();
		g_theCamera->GetThirdpersonCamera()->MoveToGoalPosition(
			RE::PlayerCharacter::GetSingleton(), ref ? ref : RE::PlayerCharacter::GetSingleton(),
			RE::PlayerCamera::GetSingleton()
		);
	} else {
		wantsMoveToGoal = shouldMoveToGoal;
	}
	return APIResult::OK;
}

void Messaging::SmoothCamInterface::GetGoalPosition(RE::TESObjectREFR* ref, RE::NiPoint3& world, RE::NiPoint3& local) const noexcept {
	if (!ref || !g_theCamera) {
		world = {};
		local = {};
		return;
	}

	const auto mdmp = Debug::MiniDumpScope();
	glm::vec3 vworld{};
	glm::vec3 vlocal{};
	g_theCamera->GetThirdpersonCamera()->GetCameraGoalPosition(vworld, vlocal, ref);
	world = { vworld.x, vworld.y, vworld.z };
	local = { vlocal.x, vlocal.y, vlocal.z };
}

bool Messaging::SmoothCamInterface::IsCameraEnabled() const noexcept {
	return !Config::GetCurrentConfig()->modDisabled;
}

void Messaging::SmoothCamInterface::EnableUnlockedHorseAim(bool enable) noexcept {
	unlockedHorseAim = enable;
}

void Messaging::SmoothCamInterface::SetCrosshairManager(Crosshair::Manager* mgr) noexcept {
	crosshairMgr = mgr;
}

void Messaging::SmoothCamInterface::SetNeedsCameraControl(bool needsControl) noexcept {
	needsCameraControl = needsControl;
}

void Messaging::SmoothCamInterface::SetNeedsCrosshairControl(bool needsControl) noexcept {
	needsCrosshairControl = needsControl;
}

void Messaging::SmoothCamInterface::SetNeedsStealthMeterControl(bool needsControl) noexcept {
	needsStealthMeterControl = needsControl;
}

bool Messaging::SmoothCamInterface::IsCameraTaken() const noexcept {
	return cameraOwner.load(std::memory_order::memory_order_acquire) != SKSE::kInvalidPluginHandle;
}

bool Messaging::SmoothCamInterface::IsCrosshairTaken() const noexcept {
	return crosshairOwner.load(std::memory_order::memory_order_acquire) != SKSE::kInvalidPluginHandle;
}

bool Messaging::SmoothCamInterface::IsStealthMeterTaken() const noexcept {
	return stealthMeterOwner.load(std::memory_order::memory_order_acquire) != SKSE::kInvalidPluginHandle;
}

bool Messaging::SmoothCamInterface::CrosshairDirty() const noexcept {
	return wantCrosshairReset;
}

bool Messaging::SmoothCamInterface::StealthMeterDirty() const noexcept {
	return wantStealthMeterReset;
}

void Messaging::SmoothCamInterface::ClearCrosshairDirtyFlag() noexcept {
	wantCrosshairReset = false;
}

void Messaging::SmoothCamInterface::ClearStealthMeterDirtyFlag() noexcept {
	wantStealthMeterReset = false;
}

bool Messaging::SmoothCamInterface::WantsInterpolatorUpdates() const noexcept {
	return wantsInterpolatorUpdates;
}

bool Messaging::SmoothCamInterface::WantsMoveToGoal() const noexcept {
	return wantsMoveToGoal;
}

void Messaging::SmoothCamInterface::ClearMoveToGoalFlag() noexcept {
	wantsMoveToGoal = false;
}

bool Messaging::SmoothCamInterface::IsHorseAimUnlocked() const noexcept {
	return unlockedHorseAim;
}

void Messaging::SmoothCamInterface::RegisterConsumer(const char* modName) noexcept {
	consumers.push_back(eastl::move(eastl::string(modName)));
	logger::info(FMT_STRING("Added API consumer '{}'"), modName);
}

const Messaging::SmoothCamInterface::Consumers& Messaging::SmoothCamInterface::GetConsumers() const noexcept {
	return consumers;
}

void Messaging::HandleInterfaceRequest(SKSE::MessagingInterface::Message* msg) noexcept {
	if (msg->type != 0) return;
	constexpr const auto DispatchToPlugin = [](SmoothCamAPI::PluginResponse* packet, const char* to) noexcept {
		if (!g_messaging->Dispatch(0, packet, sizeof(SmoothCamAPI::PluginResponse), to))
			logger::warn(FMT_STRING("Failed to dispatch API message to '{}'"), to ? to : "unnamed");
	};

	SmoothCamAPI::PluginResponse packet = {};
	packet.type = SmoothCamAPI::PluginResponse::Type::Error;
	
	if (msg->dataLen != sizeof(SmoothCamAPI::PluginCommand)) {
		DispatchToPlugin(&packet, msg->sender);
		return;
	}

	const auto cmd = reinterpret_cast<const SmoothCamAPI::PluginCommand*>(msg->data);
	if (cmd->header != 0x9007CA50 || cmd->type != SmoothCamAPI::PluginCommand::Type::RequestInterface) {
		DispatchToPlugin(&packet, msg->sender);
		return;
	}

	const auto request = reinterpret_cast<const SmoothCamAPI::InterfaceRequest*>(cmd->commandStructure);
	if (!(request->interfaceVersion == SmoothCamAPI::InterfaceVersion::V1 ||
		  request->interfaceVersion == SmoothCamAPI::InterfaceVersion::V2 ||
		  request->interfaceVersion == SmoothCamAPI::InterfaceVersion::V3))
	{
		DispatchToPlugin(&packet, msg->sender);
		return;
	}

	auto api = Messaging::SmoothCamInterface::GetInstance();
	if (msg->sender)
		api->RegisterConsumer(msg->sender);
	else
		logger::info("Added unnamed API consumer");

	SmoothCamAPI::InterfaceContainer container = {};
	container.interfaceVersion = request->interfaceVersion;

	switch (request->interfaceVersion) {
		case SmoothCamAPI::InterfaceVersion::V1: [[fallthrough]];
		case SmoothCamAPI::InterfaceVersion::V2: [[fallthrough]];
		case SmoothCamAPI::InterfaceVersion::V3:
			container.interfaceInstance = static_cast<void*>(api);
			break;
		default:
			api->RegisterConsumer(msg->sender);
			return;
	}
	
	packet.type = SmoothCamAPI::PluginResponse::Type::InterfaceProvider;
	packet.responseData = &container;
	DispatchToPlugin(&packet, msg->sender);
}