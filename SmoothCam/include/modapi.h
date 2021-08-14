#pragma once
#define SMOOTHCAM_API_SKSE
#include "SmoothCamAPI.h"

namespace Crosshair {
	class Manager;
}

namespace Messaging {
	using APIResult = ::SmoothCamAPI::APIResult;
	using InterfaceVersion1 = ::SmoothCamAPI::IVSmoothCam1;
	using InterfaceVersion2 = ::SmoothCamAPI::IVSmoothCam2;

	class SmoothCamInterface : public InterfaceVersion2 {
		private:
		SmoothCamInterface() noexcept;
			virtual ~SmoothCamInterface() noexcept;

		public:
			static SmoothCamInterface* GetInstance() noexcept {
				static SmoothCamInterface instance;
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

			// InterfaceVersion2
			virtual NiPoint3 GetLastCameraPosition() const noexcept override;
			virtual APIResult RequestInterpolatorUpdates(PluginHandle modHandle, bool allowUpdates) noexcept override;
			virtual APIResult SendToGoalPosition(PluginHandle modHandle, bool shouldMoveToGoal, bool moveNow,
				const TESObjectREFR* ref, NiCamera* niCamera) noexcept override;
			virtual void GetGoalPosition(TESObjectREFR* ref, NiPoint3& world, NiPoint3& local) const noexcept override;
			virtual bool IsCameraEnabled() const noexcept override;

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

			// The current camera owner has requested that we continue updating interpolators
			// in the background
			bool WantsInterpolatorUpdates() const noexcept;
			// The current camera owner has requested that we move to our goal position once
			// they return control to us
			bool WantsMoveToGoal() const noexcept;
			// Clear the moveToGoal flag after processing
			void ClearMoveToGoalFlag() noexcept;



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

			bool wantsInterpolatorUpdates = false;
			bool wantsMoveToGoal = false;
	};

	void HandleInterfaceRequest(SKSEMessagingInterface::Message* msg) noexcept;
}