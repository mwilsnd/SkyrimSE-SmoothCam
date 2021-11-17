#pragma once
#define SMOOTHCAM_API_COMMONLIB
#include "SmoothCamAPI.h"

namespace Crosshair {
	class Manager;
}

namespace Messaging {
	using APIResult = ::SmoothCamAPI::APIResult;
	using InterfaceVersion1 = ::SmoothCamAPI::IVSmoothCam1;
	using InterfaceVersion2 = ::SmoothCamAPI::IVSmoothCam2;
	using InterfaceVersion3 = ::SmoothCamAPI::IVSmoothCam3;

	class SmoothCamInterface : public InterfaceVersion3 {
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
			virtual APIResult RequestCameraControl(SKSE::PluginHandle modHandle) noexcept override;
			virtual APIResult RequestCrosshairControl(SKSE::PluginHandle modHandle, bool restoreDefaults = true) noexcept override;
			virtual APIResult RequestStealthMeterControl(SKSE::PluginHandle modHandle, bool restoreDefaults = true) noexcept override;
			virtual SKSE::PluginHandle GetCameraOwner() const noexcept override;
			virtual SKSE::PluginHandle GetCrosshairOwner() const noexcept override;
			virtual SKSE::PluginHandle GetStealthMeterOwner() const noexcept override;
			virtual APIResult ReleaseCameraControl(SKSE::PluginHandle modHandle) noexcept override;
			virtual APIResult ReleaseCrosshairControl(SKSE::PluginHandle modHandle) noexcept override;
			virtual APIResult ReleaseStealthMeterControl(SKSE::PluginHandle modHandle) noexcept override;

			// InterfaceVersion2
			virtual RE::NiPoint3 GetLastCameraPosition() const noexcept override;
			virtual APIResult RequestInterpolatorUpdates(SKSE::PluginHandle modHandle, bool allowUpdates) noexcept override;
			virtual APIResult SendToGoalPosition(SKSE::PluginHandle modHandle, bool shouldMoveToGoal, bool moveNow,
				const RE::Actor* ref) noexcept override;
			virtual void GetGoalPosition(RE::TESObjectREFR* ref, RE::NiPoint3& world, RE::NiPoint3& local) const noexcept override;
			virtual bool IsCameraEnabled() const noexcept override;

			// InterfaceVersion3
			virtual void EnableUnlockedHorseAim(bool enable) noexcept override;

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

			// Return the current horse aim unlock state
			bool IsHorseAimUnlocked() const noexcept;

		public:
			using Consumers = eastl::vector<eastl::string>;

			void RegisterConsumer(const char* modName) noexcept;
			const Consumers& GetConsumers() const noexcept;

		private:
			Crosshair::Manager* crosshairMgr = nullptr;
			Consumers consumers = {};
			DWORD apiTID = 0;

			bool needsCameraControl = false;
			std::atomic<SKSE::PluginHandle> cameraOwner = SKSE::kInvalidPluginHandle;

			bool needsCrosshairControl = false;
			bool wantCrosshairReset = false;
			std::atomic<SKSE::PluginHandle> crosshairOwner = SKSE::kInvalidPluginHandle;

			bool needsStealthMeterControl = false;
			bool wantStealthMeterReset = false;
			std::atomic<SKSE::PluginHandle> stealthMeterOwner = SKSE::kInvalidPluginHandle;

			bool wantsInterpolatorUpdates = false;
			bool wantsMoveToGoal = false;

			bool unlockedHorseAim = false;
	};

	void HandleInterfaceRequest(SKSE::MessagingInterface::Message* msg) noexcept;
}