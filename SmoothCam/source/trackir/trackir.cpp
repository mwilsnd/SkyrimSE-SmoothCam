#ifdef DEVELOPER
#include "trackir/trackir.h"

static bool running = false;

TrackIR::PF_NP_REGISTERWINDOWHANDLE       gpfNP_RegisterWindowHandle = NULL;
TrackIR::PF_NP_UNREGISTERWINDOWHANDLE     gpfNP_UnregisterWindowHandle = NULL;
TrackIR::PF_NP_REGISTERPROGRAMPROFILEID   gpfNP_RegisterProgramProfileID = NULL;
TrackIR::PF_NP_QUERYVERSION               gpfNP_QueryVersion = NULL;
TrackIR::PF_NP_REQUESTDATA                gpfNP_RequestData = NULL;
TrackIR::PF_NP_GETSIGNATURE               gpfNP_GetSignature = NULL;
TrackIR::PF_NP_GETDATA                    gpfNP_GetData = NULL;
TrackIR::PF_NP_STARTCURSOR                gpfNP_StartCursor = NULL;
TrackIR::PF_NP_STOPCURSOR                 gpfNP_StopCursor = NULL;
TrackIR::PF_NP_RECENTER	                  gpfNP_ReCenter = NULL;
TrackIR::PF_NP_STARTDATATRANSMISSION      gpfNP_StartDataTransmission = NULL;
TrackIR::PF_NP_STOPDATATRANSMISSION       gpfNP_StopDataTransmission = NULL;

HMODULE ghNPClientDLL = NULL;

TrackIR::NPResult __stdcall TrackIR::RegisterWindowHandle(HWND hWnd) {
	auto result = NPResult::DLL_NOT_FOUND;
	if (gpfNP_RegisterWindowHandle)
		result = (*gpfNP_RegisterWindowHandle)(hWnd);
	return result;
}

TrackIR::NPResult __stdcall TrackIR::UnregisterWindowHandle() {
	auto result = NPResult::DLL_NOT_FOUND;

	if (gpfNP_UnregisterWindowHandle)
		result = (*gpfNP_UnregisterWindowHandle)();
	return result;
}

TrackIR::NPResult __stdcall TrackIR::RegisterProgramProfileID(uint16_t wPPID) {
	auto result = NPResult::DLL_NOT_FOUND;

	if (gpfNP_RegisterProgramProfileID)
		result = (*gpfNP_RegisterProgramProfileID)(wPPID);
	return result;
}

TrackIR::NPResult __stdcall TrackIR::QueryVersion(uint16_t* pwVersion) {
	auto result = NPResult::DLL_NOT_FOUND;
	if (gpfNP_QueryVersion)
		result = (*gpfNP_QueryVersion)(pwVersion);
	return result;
}

TrackIR::NPResult __stdcall TrackIR::RequestData(uint16_t wDataReq) {
	auto result = NPResult::DLL_NOT_FOUND;
	if (gpfNP_RequestData)
		result = (*gpfNP_RequestData)(wDataReq);
	return result;
}

TrackIR::NPResult __stdcall TrackIR::GetSignature(LPTRACKIRSIGNATURE pSignature) {
	auto result = NPResult::DLL_NOT_FOUND;
	if (gpfNP_GetSignature)
		result = (*gpfNP_GetSignature)(pSignature);
	return result;
}

TrackIR::NPResult __stdcall TrackIR::GetData(LPTRACKIRDATA pTID) {
	auto result = NPResult::DLL_NOT_FOUND;
	if (gpfNP_GetData)
		result = (*gpfNP_GetData)(pTID);
	return result;
}

TrackIR::NPResult __stdcall TrackIR::StartCursor() {
	auto result = NPResult::DLL_NOT_FOUND;
	if (gpfNP_StartCursor)
		result = (*gpfNP_StartCursor)();
	return result;
}

TrackIR::NPResult __stdcall TrackIR::StopCursor() {
	auto result = NPResult::DLL_NOT_FOUND;
	if (gpfNP_StopCursor)
		result = (*gpfNP_StopCursor)();
	return result;
}

TrackIR::NPResult __stdcall TrackIR::ReCenter() {
	auto result = NPResult::DLL_NOT_FOUND;
	if (gpfNP_ReCenter)
		result = (*gpfNP_ReCenter)();
	return result;
}

TrackIR::NPResult __stdcall TrackIR::StartDataTransmission() {
	auto result = NPResult::DLL_NOT_FOUND;
	if (gpfNP_StartDataTransmission)
		result = (*gpfNP_StartDataTransmission)();
	return result;
}

TrackIR::NPResult __stdcall TrackIR::StopDataTransmission() {
	auto result = NPResult::DLL_NOT_FOUND;
	if (gpfNP_StopDataTransmission)
		result = (*gpfNP_StopDataTransmission)();
	return result;
}

TrackIR::NPResult TrackIR::Init(LPTSTR pszDLLPath) {
	NPResult result = NPResult::OK;
	WCHAR szFullPath[MAX_PATH * 2];

	if (pszDLLPath == NULL)
		return NPResult::DLL_NOT_FOUND;

	lstrcpy(szFullPath, pszDLLPath);

	if (lstrlen(szFullPath) > 0) {
		lstrcat(szFullPath, L"\\");
	}

#if defined(_WIN64) || defined(__amd64__)
	lstrcat(szFullPath, L"NPClient64.dll");
#else	    
	strcat(szFullPath, "NPClient.dll");
#endif

	ghNPClientDLL = ::LoadLibrary(szFullPath);
	if (NULL != ghNPClientDLL) {
		// verify the dll signature
		gpfNP_GetSignature = (PF_NP_GETSIGNATURE)::GetProcAddress(ghNPClientDLL, "NP_GetSignature");

		SIGNATUREDATA pSignature;
		SIGNATUREDATA verifySignature;
		// init the signatures
		strcpy_s(verifySignature.DllSignature, "precise head tracking\n put your head into the game\n now go look around\n\n Copyright EyeControl Technologies");
		strcpy_s(verifySignature.AppSignature, "hardware camera\n software processing data\n track user movement\n\n Copyright EyeControl Technologies");
		// query the dll and compare the results
		auto vresult = TrackIR::GetSignature(&pSignature);
		if (vresult == NPResult::OK) {
			if ((strcmp(verifySignature.DllSignature,pSignature.DllSignature)==0) 
				&& (strcmp(verifySignature.AppSignature,pSignature.AppSignature)==0))
			{	
				result = NPResult::OK;	

				// Get addresses of all exported functions
				gpfNP_RegisterWindowHandle     = (PF_NP_REGISTERWINDOWHANDLE)::GetProcAddress(ghNPClientDLL, "NP_RegisterWindowHandle");
				gpfNP_UnregisterWindowHandle   = (PF_NP_UNREGISTERWINDOWHANDLE)::GetProcAddress(ghNPClientDLL, "NP_UnregisterWindowHandle");
				gpfNP_RegisterProgramProfileID = (PF_NP_REGISTERPROGRAMPROFILEID)::GetProcAddress(ghNPClientDLL, "NP_RegisterProgramProfileID");
				gpfNP_QueryVersion             = (PF_NP_QUERYVERSION)::GetProcAddress(ghNPClientDLL, "NP_QueryVersion");
				gpfNP_RequestData              = (PF_NP_REQUESTDATA)::GetProcAddress(ghNPClientDLL, "NP_RequestData");
				gpfNP_GetData                  = (PF_NP_GETDATA)::GetProcAddress(ghNPClientDLL, "NP_GetData");
				gpfNP_StartCursor              = (PF_NP_STARTCURSOR)::GetProcAddress(ghNPClientDLL, "NP_StartCursor");
				gpfNP_StopCursor               = (PF_NP_STOPCURSOR)::GetProcAddress(ghNPClientDLL, "NP_StopCursor");
				gpfNP_ReCenter	               = (PF_NP_RECENTER)::GetProcAddress(ghNPClientDLL, "NP_ReCenter");
				gpfNP_StartDataTransmission    = (PF_NP_STARTDATATRANSMISSION)::GetProcAddress(ghNPClientDLL, "NP_StartDataTransmission");
				gpfNP_StopDataTransmission     = (PF_NP_STOPDATATRANSMISSION)::GetProcAddress(ghNPClientDLL, "NP_StopDataTransmission");
			} else {	
				result = NPResult::DLL_NOT_FOUND;	
			}
		} else {
			result = NPResult::DLL_NOT_FOUND;	
		}
	} else
		result = NPResult::DLL_NOT_FOUND;

	return result;
}

TrackIR::NPResult TrackIR::GetDLLLocation(LPTSTR pszPath) {
	if (pszPath == NULL)
		return NPResult::INVALID_ARG;

	//find path to NPClient.dll
	HKEY pKey = NULL;
	//open the registry key 
	if (::RegOpenKeyEx(HKEY_CURRENT_USER,
		L"Software\\NaturalPoint\\NATURALPOINT\\NPClient Location",
		0,
		KEY_READ,
		&pKey) != ERROR_SUCCESS)
	{
		return NPResult::DLL_NOT_FOUND;
	}

	//get the value from the key
	LPTSTR pszValue;
	DWORD dwSize;
	//first discover the size of the value
	if (RegQueryValueEx(pKey,
		L"Path",
		NULL,
		NULL,
		NULL,
		&dwSize) == ERROR_SUCCESS)
	{
		//allocate memory for the buffer for the value
		pszValue = (LPTSTR)malloc(dwSize);
		if (!pszValue)
			return NPResult::DLL_NOT_FOUND;

		//now get the value
		if (RegQueryValueEx(pKey,
			L"Path",
			NULL,
			NULL,
			(LPBYTE)pszValue,
			&dwSize) == ERROR_SUCCESS)
		{
			//everything worked
			::RegCloseKey(pKey);
			lstrcpy(pszPath, pszValue);
			free(pszValue);

			return NPResult::OK;

		} else {
			free(pszValue);
			return NPResult::DLL_NOT_FOUND;
		}

	}

	::RegCloseKey(pKey);
	lstrcpy(pszPath, L"Error");
	return NPResult::DLL_NOT_FOUND;
}

TrackIR::NPResult TrackIR::Initialize(HWND hWnd) {
	auto path = reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t) * 512));
	if (GetDLLLocation(path) != NPResult::OK) {
		free(path);
		return NPResult::DLL_NOT_FOUND;
	}

	if (Init(path) != NPResult::OK) {
		free(path);
		return NPResult::DLL_NOT_FOUND;
	}

	free(path);
	if (RegisterWindowHandle(hWnd) != NPResult::OK)
		return NPResult::INVALID_ARG;

	// Register with the developer ID
	if (RegisterProgramProfileID(1000) != NPResult::OK)
		return NPResult::INVALID_ARG;

	if (StartDataTransmission() != NPResult::OK)
		return NPResult::NO_DATA;

	running = true;
	return NPResult::OK;
}

void TrackIR::Shutdown() {
	StopDataTransmission();
	UnregisterWindowHandle();
	running = false;
}

bool TrackIR::IsRunning() noexcept {
	return running;
}

TrackIR::TrackingSnapshot TrackIR::GetTrackingData() noexcept {
	static TrackingSnapshot snap;

	TRACKIRDATA tid;
	if (GetData(&tid) != NPResult::OK)
		return snap;

	if (tid.wNPStatus != NPSTATUS_REMOTEACTIVE)
		return snap;

	static uint32_t frameSig = 0;
	if (frameSig == tid.wPFrameSignature)
		return snap;
	frameSig = tid.wPFrameSignature;

	snap.pos = { tid.fNPX, tid.fNPY, tid.fNPZ };
	snap.rot = {
		tid.fNPPitch * (MaxRotation / NPMaxValue),
		tid.fNPYaw * (MaxRotation / NPMaxValue),
		tid.fNPRoll * (MaxRotation / NPMaxValue)
	};

	return snap;
}
#endif