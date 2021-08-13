#include "modapi.h"
#include "crosshair.h"
#include "debug/eh.h"

extern PluginHandle g_pluginHandle;
extern const SKSEInterface* g_skse;
extern const SKSEMessagingInterface* g_messaging;

Messaging::SmoothCamAPIV1::SmoothCamAPIV1() noexcept {
	apiTID = GetCurrentThreadId();
}

Messaging::SmoothCamAPIV1::~SmoothCamAPIV1() noexcept {}

DWORD Messaging::SmoothCamAPIV1::GetSmoothCamThreadId() const noexcept {
	return apiTID;
}

Messaging::APIResult Messaging::SmoothCamAPIV1::RequestCameraControl(PluginHandle modHandle) noexcept {
	if (GetCurrentThreadId() != apiTID) return APIResult::BadThread;
	
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

	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamAPIV1::RequestCrosshairControl(PluginHandle modHandle,
	bool restoreDefaults) noexcept
{
	if (GetCurrentThreadId() != apiTID) return APIResult::BadThread;

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

Messaging::APIResult Messaging::SmoothCamAPIV1::RequestStealthMeterControl(PluginHandle modHandle,
	bool restoreDefaults) noexcept
{
	if (GetCurrentThreadId() != apiTID) return APIResult::BadThread;

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

PluginHandle Messaging::SmoothCamAPIV1::GetCameraOwner() const noexcept {
	return cameraOwner;
}

PluginHandle Messaging::SmoothCamAPIV1::GetCrosshairOwner() const noexcept {
	return crosshairOwner;
}

PluginHandle Messaging::SmoothCamAPIV1::GetStealthMeterOwner() const noexcept {
	return stealthMeterOwner;
}

Messaging::APIResult Messaging::SmoothCamAPIV1::ReleaseCameraControl(PluginHandle modHandle) noexcept {
	if (cameraOwner != modHandle) return APIResult::NotOwner;
	cameraOwner.store(kPluginHandle_Invalid, std::memory_order::memory_order_release);
	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamAPIV1::ReleaseCrosshairControl(PluginHandle modHandle) noexcept {
	if (crosshairOwner != modHandle) return APIResult::NotOwner;
	crosshairOwner.store(kPluginHandle_Invalid, std::memory_order::memory_order_release);
	wantCrosshairReset = true;
	return APIResult::OK;
}

Messaging::APIResult Messaging::SmoothCamAPIV1::ReleaseStealthMeterControl(PluginHandle modHandle) noexcept {
	if (stealthMeterOwner != modHandle) return APIResult::NotOwner;
	stealthMeterOwner.store(kPluginHandle_Invalid, std::memory_order::memory_order_release);
	wantStealthMeterReset = true;
	return APIResult::OK;
}

void Messaging::SmoothCamAPIV1::SetCrosshairManager(Crosshair::Manager* mgr) noexcept {
	crosshairMgr = mgr;
}

void Messaging::SmoothCamAPIV1::SetNeedsCameraControl(bool needsControl) noexcept {
	needsCameraControl = needsControl;
}

void Messaging::SmoothCamAPIV1::SetNeedsCrosshairControl(bool needsControl) noexcept {
	needsCrosshairControl = needsControl;
}

void Messaging::SmoothCamAPIV1::SetNeedsStealthMeterControl(bool needsControl) noexcept {
	needsStealthMeterControl = needsControl;
}

bool Messaging::SmoothCamAPIV1::IsCameraTaken() const noexcept {
	return cameraOwner.load(std::memory_order::memory_order_acquire) != kPluginHandle_Invalid;
}

bool Messaging::SmoothCamAPIV1::IsCrosshairTaken() const noexcept {
	return crosshairOwner.load(std::memory_order::memory_order_acquire) != kPluginHandle_Invalid;
}

bool Messaging::SmoothCamAPIV1::IsStealthMeterTaken() const noexcept {
	return stealthMeterOwner.load(std::memory_order::memory_order_acquire) != kPluginHandle_Invalid;
}

bool Messaging::SmoothCamAPIV1::CrosshairDirty() const noexcept {
	return wantCrosshairReset;
}

bool Messaging::SmoothCamAPIV1::StealthMeterDirty() const noexcept {
	return wantStealthMeterReset;
}

void Messaging::SmoothCamAPIV1::ClearCrosshairDirtyFlag() noexcept {
	wantCrosshairReset = false;
}

void Messaging::SmoothCamAPIV1::ClearStealthMeterDirtyFlag() noexcept {
	wantStealthMeterReset = false;
}

void Messaging::SmoothCamAPIV1::RegisterConsumer(const char* modName) noexcept {
	consumers.push_back(eastl::move(eastl::string(modName)));
	_MESSAGE("Added API consumer '%s'", modName);
}

const Messaging::SmoothCamAPIV1::Consumers& Messaging::SmoothCamAPIV1::GetConsumers() const noexcept {
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
	if (request->interfaceVersion != SmoothCamAPI::InterfaceVersion::V1) {
		DispatchToPlugin(&packet, msg->sender);
		return;
	}

	auto apiV1 = Messaging::SmoothCamAPIV1::GetInstance();
	if (msg->sender)
		apiV1->RegisterConsumer(msg->sender);
	else
		_MESSAGE("Added unnamed API consumer");

	SmoothCamAPI::InterfaceContainer container = {};
	container.interfaceVersion = SmoothCamAPI::InterfaceVersion::V1;
	container.interfaceInstance = apiV1;
	packet.type = SmoothCamAPI::PluginResponse::Type::InterfaceProvider;
	packet.responseData = &container;
	DispatchToPlugin(&packet, msg->sender);
}