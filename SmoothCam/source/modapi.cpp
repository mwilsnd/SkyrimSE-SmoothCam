#include "modapi.h"
#include "camera.h"
#include "thirdperson.h"
#include "crosshair.h"
#include "debug/eh.h"

extern eastl::unique_ptr<Camera::Camera> g_theCamera;
extern PluginHandle g_pluginHandle;
extern const SKSEInterface* g_skse;
extern const SKSEMessagingInterface* g_messaging;

Messaging::SmoothCamInterface::SmoothCamInterface() noexcept {
	apiTID = GetCurrentThreadId();
}

Messaging::SmoothCamInterface::~SmoothCamInterface() noexcept {}

DWORD Messaging::SmoothCamInterface::GetSmoothCamThreadId() const noexcept {
	return apiTID;
}

Messaging::APIResult Messaging::SmoothCamInterface::RequestCameraControl(PluginHandle modHandle) noexcept {
	const auto owner = cameraOwner.load(std::memory_order::memory_order_acquire);
	if (owner != kPluginHandle_Invalid)
		if (owner == modHandle)
			return APIResult::AlreadyGiven;
		else
			return APIResult::AlreadyTaken;

	if (needsCameraControl) return APIResult::MustKeep;
	auto expected = static_cast<PluginHandle>(kPluginHandle_Invalid);
	if (!cameraOwner.compare_exchange_strong(expected, modHandle, std::memory_order::memory_order_acq_rel))
		return APIResult::AlreadyTaken;

	wantsInterpolatorUpdates = false;
	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamInterface::RequestCrosshairControl(PluginHandle modHandle,
	bool restoreDefaults) noexcept
{
	const auto owner = crosshairOwner.load(std::memory_order::memory_order_acquire);
	if (owner != kPluginHandle_Invalid)
		if (owner == modHandle)
			return APIResult::AlreadyGiven;
		else
			return APIResult::AlreadyTaken;

	if (needsCrosshairControl) return APIResult::MustKeep;
	auto expected = static_cast<PluginHandle>(kPluginHandle_Invalid);
	if (!crosshairOwner.compare_exchange_strong(expected, modHandle, std::memory_order::memory_order_acq_rel))
		return APIResult::AlreadyTaken;

	if (restoreDefaults && crosshairMgr) {
		const auto mdmp = Debug::MiniDumpScope();
		crosshairMgr->ResetCrosshair();
	}

	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamInterface::RequestStealthMeterControl(PluginHandle modHandle,
	bool restoreDefaults) noexcept
{
	const auto owner = stealthMeterOwner.load(std::memory_order::memory_order_acquire);
	if (owner != kPluginHandle_Invalid)
		if (owner == modHandle)
			return APIResult::AlreadyGiven;
		else
			return APIResult::AlreadyTaken;

	if (needsStealthMeterControl) return APIResult::MustKeep;
	auto expected = static_cast<PluginHandle>(kPluginHandle_Invalid);
	if (!stealthMeterOwner.compare_exchange_strong(expected, modHandle, std::memory_order::memory_order_acq_rel))
		return APIResult::AlreadyTaken;

	if (restoreDefaults && crosshairMgr) {
		const auto mdmp = Debug::MiniDumpScope();
		crosshairMgr->ResetStealthMeter();
	}

	return APIResult::OK;
}

PluginHandle Messaging::SmoothCamInterface::GetCameraOwner() const noexcept {
	return cameraOwner;
}

PluginHandle Messaging::SmoothCamInterface::GetCrosshairOwner() const noexcept {
	return crosshairOwner;
}

PluginHandle Messaging::SmoothCamInterface::GetStealthMeterOwner() const noexcept {
	return stealthMeterOwner;
}

Messaging::APIResult Messaging::SmoothCamInterface::ReleaseCameraControl(PluginHandle modHandle) noexcept {
	if (cameraOwner != modHandle) return APIResult::NotOwner;
	cameraOwner.store(kPluginHandle_Invalid, std::memory_order::memory_order_release);
	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamInterface::ReleaseCrosshairControl(PluginHandle modHandle) noexcept {
	if (crosshairOwner != modHandle) return APIResult::NotOwner;
	crosshairOwner.store(kPluginHandle_Invalid, std::memory_order::memory_order_release);
	wantCrosshairReset = true;
	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamInterface::ReleaseStealthMeterControl(PluginHandle modHandle) noexcept {
	if (stealthMeterOwner != modHandle) return APIResult::NotOwner;
	stealthMeterOwner.store(kPluginHandle_Invalid, std::memory_order::memory_order_release);
	wantStealthMeterReset = true;
	return APIResult::OK;
}

NiPoint3 Messaging::SmoothCamInterface::GetLastCameraPosition() const noexcept {
	return g_theCamera->GetThirdpersonCamera()->GetPosition().ToNiPoint3();
}

Messaging::APIResult Messaging::SmoothCamInterface::RequestInterpolatorUpdates(PluginHandle modHandle, bool allowUpdates) noexcept {
	if (cameraOwner != modHandle) return APIResult::NotOwner;
	wantsInterpolatorUpdates = allowUpdates;
	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamInterface::SendToGoalPosition(PluginHandle modHandle, bool shouldMoveToGoal,
	bool moveNow, const TESObjectREFR* ref, NiCamera* niCamera) noexcept
{
	if (cameraOwner != modHandle) return APIResult::NotOwner;
	if (moveNow) {
		g_theCamera->GetThirdpersonCamera()->MoveToGoalPosition(
			*g_thePlayer, CorrectedPlayerCamera::GetSingleton(), ref ? ref : *g_thePlayer, niCamera
		);
	} else {
		wantsMoveToGoal = shouldMoveToGoal;
	}
	return APIResult::OK;
}

void Messaging::SmoothCamInterface::GetGoalPosition(TESObjectREFR* ref, NiPoint3& world, NiPoint3& local) const noexcept {
	if (!ref) {
		world = {};
		local = {};
		return;
	}

	glm::vec3 vworld{};
	glm::vec3 vlocal{};
	g_theCamera->GetThirdpersonCamera()->GetCameraGoalPosition(CorrectedPlayerCamera::GetSingleton(), vworld, vlocal, ref);
	world = { vworld.x, vworld.y, vworld.z };
	local = { vlocal.x, vlocal.y, vlocal.z };
}

bool Messaging::SmoothCamInterface::IsCameraEnabled() const noexcept {
	return !Config::GetCurrentConfig()->modDisabled;
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
	return cameraOwner.load(std::memory_order::memory_order_acquire) != kPluginHandle_Invalid;
}

bool Messaging::SmoothCamInterface::IsCrosshairTaken() const noexcept {
	return crosshairOwner.load(std::memory_order::memory_order_acquire) != kPluginHandle_Invalid;
}

bool Messaging::SmoothCamInterface::IsStealthMeterTaken() const noexcept {
	return stealthMeterOwner.load(std::memory_order::memory_order_acquire) != kPluginHandle_Invalid;
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

void Messaging::SmoothCamInterface::RegisterConsumer(const char* modName) noexcept {
	consumers.push_back(eastl::move(eastl::string(modName)));
	_MESSAGE("Added API consumer '%s'", modName);
}

const Messaging::SmoothCamInterface::Consumers& Messaging::SmoothCamInterface::GetConsumers() const noexcept {
	return consumers;
}

void Messaging::HandleInterfaceRequest(SKSEMessagingInterface::Message* msg) noexcept {
	if (msg->type != 0) return;
	constexpr const auto DispatchToPlugin = [](SmoothCamAPI::PluginResponse* packet, const char* to) noexcept {
		if (!g_messaging->Dispatch(g_pluginHandle, 0, packet, sizeof(SmoothCamAPI::PluginResponse), to))
			_WARNING("Failed to dispatch API message to %s", to);
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
		request->interfaceVersion == SmoothCamAPI::InterfaceVersion::V2))
	{
		DispatchToPlugin(&packet, msg->sender);
		return;
	}

	auto api = Messaging::SmoothCamInterface::GetInstance();
	if (msg->sender)
		api->RegisterConsumer(msg->sender);
	else
		_MESSAGE("Added unnamed API consumer");

	SmoothCamAPI::InterfaceContainer container = {};
	container.interfaceVersion = request->interfaceVersion;

	switch (request->interfaceVersion) {
		case SmoothCamAPI::InterfaceVersion::V1:
			container.interfaceInstance = dynamic_cast<InterfaceVersion1*>(api);
			break;
		case SmoothCamAPI::InterfaceVersion::V2:
			container.interfaceInstance = dynamic_cast<InterfaceVersion2*>(api);
			break;
		default:
			api->RegisterConsumer(msg->sender);
			return;
	}
	
	packet.type = SmoothCamAPI::PluginResponse::Type::InterfaceProvider;
	packet.responseData = &container;
	DispatchToPlugin(&packet, msg->sender);
}