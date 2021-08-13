#pragma once
#define SMOOTHCAM_API_SKSE
#include "SmoothCamAPI.h"

namespace Crosshair {
	class Manager;
}

namespace Messaging {
	using APIResult = ::SmoothCamAPI::APIResult;
	using InterfaceVersion1 = ::SmoothCamAPI::IVSmoothCam1;

	class SmoothCamAPIV1 : public InterfaceVersion1 {
		private:
			SmoothCamAPIV1() noexcept;
			virtual ~SmoothCamAPIV1() noexcept;

		public:
			static SmoothCamAPIV1* GetInstance() noexcept {
				static SmoothCamAPIV1 instance;
				return &instance;
			}

			// InterfaceVersion1
			virtual DWORD GetSmoothCamThreadId() const noexcept override;
			virtual APIResult RequestCameraControl(PluginHandle modHandle) noexcept override;
			virtual APIResult RequestCrosshairControl(PluginHandle modHandle, bool restoreDefaults = true) noexcept override;
			virtual APIResult RequestStealthMeterControl(PluginHandle modHandle, bool restoreDefaults = true) noexcept override;
			virtual PluginHandle GetCameraOwner() const noexcept override;
			virtual PluginHandle GetCrosshairOwner() const noexcept override;
			virtual PluginHandle GetStealthMeterOwner() const noexcept override;
			virtual APIResult ReleaseCameraControl(PluginHandle modHandle) noexcept override;
			virtual APIResult ReleaseCrosshairControl(PluginHandle modHandle) noexcept override;
			virtual APIResult ReleaseStealthMeterControl(PluginHandle modHandle) noexcept override;

			// Internal
			// Provide the crosshair manager for API reset control
			void SetCrosshairManager(Crosshair::Manager* mgr) noexcept;

			// Mark camera control as required by SmoothCam for API requests
			void SetNeedsCameraControl(bool needsControl) noexcept;
			// Mark crosshair control as required by SmoothCam for API requests
			void SetNeedsCrosshairControl(bool needsControl) noexcept;
			// Mark stealth meter control as required by SmoothCam for API requests
			void SetNeedsStealthMeterControl(bool needsControl) noexcept;

			// Does a mod have control over the camera?
			bool IsCameraTaken() const noexcept;
			// Does a mod have control over the crosshair?
			bool IsCrosshairTaken() const noexcept;
			// Does a mod have control over the stealth meter?
			bool IsStealthMeterTaken() const noexcept;

			// Set when a mod releases control of the crosshair so we know to perform cleanup
			bool CrosshairDirty() const noexcept;
			// Set when a mod releases control of the stealth meter so we know to perform cleanup
			bool StealthMeterDirty() const noexcept;

			// Clear the dirty flag on the crosshair
			void ClearCrosshairDirtyFlag() noexcept;
			// Clear the dirty flag on the stealth meter
			void ClearStealthMeterDirtyFlag() noexcept;

		public:
			using Consumers = eastl::vector<eastl::string>;

			void RegisterConsumer(const char* modName) noexcept;
			const Consumers& GetConsumers() const noexcept;

		private:
			Crosshair::Manager* crosshairMgr = nullptr;
			Consumers consumers = {};
			DWORD apiTID = 0;

			bool needsCameraControl = false;
			std::atomic<PluginHandle> cameraOwner = kPluginHandle_Invalid;

			bool needsCrosshairControl = false;
			bool wantCrosshairReset = false;
			std::atomic<PluginHandle> crosshairOwner = kPluginHandle_Invalid;

			bool needsStealthMeterControl = false;
			bool wantStealthMeterReset = false;
			std::atomic<PluginHandle> stealthMeterOwner = kPluginHandle_Invalid;
	};

	void HandleInterfaceRequest(SKSEMessagingInterface::Message* msg) noexcept;
}