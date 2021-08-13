Function SmoothCam_SetIntConfig(string member, int value) global native
Function SmoothCam_SetStringConfig(string member, string value) global native
Function SmoothCam_SetBoolConfig(string member, bool value) global native
Function SmoothCam_SetFloatConfig(string member, float value) global native
Function SmoothCam_ResetConfig() global native
Function SmoothCam_ResetCrosshair() global native
Function SmoothCam_FixCameraState() global native
int Function SmoothCam_GetIntConfig(string member) global native
string Function SmoothCam_GetStringConfig(string member) global native
bool Function SmoothCam_GetBoolConfig(string member) global native
float Function SmoothCam_GetFloatConfig(string member) global native
string Function SmoothCam_SaveAsPreset(int index, string name) global native
bool Function SmoothCam_LoadPreset(int index) global native
string Function SmoothCam_GetPresetNameAtIndex(int index) global native
string Function SmoothCam_IsModDetected(int modID) global native
int Function SmoothCam_NumAPIConsumers() global native
string Function SmoothCam_GetAPIConsumerName(int index) global native