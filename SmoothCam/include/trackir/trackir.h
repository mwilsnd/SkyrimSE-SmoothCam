#pragma once
#ifdef DEVELOPER
namespace TrackIR {
	constexpr uint16_t NPQUERYVERSION = 1040;

	constexpr uint8_t NPSTATUS_REMOTEACTIVE = 0;
	constexpr uint8_t NPSTATUS_REMOTEDISABLED = 1;

	constexpr uint8_t NPVERSIONMAJOR = 1;
	constexpr uint8_t NPVERSIONMINOR = 2;

	constexpr uint16_t NPRoll = 1;
	constexpr uint16_t NPPitch = 2;
	constexpr uint16_t NPYaw = 4;

	constexpr uint16_t NPControl = 8;

	constexpr uint16_t NPX = 16;
	constexpr uint16_t NPY = 32;
	constexpr uint16_t NPZ = 64;

	constexpr uint16_t NPRawX = 128;
	constexpr uint16_t NPRawY = 256;
	constexpr uint16_t NPRawZ = 512;

	constexpr uint16_t NPDeltaX = 1024;
	constexpr uint16_t NPDeltaY = 2048;
	constexpr uint16_t NPDeltaZ = 4096;

	constexpr uint16_t NPSmoothX = 8192;
	constexpr uint16_t NPSmoothY = 16384;
	constexpr uint16_t NPSmoothZ = 32768;

	constexpr float NPMaxValue = 16383.0f;
	constexpr float NPMinValue = -16383.0f;

	constexpr float MaxRotation = 180.0f;

	enum class NPResult : uint8_t {
		OK = 0,
		DEVICE_NOT_PRESENT,
		UNSUPPORTED_OS,
		INVALID_ARG,
		DLL_NOT_FOUND,
		NO_DATA,
		INTERNAL_DATA
	};
	
#pragma pack(push, 1)
	typedef struct tagTrackIRSignature {
		char DllSignature[200];
		char AppSignature[200];
	} SIGNATUREDATA, *LPTRACKIRSIGNATURE;

	typedef struct tagTrackIRData {
		uint16_t wNPStatus;
		uint16_t wPFrameSignature;
		uint32_t dwNPIOData;

		float fNPRoll;
		float fNPPitch;
		float fNPYaw;
		float fNPX;
		float fNPY;
		float fNPZ;
		float fNPRawX;
		float fNPRawY;
		float fNPRawZ;
		float fNPDeltaX;
		float fNPDeltaY;
		float fNPDeltaZ;
		float fNPSmoothX;
		float fNPSmoothY;
		float fNPSmoothZ;
	} TRACKIRDATA, *LPTRACKIRDATA;
#pragma pack(pop)

	typedef NPResult(__stdcall *PF_NOTIFYCALLBACK)(uint16_t, uint16_t);
	typedef NPResult(__stdcall *PF_NP_REGISTERWINDOWHANDLE)(HWND);
	typedef NPResult(__stdcall *PF_NP_UNREGISTERWINDOWHANDLE)(void);
	typedef NPResult(__stdcall *PF_NP_REGISTERPROGRAMPROFILEID)(uint16_t);
	typedef NPResult(__stdcall *PF_NP_QUERYVERSION)(uint16_t*);
	typedef NPResult(__stdcall *PF_NP_REQUESTDATA)(uint16_t);
	typedef NPResult(__stdcall *PF_NP_GETSIGNATURE)(LPTRACKIRSIGNATURE);
	typedef NPResult(__stdcall *PF_NP_GETDATA)(LPTRACKIRDATA);
	typedef NPResult(__stdcall *PF_NP_STARTCURSOR)(void);
	typedef NPResult(__stdcall *PF_NP_STOPCURSOR)(void);
	typedef NPResult(__stdcall *PF_NP_RECENTER)(void);
	typedef NPResult(__stdcall *PF_NP_STARTDATATRANSMISSION)(void);
	typedef NPResult(__stdcall *PF_NP_STOPDATATRANSMISSION)(void);

	NPResult __stdcall RegisterWindowHandle(HWND hWnd);
	NPResult __stdcall UnregisterWindowHandle(void);
	NPResult __stdcall RegisterProgramProfileID(uint16_t wPPID);
	NPResult __stdcall QueryVersion(uint16_t* pwVersion);
	NPResult __stdcall RequestData(uint16_t wDataReq);
	NPResult __stdcall GetSignature(LPTRACKIRSIGNATURE pSignature);
	NPResult __stdcall GetData(LPTRACKIRDATA pTID);
	NPResult __stdcall StartCursor(void);
	NPResult __stdcall StopCursor(void);
	NPResult __stdcall ReCenter(void);
	NPResult __stdcall StartDataTransmission(void);
	NPResult __stdcall StopDataTransmission(void);

	NPResult Init(LPTSTR pszDLLPath);
	NPResult GetDLLLocation(LPTSTR pszPath);

	NPResult Initialize(HWND hWnd);
	void Shutdown();
	bool IsRunning() noexcept;

	struct TrackingSnapshot {
		glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rot = { 0.0f, 0.0f, 0.0f };
	};
	TrackingSnapshot GetTrackingData() noexcept;
}
#endif