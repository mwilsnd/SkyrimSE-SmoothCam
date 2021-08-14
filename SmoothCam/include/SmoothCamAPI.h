#pragma once
#include <functional>
#include <stdint.h>

#if !defined(SMOOTHCAM_API_SKSE) && !defined(SMOOTHCAM_API_COMMONLIB)
static_assert(0, "SmoothCamAPI: SKSE API type not defined. Define one of (SMOOTHCAM_API_SKSE, SMOOTHCAM_API_COMMONLIB)")
#endif

#if defined(SMOOTHCAM_API_SKSE) && defined(SMOOTHCAM_API_COMMONLIB)
static_assert(0, "SmoothCamAPI: Define one of (SMOOTHCAM_API_SKSE, SMOOTHCAM_API_COMMONLIB), not both.")
#endif

#ifdef SMOOTHCAM_API_SKSE
#define SMOOTHCAM_API_LOGGER _MESSAGE
#elif defined(SMOOTHCAM_API_COMMONLIB)
#define SMOOTHCAM_API_LOGGER SKSE::log::info
#endif

/*
* For modders: Copy this file into your own project if you wish to use this API
* 
* #define SMOOTHCAM_API_SKSE if using SKSE
* #define SMOOTHCAM_API_COMMONLIB if using CommonLibSSE
*/
namespace SmoothCamAPI {
	constexpr const auto SmoothCamPluginName = "SmoothCam";

#ifdef SMOOTHCAM_API_COMMONLIB
	using PluginHandle = SKSE::PluginHandle;
	using TESObjectREFR = RE::TESObjectREFR;
	using NiCamera = RE::NiCamera;
	using NiPoint3 = RE::NiPoint3;
#endif

	// Available SmoothCam interface versions
	enum class InterfaceVersion : uint8_t {
		V1,
		V2
	};

	// Error types that may be returned by the SmoothCam API
	enum class APIResult : uint8_t {
		// Your API call was successful
		OK,

		// You tried to release a resource that was not allocated to you
		// Do not attempt to manipulate the requested resource if you receive this response
		NotOwner,

		// SmoothCam currently must keep control of this resource for proper functionality
		// Do not attempt to manipulate the requested resource if you receive this response
		MustKeep,

		// You have already been given control of this resource
		AlreadyGiven,

		// Another mod has been given control of this resource at the present time
		// Do not attempt to manipulate the requested resource if you receive this response
		AlreadyTaken,

		// You sent a command on a thread that could cause a data race were it to be processed
		// Do not attempt to manipulate the requested resource if you receive this response
		BadThread,
	};

	// SmoothCam's modder interface
	class IVSmoothCam1 {
		public:
		/// <summary>
		/// Get the thread ID SmoothCam is running in.
		/// You may compare this with the result of GetCurrentThreadId() to help determine
		/// if you are using the correct thread.
		/// </summary>
		/// <returns>TID</returns>
		[[nodiscard]] virtual unsigned long GetSmoothCamThreadId() const noexcept = 0;

		/// <summary>
		/// Request control of the player camera.
		/// If granted, you may manipulate the camera in whatever ways you wish for the duration of your control.
		/// SmoothCam will not perform any positional clean-up of the camera or camera states, you get the camera as-is.
		/// </summary>
		/// <param name="myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, MustKeep, AlreadyGiven, AlreadyTaken</returns>
		[[nodiscard]] virtual APIResult RequestCameraControl(PluginHandle myPluginHandle) noexcept = 0;

		/// <summary>
		/// Request control of the HUD crosshair.
		/// If enabled, SmoothCam will hide the world-space crosshair for the duration of your control
		/// </summary>
		/// <param name="myPluginHandle">Your assigned plugin handle</param>
		/// <param name="restoreDefaults">SmoothCam will first restore the crosshair to default settings</param>
		/// <returns>OK, MustKeep, AlreadyGiven, AlreadyTaken</returns>
		[[nodiscard]] virtual APIResult RequestCrosshairControl(PluginHandle myPluginHandle,
			bool restoreDefaults = true) noexcept = 0;

		/// <summary>
		/// Request control of the HUD stealth meter.
		/// If granted, you may maniuplate the resource in whatever ways you wish for the duration of your control.
		/// </summary>
		/// <param name="myPluginHandle">Your assigned plugin handle</param>
		/// <param name="restoreDefaults">SmoothCam will first restore the stealth meter to default settings</param>
		/// <returns>OK, MustKeep, AlreadyGiven, AlreadyTaken</returns>
		[[nodiscard]] virtual APIResult RequestStealthMeterControl(PluginHandle myPluginHandle,
			bool restoreDefaults = true) noexcept = 0;

		/// <summary>
		/// Returns the current owner of the camera resource
		/// </summary>
		/// <returns>Handle or kPluginHandle_Invalid if no one currently owns the resource</returns>
		virtual PluginHandle GetCameraOwner() const noexcept = 0;

		/// <summary>
		/// Returns the current owner of the crosshair resource
		/// </summary>
		/// <returns>Handle or kPluginHandle_Invalid if no one currently owns the resource</returns>
		virtual PluginHandle GetCrosshairOwner() const noexcept = 0;

		/// <summary>
		/// Returns the current owner of the stealth meter resource
		/// </summary>
		/// <returns>Handle or kPluginHandle_Invalid if no one currently owns the resource</returns>
		virtual PluginHandle GetStealthMeterOwner() const noexcept = 0;

		/// <summary>
		/// Release your control of the player camera.
		/// </summary>
		/// <param name="myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotOwner</returns>
		virtual APIResult ReleaseCameraControl(PluginHandle myPluginHandle) noexcept = 0;

		/// <summary>
		/// Release your control of the crosshair.
		/// </summary>
		/// <param name="myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotOwner</returns>
		virtual APIResult ReleaseCrosshairControl(PluginHandle myPluginHandle) noexcept = 0;

		/// <summary>
		/// Release your control of the stealth meter.
		/// </summary>
		/// <param name="myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotOwner</returns>
		virtual APIResult ReleaseStealthMeterControl(PluginHandle myPluginHandle) noexcept = 0;
	};

	class IVSmoothCam2 : public IVSmoothCam1 {
		public:
		/// <summary>
		/// Get the last world position of the camera set by SmoothCam.
		/// </summary>
		/// <returns>World position</returns>
		virtual NiPoint3 GetLastCameraPosition() const noexcept = 0;

		/// <summary>
		/// Request that position interpolators continue to update while you have camera control.
		/// SmoothCam will not update the camera position with interpolators running, but will enable functions like
		/// `GetLastCameraPosition` to return current, up to date values while you own the camera resource.
		/// 
		/// When given control of the camera, the default action is to pause interpolator updates. If you want them
		/// to continue running, you must call this method each time you gain control of the camera.
		/// </summary>
		/// <param name="myPluginHandle">Your assigned plugin handle</param>
		/// <param name="allowUpdates">Allow interpolators to continue updates in the background</param>
		/// <returns>OK, NotOwner</returns>
		virtual APIResult RequestInterpolatorUpdates(PluginHandle myPluginHandle, bool allowUpdates) noexcept = 0;

		/// <summary>
		/// When called before releasing camera control, instructs the camera to move to its goal position once control
		/// is returned to SmoothCam.
		/// </summary>
		/// <param name="myPluginHandle">Your assigned plugin handle</param>
		/// <param name="shouldMoveToGoal">Instructs the camera to move to it's goal position when control is gained back</param>
		/// <param name="moveNow">Move to the current goal immediately</param>
		/// /// <param name="ref">Object reference for rotation calculations</param>
		/// <returns>OK, NotOwner</returns>
		virtual APIResult SendToGoalPosition(PluginHandle myPluginHandle, bool shouldMoveToGoal, bool moveNow = false,
			const TESObjectREFR* ref = nullptr, NiCamera* niCamera = nullptr) noexcept = 0;

		/// <summary>
		/// Return SmoothCam's current goal position.
		/// </summary>
		/// <param name="ref">Object reference for rotation calculations</param>
		/// <param name="world">World goal position</param>
		/// <param name="local">Player local goal position</param>
		/// <returns></returns>
		virtual void GetGoalPosition(TESObjectREFR* ref, NiPoint3& world, NiPoint3& local) const noexcept = 0;

		/// <summary>
		/// Returns false if the user has disabled SmoothCam via the MCM/hotkey.
		/// When in a disabled state, SmoothCam will not update position information.
		/// </summary>
		/// <returns>Enabled</returns>
		virtual bool IsCameraEnabled() const noexcept = 0;
	};

	struct PluginCommand {
		// Command types available
		enum class Type : uint8_t {
			RequestInterface,
		};

		// Packet header
		uint32_t header = 0x9007CA50;
		// Command type to invoke
		Type type;
		// Pointer to data for the given command
		void* commandStructure = nullptr;
	};

	struct InterfaceRequest {
		// Version to request
		InterfaceVersion interfaceVersion;
	};

	struct PluginResponse {
		enum class Type : uint8_t {
			Error,
			InterfaceProvider,
		};

		// Response type
		Type type;
		// Pointer to data for the given resposne
		void* responseData = nullptr;
	};

	struct InterfaceContainer {
		// Pointer to interface
		void* interfaceInstance = nullptr;
		// Contained version
		InterfaceVersion interfaceVersion;
	};

	using InterfaceLoaderCallback = std::function<void(
		void* interfaceInstance, InterfaceVersion interfaceVersion
	)>;

#ifdef SMOOTHCAM_API_SKSE
	/// <summary>
	/// Initiate a request for the SmoothCam API interface via SKSE's messaging system.
	/// You must register a callback to obtain the response to this request.
	/// Recommended: Send your request during SKSEMessagingInterface::kMessage_PostPostLoad
	/// </summary>
	/// <param name="skseMessaging">SKSE's messaging interface</param>
	/// <param name="myPluginHandle">Your assigned plugin handle</param>
	/// <param name="version">The interface version to request</param>
	/// <returns>If any plugin was listening for this request, true. See skse/PluginAPI.h</returns>
	[[nodiscard]]
	inline bool RequestInterface(SKSEMessagingInterface* skseMessaging, PluginHandle myPluginHandle,
		InterfaceVersion version = InterfaceVersion::V2) noexcept
	{
		InterfaceRequest req = {};
		req.interfaceVersion = version;

		PluginCommand cmd = {};
		cmd.type = PluginCommand::Type::RequestInterface;
		cmd.commandStructure = &req;

		return skseMessaging->Dispatch(
			myPluginHandle,
			0,
			&cmd, sizeof(PluginCommand),
			SmoothCamPluginName
		);
	};

	/// <summary>
	/// Register the callback for obtaining the SmoothCam API interface. Call only once.
	/// Recommended: Register your callback during SKSEMessagingInterface::kMessage_PostLoad
	/// </summary>
	/// <param name="skseMessaging">SKSE's messaging interface</param>
	/// <param name="myPluginHandle">Your assigned plugin handle</param>
	/// <param name="callback">A callback function receiving both the interface pointer and interface version</param>
	/// <returns></returns>
	[[nodiscard]]
	inline bool RegisterInterfaceLoaderCallback(SKSEMessagingInterface* skseMessaging,
		PluginHandle myPluginHandle, InterfaceLoaderCallback&& callback) noexcept
	{
		static InterfaceLoaderCallback storedCallback = callback;

		return skseMessaging->RegisterListener(
			myPluginHandle, SmoothCamPluginName,
			[](SKSEMessagingInterface::Message* msg) {
				if (msg->sender && strcmp(msg->sender, SmoothCamPluginName) != 0) return;
				if (msg->type != 0) return;
				if (msg->dataLen != sizeof(PluginResponse)) return;

				const auto resp = reinterpret_cast<PluginResponse*>(msg->data);
				switch (resp->type) {
					case PluginResponse::Type::InterfaceProvider: {
						auto interfaceContainer = reinterpret_cast<InterfaceContainer*>(resp->responseData);
						storedCallback(
							interfaceContainer->interfaceInstance,
							interfaceContainer->interfaceVersion
						);
						break;
					}
					case PluginResponse::Type::Error: {
						SMOOTHCAM_API_LOGGER("SmoothCam API: Error obtaining interface");
						break;
					}
					default: return;
				}
			}
		);
	}
#elif defined(SMOOTHCAM_API_COMMONLIB)
	/// <summary>
	/// Initiate a request for the SmoothCam API interface via SKSE's messaging system.
	/// You must register a callback to obtain the response to this request.
	/// Recommended: Send your request during SKSEMessagingInterface::kMessage_PostPostLoad
	/// </summary>
	/// <param name="skseMessaging">SKSE's messaging interface</param>
	/// <param name="version">The interface version to request</param>
	/// <returns>If any plugin was listening for this request, true. See skse/PluginAPI.h</returns>
	[[nodiscard]]
	inline bool RequestInterface(const SKSE::MessagingInterface* skseMessaging,
		InterfaceVersion version = InterfaceVersion::V2) noexcept
	{
		InterfaceRequest req = {};
		req.interfaceVersion = version;

		PluginCommand cmd = {};
		cmd.type = PluginCommand::Type::RequestInterface;
		cmd.commandStructure = &req;

		return skseMessaging->Dispatch(
			0,
			&cmd, sizeof(PluginCommand),
			SmoothCamPluginName
		);
	}

	/// <summary>
	/// Register the callback for obtaining the SmoothCam API interface. Call only once.
	/// Recommended: Register your callback during SKSEMessagingInterface::kMessage_PostLoad
	/// </summary>
	/// <param name="skseMessaging">SKSE's messaging interface</param>
	/// <param name="callback">A callback function receiving both the interface pointer and interface version</param>
	/// <returns></returns>
	[[nodiscard]]
	inline bool RegisterInterfaceLoaderCallback(const SKSE::MessagingInterface* skseMessaging,
		InterfaceLoaderCallback&& callback) noexcept
	{
		static InterfaceLoaderCallback storedCallback = callback;

		return skseMessaging->RegisterListener(
			SmoothCamPluginName,
			[](SKSE::MessagingInterface::Message* msg) {
				if (msg->sender && strcmp(msg->sender, SmoothCamPluginName) != 0) return;
				if (msg->type != 0) return;
				if (msg->dataLen != sizeof(PluginResponse)) return;

				const auto resp = reinterpret_cast<PluginResponse*>(msg->data);
				switch (resp->type) {
					case PluginResponse::Type::InterfaceProvider: {
						auto interfaceContainer = reinterpret_cast<InterfaceContainer*>(resp->responseData);
						storedCallback(
							interfaceContainer->interfaceInstance,
							interfaceContainer->interfaceVersion
						);
						break;
					}
					case PluginResponse::Type::Error: {
						SMOOTHCAM_API_LOGGER("SmoothCam API: Error obtaining interface");
						break;
					}
					default: return;
				}
			}
		);
	}
#endif
}