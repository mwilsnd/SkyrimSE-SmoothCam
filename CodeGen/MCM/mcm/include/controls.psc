#constexpr_struct SliderSetting [
	real int ref = 0
	float min = -100.0
	float max = 100.0
	float interval = 1.0
	float defaultValue = 0.0
	mangle string settingName = ""
	string displayName = ""
	string desc = ""
	string displayFormat = "{1}"
	string page = ""

	MACRO implControl = [
		this->ref = AddSliderOption(this->displayName, SmoothCam_GetFloatConfig(this->settingName), this->displayFormat)
	]

	MACRO implOpenHandler = [
		SetSliderDialogStartValue(SmoothCam_GetFloatConfig(this->settingName))
		SetSliderDialogDefaultValue(this->defaultValue)
		SetSliderDialogRange(this->min, this->max)
		SetSliderDialogInterval(this->interval)
	]

	MACRO implAcceptHandler = [
		SetSliderOptionValue(a_option, a_value, this->displayFormat)
		SmoothCam_SetFloatConfig(this->settingName, a_value)
	]

	MACRO implDesc = [
		SetInfoText(this->desc)
	]
]
#constexpr_struct ToggleSetting [
	real int ref = 0
	mangle string settingName = ""
	string displayName = ""
	string desc = ""
	string page = ""

	MACRO implControl = [
		this->ref = AddToggleOption(this->displayName, SmoothCam_GetBoolConfig(this->settingName))
	]

	MACRO implSelectHandler = [
		SmoothCam_SetBoolConfig(this->settingName, !SmoothCam_GetBoolConfig(this->settingName))
		SetToggleOptionValue(a_option, SmoothCam_GetBoolConfig(this->settingName))
	]

	MACRO implDesc = [
		SetInfoText(this->desc)
	]
]
#constexpr_struct ResetSetting [
	real int ref = 0
	string displayName = ""
	string desc = ""
	string page = ""

	MACRO implControl = [
		this->ref = AddToggleOption(this->displayName, false)
	]

	MACRO implSelectHandler = [
		if (ShowMessage("Are you sure? This will reset all settings to their default values."))
			SmoothCam_ResetConfig()
			ShowMessage("Settings reset.")
			ForcePageReset()
		endIf
	]

	MACRO implDesc = [
		SetInfoText(this->desc)
	]
]
#constexpr_struct ResetCrosshairSetting [
	real int ref = 0
	string displayName = ""
	string desc = ""
	string page = ""

	MACRO implControl = [
		this->ref = AddToggleOption(this->displayName, false)
	]

	MACRO implSelectHandler = [
		SmoothCam_ResetCrosshair()
		ShowMessage("Crosshair reset.")
		ForcePageReset()
	]

	MACRO implDesc = [
		SetInfoText(this->desc)
	]
]
#constexpr_struct FixStateSetting [
	real int ref = 0
	string displayName = ""
	string desc = ""
	string page = ""

	MACRO implControl = [
		this->ref = AddToggleOption(this->displayName, false)
	]

	MACRO implSelectHandler = [
		if (ShowMessage("Are you sure? You should only do this if the camera is stuck when you should be in normal thirdperson (Not on horseback or a dragon). Doing otherwise can cause issues. This is intended to fix the bug where camera rotation becomes locked."))
			SmoothCam_FixCameraState()
			ShowMessage("Camera state forced back to thirdperson.")
		endIf
	]

	MACRO implDesc = [
		SetInfoText(this->desc)
	]
]
#constexpr_struct ListSetting [
	real int ref = 0
	mangle string settingName = ""
	string displayName = ""
	string desc = ""
	literal arrayType = PLACEHOLDER
	string page = ""

	MACRO implControl = [
		this->ref = AddMenuOption(this->displayName, this->arrayType[GetCurrentArrayIndex(this->settingName, this->arrayType)])
	]

	MACRO implOpenHandler = [
		SetMenuDialogStartIndex(GetCurrentArrayIndex(this->settingName, this->arrayType))
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(this->arrayType)
	]

	MACRO implAcceptHandler = [
		SetMenuOptionValue(a_option, this->arrayType[a_index])
		SmoothCam_SetStringConfig(this->settingName, this->arrayType[a_index])
	]

	MACRO implDesc = [
		SetInfoText(this->desc)
	]
]
#constexpr_struct SavePresetSetting [
	real int ref = 0
	int presetIndex = 0
	string displayName = ""
	string desc = "Save your current settings to this preset slot"
	string page = ""

	MACRO implControl = [
		this->ref = AddInputOption(this->displayName + " " + SmoothCam_GetPresetNameAtIndex(this->presetIndex), SmoothCam_GetPresetNameAtIndex(this->presetIndex))
	]

	MACRO implOpenHandler = [
		string value = SmoothCam_GetPresetNameAtIndex(this->presetIndex)
		if (value == "Empty")
			SetInputDialogStartText("Enter a preset name")
		else
			SetInputDialogStartText(value)
		endIf
	]

	MACRO implAcceptHandler = [
		if (SmoothCam_SaveAsPreset(this->presetIndex, a_input) == "")
			ShowMessage("Preset saved.", false)
		else
			ShowMessage("Failed to save preset!", false)
		endIf
		ForcePageReset()
	]

	MACRO implDesc = [
		SetInfoText(this->desc)
	]
]
#constexpr_struct LoadPresetSetting [
	real int ref = 0
	int presetIndex = 0
	string displayName = ""
	string desc = "Load this preset"
	string page = ""

	MACRO implControl = [
		this->ref = AddToggleOption(this->displayName + " " + SmoothCam_GetPresetNameAtIndex(this->presetIndex), false)
	]

	MACRO implSelectHandler = [
		if (!SmoothCam_LoadPreset(this->presetIndex))
			ShowMessage("Failed to load preset! Have you saved this slot yet?", false)
		else
			ShowMessage("Preset loaded.", false)
			ForcePageReset()
		endIf
	]

	MACRO implDesc = [
		SetInfoText(this->desc)
	]
]
#constexpr_struct KeyBindSetting [
	real int ref = 0
	mangle string settingName = ""
	string displayName = ""
	string desc = ""
	string page = ""

	MACRO implControl = [
		this->ref = AddKeyMapOption(this->displayName, SmoothCam_GetIntConfig(this->settingName))
	]

	MACRO implSelectHandler = [
		if (StringUtil.GetLength(a_conflictControl) == 0)
			SmoothCam_SetIntConfig(this->settingName, a_keyCode)
			SetKeyMapOptionValue(this->ref, a_keyCode)
		else
			if (ShowMessage(a_conflictControl + " conflicts with another control assigned to " + a_conflictName + ".\nAre you sure you want to assign this control?"))
				SmoothCam_SetIntConfig(this->settingName, a_keyCode)
				SetKeyMapOptionValue(this->ref, a_keyCode)
			endIf
		endIf
	]

	MACRO implDesc = [
		SetInfoText(this->desc)
	]
]

event OnOptionSelect(int a_option)
	#StructInvokeSwitchIfEquals(
		a_option, ref,
		activePage, page,
		implSelectHandler,
		[
			ToggleSetting # ImplsOf,
			ResetSetting # ImplsOf,
			ResetCrosshairSetting # ImplsOf,
			FixStateSetting # ImplsOf,
			LoadPresetSetting # ImplsOf
		]
	)
endEvent

event OnOptionSliderOpen(int a_option)
	#StructInvokeSwitchIfEquals(
		a_option, ref,
		activePage, page,
		implOpenHandler,
		[SliderSetting # ImplsOf]
	)
endEvent

event OnOptionSliderAccept(int a_option, float a_value)
	#StructInvokeSwitchIfEquals(
		a_option, ref,
		activePage, page,
		implAcceptHandler,
		[SliderSetting # ImplsOf]
	)
endEvent

event OnOptionMenuOpen(int a_option)
	#StructInvokeSwitchIfEquals(
		a_option, ref,
		activePage, page,
		implOpenHandler,
		[ListSetting # ImplsOf]
	)
endEvent

event OnOptionMenuAccept(int a_option, int a_index)
	#StructInvokeSwitchIfEquals(
		a_option, ref,
		activePage, page,
		implAcceptHandler,
		[ListSetting # ImplsOf]
	)
endEvent

event OnOptionInputOpen(int a_option)
	#StructInvokeSwitchIfEquals(
		a_option, ref,
		activePage, page,
		implOpenHandler,
		[SavePresetSetting # ImplsOf]
	)
endEvent

event OnOptionInputAccept(int a_option, string a_input)
	#StructInvokeSwitchIfEquals(
		a_option, ref,
		activePage, page,
		implAcceptHandler,
		[SavePresetSetting # ImplsOf]
	)
endEvent

event OnOptionKeyMapChange(int a_option, int a_keyCode, string a_conflictControl, string a_conflictName)
	#StructInvokeSwitchIfEquals(
		a_option, ref,
		activePage, page,
		implSelectHandler,
		[KeyBindSetting # ImplsOf]
	)
endEvent

event OnOptionHighlight(int a_option)
	#StructInvokeSwitchIfEquals(
		a_option, ref,
		activePage, page,
		implDesc,
		[
			SliderSetting # ImplsOf,
			ToggleSetting # ImplsOf,
			ResetSetting # ImplsOf,
			ResetCrosshairSetting # ImplsOf,
			FixStateSetting # ImplsOf,
			ListSetting # ImplsOf,
			SavePresetSetting # ImplsOf,
			LoadPresetSetting # ImplsOf,
			KeyBindSetting # ImplsOf
		]
	)
endEvent