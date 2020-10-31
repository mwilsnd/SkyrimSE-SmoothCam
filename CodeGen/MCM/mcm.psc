ScriptName SmoothCamMCM extends SKI_ConfigBase
Import SKSE
; I hate everything about this
; This used to be way worse until I wrote the code generation tools

string[] interpMethods
string[] presets
string[] crosshairTypes
string activePage

Function SmoothCam_SetIntConfig(string member, int value) global native
Function SmoothCam_SetStringConfig(string member, string value) global native
Function SmoothCam_SetBoolConfig(string member, bool value) global native
Function SmoothCam_SetFloatConfig(string member, float value) global native
Function SmoothCam_ResetConfig() global native

int Function SmoothCam_GetIntConfig(string member) global native
string Function SmoothCam_GetStringConfig(string member) global native
bool Function SmoothCam_GetBoolConfig(string member) global native
float Function SmoothCam_GetFloatConfig(string member) global native

string Function SmoothCam_SaveAsPreset(int index, string name) global native
bool Function SmoothCam_LoadPreset(int index) global native
string Function SmoothCam_GetPresetNameAtIndex(int index) global native

int Function GetCurrentArrayIndex(string setting, string[] array)
	string value = SmoothCam_GetStringConfig(setting)
	
	Int i = array.Length
	While i
		i -= 1
		if (array[i] == value)
			return i
		endIf
	endWhile

	return 0
endFunction

#constexpr_struct ScriptMeta {
	int version = 0
}

#constexpr_struct SliderSetting {
	real_int ref = 0
	float min = -100.0
	float max = 100.0
	float interval = 1.0
	float defaultValue = 0.0
	string settingName = ""
	string displayName = ""
	string desc = ""
	string displayFormat = "{1}"
	string page = ""

	MACRO implControl = {
		this->ref = AddSliderOption(this->displayName, SmoothCam_GetFloatConfig(this->settingName), this->displayFormat)
	}

	MACRO implOpenHandler = {
		SetSliderDialogStartValue(SmoothCam_GetFloatConfig(this->settingName))
		SetSliderDialogDefaultValue(this->defaultValue)
		SetSliderDialogRange(this->min, this->max)
		SetSliderDialogInterval(this->interval)
	}

	MACRO implAcceptHandler = {
		SetSliderOptionValue(a_option, a_value, this->displayFormat)
		SmoothCam_SetFloatConfig(this->settingName, a_value)
	}

	MACRO implDesc = {
		SetInfoText(this->desc)
	}
}

#constexpr_struct ToggleSetting {
	real_int ref = 0
	string settingName = ""
	string displayName = ""
	string desc = ""
	string page = ""

	MACRO implControl = {
		this->ref = AddToggleOption(this->displayName, SmoothCam_GetBoolConfig(this->settingName))
	}

	MACRO implSelectHandler = {
		SmoothCam_SetBoolConfig(this->settingName, !SmoothCam_GetBoolConfig(this->settingName))
		SetToggleOptionValue(a_option, SmoothCam_GetBoolConfig(this->settingName))
	}

	MACRO implDesc = {
		SetInfoText(this->desc)
	}
}

#constexpr_struct ResetSetting {
	real_int ref = 0
	string displayName = ""
	string desc = ""
	string page = ""

	MACRO implControl = {
		this->ref = AddToggleOption(this->displayName, false)
	}

	MACRO implSelectHandler = {
		if (ShowMessage("Are you sure? This will reset all settings to their default values."))
			SmoothCam_ResetConfig()
			ShowMessage("Settings reset.")
		endIf
	}

	MACRO implDesc = {
		SetInfoText(this->desc)
	}
}

#constexpr_struct ListSetting {
	real_int ref = 0
	string settingName = ""
	string displayName = ""
	string desc = ""
	LITERAL arrayType = 0
	string page = ""

	MACRO implControl = {
		this->ref = AddMenuOption(this->displayName, this->arrayType[GetCurrentArrayIndex(this->settingName, this->arrayType)])
	}

	MACRO implOpenHandler = {
		SetMenuDialogStartIndex(GetCurrentArrayIndex(this->settingName, this->arrayType))
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(this->arrayType)
	}

	MACRO implAcceptHandler = {
		SetMenuOptionValue(a_option, this->arrayType[a_index])
		SmoothCam_SetStringConfig(this->settingName, this->arrayType[a_index])
	}

	MACRO implDesc = {
		SetInfoText(this->desc)
	}
}

#constexpr_struct SavePresetSetting {
	real_int ref = 0
	int presetIndex = 0
	string displayName = ""
	string desc = "Save your current settings to this preset slot"
	string page = ""

	MACRO implControl = {
		this->ref = AddInputOption(this->displayName + " " + SmoothCam_GetPresetNameAtIndex(this->presetIndex), SmoothCam_GetPresetNameAtIndex(this->presetIndex))
	}

	MACRO implOpenHandler = {
		string value = SmoothCam_GetPresetNameAtIndex(this->presetIndex)
		if (value == "Empty")
			SetInputDialogStartText("Enter a preset name")
		else
			SetInputDialogStartText(value)
		endIf
	}

	MACRO implAcceptHandler = {
		if (SmoothCam_SaveAsPreset(this->presetIndex, a_input) == "")
			ShowMessage("Preset saved.", false)
		else
			ShowMessage("Failed to save preset!", false)
		endIf
		ForcePageReset()
	}

	MACRO implDesc = {
		SetInfoText(this->desc)
	}
}

#constexpr_struct LoadPresetSetting {
	real_int ref = 0
	int presetIndex = 0
	string displayName = ""
	string desc = "Load this preset"
	string page = ""

	MACRO implControl = {
		this->ref = AddToggleOption(this->displayName + " " + SmoothCam_GetPresetNameAtIndex(this->presetIndex), false)
	}

	MACRO implSelectHandler = {
		if (!SmoothCam_LoadPreset(this->presetIndex))
			ShowMessage("Failed to load preset! Have you saved this slot yet?", false)
		else
			ShowMessage("Preset loaded.", false)
			ForcePageReset()
		endIf
	}

	MACRO implDesc = {
		SetInfoText(this->desc)
	}
}

#constexpr_struct KeyBindSetting {
	real_int ref = 0
	string settingName = ""
	string displayName = ""
	string desc = ""
	string page = ""

	MACRO implControl = {
		this->ref = AddKeyMapOption(this->displayName, SmoothCam_GetIntConfig(this->settingName))
	}

	MACRO implSelectHandler = {
		if (StringUtil.GetLength(a_conflictControl) == 0)
			SmoothCam_SetIntConfig(this->settingName, a_keyCode)
			SetKeyMapOptionValue(this->ref, a_keyCode)
		else
			if (ShowMessage(a_conflictControl + " conflicts with another control assigned to " + a_conflictName + ".\nAre you sure you want to assign this control?"))
				SmoothCam_SetIntConfig(this->settingName, a_keyCode)
				SetKeyMapOptionValue(this->ref, a_keyCode)
			endIf
		endIf
	}

	MACRO implDesc = {
		SetInfoText(this->desc)
	}
}

ScriptMeta scriptMetaInfo -> {
	version: 12
}

; Presets
SavePresetSetting savePresetSlot1 -> {
	presetIndex: 0
	displayName: "Slot 1:"
	page: " Presets"
}
SavePresetSetting savePresetSlot2 -> {
	presetIndex: 1
	displayName: "Slot 2:"
	page: " Presets"
}
SavePresetSetting savePresetSlot3 -> {
	presetIndex: 2
	displayName: "Slot 3:"
	page: " Presets"
}
SavePresetSetting savePresetSlot4 -> {
	presetIndex: 3
	displayName: "Slot 4:"
	page: " Presets"
}
SavePresetSetting savePresetSlot5 -> {
	presetIndex: 4
	displayName: "Slot 5:"
	page: " Presets"
}
SavePresetSetting savePresetSlot6 -> {
	presetIndex: 5
	displayName: "Slot 6:"
	page: " Presets"
}

LoadPresetSetting loadPresetSlot1 -> {
	presetIndex: 0
	displayName: "Slot 1:"
	page: " Presets"
}
LoadPresetSetting loadPresetSlot2 -> {
	presetIndex: 1
	displayName: "Slot 2:"
	page: " Presets"
}
LoadPresetSetting loadPresetSlot3 -> {
	presetIndex: 2
	displayName: "Slot 3:"
	page: " Presets"
}
LoadPresetSetting loadPresetSlot4 -> {
	presetIndex: 3
	displayName: "Slot 4:"
	page: " Presets"
}
LoadPresetSetting loadPresetSlot5 -> {
	presetIndex: 4
	displayName: "Slot 5:"
	page: " Presets"
}
LoadPresetSetting loadPresetSlot6 -> {
	presetIndex: 5
	displayName: "Slot 6:"
	page: " Presets"
}

; Compat
ToggleSetting icCompat -> {
	settingName: "ICCompat"
	displayName: "Improved Camera"
	desc: "Enable compat fixes for Improved Camera."
	page: " Compatibility"
}
ToggleSetting accCompat -> {
	settingName: "ACCCompat"
	displayName: "Alternate Conversation Camera"
	desc: "Enable compat fixes for Alternate Conversation Camera."
	page: " Compatibility"
}
ToggleSetting ifpvCompat -> {
	settingName: "IFPVCompat"
	displayName: "Immersive First Person View"
	desc: "Enable compat fixes for Improved First Person View."
	page: " Compatibility"
}
ToggleSetting agoCompat -> {
	settingName: "AGOCompat"
	displayName: "Archery Gameplay Overhaul"
	desc: "Enable compat fixes for AGO when using the 3D crosshair/prediction arc."
	page: " Compatibility"
}

; Reset
ResetSetting reset -> {
	displayName: "Reset All Settings"
	desc: "Set all settings back to their defual values."
	page: " Following"
}

; Following
ToggleSetting interpEnabled -> {
	settingName: "InterpolationEnabled"
	displayName: "Interpolation Enabled"
	desc: "Enable camera smoothing."
	page: " Following"
}
ToggleSetting sepZInterpEnabled -> {
	settingName: "SeparateZInterpEnabled"
	displayName: "Separate Z Interpolation Enabled"
	desc: "Enable the separate Z smoothing settings for smoothing camera height differently."
	page: " Following"
}
ToggleSetting sepLocalInterpEnabled -> {
	settingName: "SeparateLocalInterpolation"
	displayName: "Local-Space Interpolation Enabled"
	desc: "Enable separate local-space camera smoothing (Camera rotation)."
	page: " Following"
}
ToggleSetting offsetInterpEnabled -> {
	settingName: "OffsetTransitionEnabled"
	displayName: "Offset Interpolation Enabled"
	desc: "Enable smoothing of camera offset state transitions."
	page: " Following"
}
ToggleSetting zoomInterpEnabled -> {
	settingName: "ZoomTransitionEnabled"
	displayName: "Zoom Interpolation Enabled"
	desc: "Enable smoothing of camera zoom state transitions."
	page: " Following"
}
ToggleSetting fovInterpEnabled -> {
	settingName: "FOVTransitionEnabled"
	displayName: "FOV Interpolation Enabled"
	desc: "Enable smoothing of camera FOV state transitions."
	page: " Following"
}
ToggleSetting disableDeltaTime -> {
	settingName: "DisableDeltaTime"
	displayName: "Disable Delta Time Factoring"
	desc: "Remove time from interpolation math. May result in less jitter but can cause speed to vary with frame rate."
	page: " Following"
}
ToggleSetting cameraDistanceClampXEnable -> {
	settingName: "CameraDistanceClampXEnable"
	displayName: "Enable X Distance Clamp"
	desc: "Clamp the maximum distance the camera may move away from the target position along the X (side) axis."
	page: " Following"
}
ToggleSetting cameraDistanceClampYEnable -> {
	settingName: "CameraDistanceClampYEnable"
	displayName: "Enable Y Distance Clamp"
	desc: "Clamp the maximum distance the camera may move away from the target position along the Y (forward) axis."
	page: " Following"
}
ToggleSetting cameraDistanceClampZEnable -> {
	settingName: "CameraDistanceClampZEnable"
	displayName: "Enable Z Distance Clamp"
	desc: "Clamp the maximum distance the camera may move away from the target position along the Z (up) axis."
	page: " Following"
}
ToggleSetting swapDistanceClampXAxis -> {
	settingName: "ShoulderSwapXClamping"
	displayName: "Also Swap X Axis Clamping"
	desc: "When shoulder swapping, will also swap the distance clamping X axis range."
	page: " Following"
}

ListSetting interpMethod -> {
	settingName: "InterpolationMethod"
	displayName: "Interpolation Method"
	desc: "The scalar method to use for camera smoothing."
	arrayType: interpMethods
	page: " Following"
}
ListSetting sepZInterpMethod -> {
	settingName: "SeparateZInterpMethod"
	displayName: "Sep. Z Interpolation Method"
	desc: "The scalar method to use for smoothing the camera height (If enabled)."
	arrayType: interpMethods
	page: " Following"
}
ListSetting sepLocalInterpMethod -> {
	settingName: "SepLocalInterpMethod"
	displayName: "Local-Space Interpolation Method"
	desc: "The scalar method to use for local-space smoothing (If enabled)."
	arrayType: interpMethods
	page: " Following"
}
ListSetting offsetInterpMethod -> {
	settingName: "OffsetTransitionMethod"
	displayName: "Offset Interpolation Method"
	desc: "The scalar method to use for offset transition smoothing (If enabled)."
	arrayType: interpMethods
	page: " Following"
}
ListSetting zoomInterpMethod -> {
	settingName: "ZoomTransitionMethod"
	displayName: "Zoom Interpolation Method"
	desc: "The scalar method to use for zoom transition smoothing (If enabled)."
	arrayType: interpMethods
	page: " Following"
}
ListSetting fovInterpMethod -> {
	settingName: "FOVTransitionMethod"
	displayName: "FOV Interpolation Method"
	desc: "The scalar method to use for FOV transition smoothing (If enabled)."
	arrayType: interpMethods
	page: " Following"
}

SliderSetting minCameraFollowRate -> {
	settingName: "MinCameraFollowRate"
	displayName: "Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the player."
	defaultValue: 0.5
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
}
SliderSetting maxCameraFollowRate -> {
	settingName: "MaxCameraFollowRate"
	displayName: "Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the player."
	defaultValue: 0.8
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
}
SliderSetting maxSmoothingInterpDistance -> {
	settingName: "MaxSmoothingInterpDistance"
	displayName: "Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 650.0
	interval: 1.0
	min: 1.0
	max: 650.0
	displayFormat: "{0}"
	page: " Following"
}
SliderSetting minCameraFollowDistance -> {
	settingName: "MinFollowDistance"
	displayName: "Min Follow Distance"
	desc: "The closest the camera may get to the player when at the lowest zoom level."
	defaultValue: 64.0
	interval: 0.1
	min: 0.0
	max: 256.0
	page: " Following"
}
SliderSetting zoomMul -> {
	settingName: "ZoomMul"
	displayName: "Zoom Multiplier"
	desc: "The amount of distance to add to the camera for each zoom level."
	defaultValue: 500.0
	interval: 1.0
	min: 1.0
	max: 500.0
	page: " Following"
}
SliderSetting minSepZFollowRate -> {
	settingName: "SepZMinFollowRate"
	displayName: "Separate Z Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the player."
	defaultValue: 0.5
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
}
SliderSetting maxSepZFollowRate -> {
	settingName: "SepZMaxFollowRate"
	displayName: "Separate Z Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the player."
	defaultValue: 0.5
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
}
SliderSetting maxSepZSmoothingDistance -> {
	settingName: "SepZMaxInterpDistance"
	displayName: "Sep. Z Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 100.0
	interval: 1.0
	min: 1.0
	max: 300.0
	displayFormat: "{0}"
	page: " Following"
}
SliderSetting offsetTransitionDuration -> {
	settingName: "OffsetTransitionDuration"
	displayName: "Offset Interpolation Duration"
	desc: "The smoothing duration to use when the camera changes offsets (In seconds)."
	defaultValue: 1.0
	interval: 0.01
	min: 0.01
	max: 5.0
	displayFormat: "{2}"
	page: " Following"
}
SliderSetting zoomTransitionDuration -> {
	settingName: "ZoomTransitionDuration"
	displayName: "Zoom Interpolation Duration"
	desc: "The smoothing duration to use when the camera changes zoom distance (In seconds)."
	defaultValue: 0.2
	interval: 0.01
	min: 0.01
	max: 5.0
	displayFormat: "{2}"
	page: " Following"
}
SliderSetting fovTransitionDuration -> {
	settingName: "FOVTransitionDuration"
	displayName: "FOV Interpolation Duration"
	desc: "The smoothing duration to use when the camera changes FOV (In seconds)."
	defaultValue: 0.2
	interval: 0.01
	min: 0.01
	max: 5.0
	displayFormat: "{2}"
	page: " Following"
}
SliderSetting cameraDistanceClampXMin -> {
	settingName: "CameraDistanceClampXMin"
	displayName: "Distance Clamp X Min"
	desc: "The minimal distance the camera may get from the target position along the X axis before being clamped."
	defaultValue: -75.0
	interval: 1.0
	min: -300.0
	max: 0.0
	displayFormat: "{0}"
	page: " Following"
}
SliderSetting cameraDistanceClampXMax -> {
	settingName: "CameraDistanceClampXMax"
	displayName: "Distance Clamp X Max"
	desc: "The maximal distance the camera may get from the target position along the X axis before being clamped."
	defaultValue: 75.0
	interval: 1.0
	min: 0.0
	max: 300.0
	displayFormat: "{0}"
	page: " Following"
}
SliderSetting cameraDistanceClampYMin -> {
	settingName: "CameraDistanceClampYMin"
	displayName: "Distance Clamp Y Min"
	desc: "The minimal distance the camera may get from the target position along the Y axis before being clamped."
	defaultValue: -100.0
	interval: 1.0
	min: -500.0
	max: 0.0
	displayFormat: "{0}"
	page: " Following"
}
SliderSetting cameraDistanceClampYMax -> {
	settingName: "CameraDistanceClampYMax"
	displayName: "Distance Clamp Y Max"
	desc: "The maximal distance the camera may get from the target position along the Y axis before being clamped."
	defaultValue: 100.0
	interval: 1.0
	min: 0.0
	max: 500.0
	displayFormat: "{0}"
	page: " Following"
}
SliderSetting cameraDistanceClampZMin -> {
	settingName: "CameraDistanceClampZMin"
	displayName: "Distance Clamp Z Min"
	desc: "The minimal distance the camera may get from the target position along the Z axis before being clamped."
	defaultValue: -50.0
	interval: 1.0
	min: -300.0
	max: 0.0
	displayFormat: "{0}"
	page: " Following"
}
SliderSetting cameraDistanceClampZMax -> {
	settingName: "CameraDistanceClampZMax"
	displayName: "Distance Clamp Z Max"
	desc: "The maximal distance the camera may get from the target position along the Z axis before being clamped."
	defaultValue: 50.0
	interval: 1.0
	min: 0.0
	max: 300.0
	displayFormat: "{0}"
	page: " Following"
}
SliderSetting sepLocalSpaceInterpRate -> {
	settingName: "SepLocalInterpRate"
	displayName: "Local-Space Follow Rate"
	desc: "The smoothing rate to use for local-space smoothing."
	defaultValue: 0.5
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
}

KeyBindSetting shoulderSwapKey -> {
	settingName: "ShoulderSwapKeyCode"
	displayName: "Shoulder Swap Key"
	desc: "Inverts the current X offset of the camera"
	page: " Following"
}

; Crosshair
ToggleSetting crosshair3DBowEnabled -> {
	settingName: "Enable3DBowCrosshair"
	displayName: "3D Bow Crosshair Enabled"
	desc: "Enable the raycasted 3D crosshair when aiming with the bow."
	page: " Crosshair"
}
ToggleSetting crosshair3DMagicEnabled -> {
	settingName: "Enable3DMagicCrosshair"
	displayName: "3D Magic Crosshair Enabled"
	desc: "Enable the raycasted 3D crosshair when using magic."
	page: " Crosshair"
}
ToggleSetting crosshair3DWorldEnabled -> {
	settingName: "UseWorldCrosshair"
	displayName: "Use World-Space Crosshair"
	desc: "When your crosshair ray has hit something, use a crosshair mesh rendered in-world, not on the HUD."
	page: " Crosshair"
}
ListSetting worldCrosshairType -> {
	settingName: "WorldCrosshairType"
	displayName: "Crosshair Type"
	desc: "Select the style of world-space crosshair to use, if world-space crosshair is enabled."
	arrayType: crosshairTypes
	page: " Crosshair"
}
ToggleSetting worldCrosshairDepthTest -> {
	settingName: "WorldCrosshairDepthTest"
	displayName: "Crosshair Occlusion"
	desc: "When using the world-space crosshair, disable this option to make it draw on top of everything rather than allow other geometry to cover it."
	page: " Crosshair"
}
ToggleSetting hideCrosshairOutOfCombat -> {
	settingName: "HideCrosshairOutOfCombat"
	displayName: "Hide Non-Combat Crosshair"
	desc: "Hide the crosshair when not in combat."
	page: " Crosshair"
}
ToggleSetting hideCrosshairMeleeCombat -> {
	settingName: "HideCrosshairMeleeCombat"
	displayName: "Hide Melee Combat Crosshair"
	desc: "Hide the crosshair when in melee combat."
	page: " Crosshair"
}
ToggleSetting enableCrosshairSizeManip -> {
	settingName: "EnableCrosshairSizeManip"
	displayName: "Enable Size Manipulation"
	desc: "Changes the size of the crosshair, based on 'Min/Max Size' and 'NPC Hit Size'. Disable if this causes conflicts with other mods. May require a game restart."
	page: " Crosshair"
}
SliderSetting crosshairNPCGrowSize -> {
	settingName: "CrosshairNPCGrowSize"
	displayName: "NPC Hit Size"
	desc: "When the 3D crosshair is over an NPC, grow the size of the crosshair by this amount."
	defaultValue: 16
	interval: 1
	min: 0
	max: 32
	page: " Crosshair"
}
SliderSetting crosshairMinDistSize -> {
	settingName: "CrosshairMinDistSize"
	displayName: "Min Crosshair Size"
	desc: "Sets the size of the 3D crosshair when the player's aim ray is at the maximum distance."
	defaultValue: 16
	interval: 1
	min: 8
	max: 32
	page: " Crosshair"
}
SliderSetting crosshairMaxDistSize -> {
	settingName: "CrosshairMaxDistSize"
	displayName: "Max Crosshair Size"
	desc: "Sets the size of the 3D crosshair when the player's aim ray is at the minimum distance."
	defaultValue: 24
	interval: 1
	min: 8
	max: 32
	page: " Crosshair"
}
ToggleSetting enableArrowPrediction -> {
	settingName: "EnableArrowPrediction"
	displayName: "Enable Arrow Prediction"
	desc: "When the 3D crosshair is enabled for ranged combat, the crosshair will account for gravity when aiming with a bow."
	page: " Crosshair"
}
ToggleSetting drawArrowArc -> {
	settingName: "DrawArrowArc"
	displayName: "Draw Arrow Prediction Arc"
	desc: "When the 3D crosshair is enabled for ranged combat and 'Enable Arrow Prediction' is selected, an arc will be drawn while aiming with bows which indicates the flight path your arrow will take."
	page: " Crosshair"
}
SliderSetting maxArrowPredictionRange -> {
	settingName: "MaxArrowPredictionRange"
	displayName: "Max Arrow Prediction Distance"
	desc: "The furthest distance to allow arrow prediction, if enabled, to function."
	defaultValue: 10000
	interval: 1
	min: 500
	max: 12000
	displayFormat: "{0}"
	page: " Crosshair"
}
SliderSetting arrowArcColorR -> {
	settingName: "ArrowArcColorR"
	displayName: "Arrow Arc Color: Red"
	desc: "The amount of red coloration to add to the arrow arc."
	defaultValue: 255.0
	interval: 1
	min: 0.0
	max: 255.0
	displayFormat: "{0}"
	page: " Crosshair"
}
SliderSetting arrowArcColorG -> {
	settingName: "ArrowArcColorG"
	displayName: "Arrow Arc Color: Green"
	desc: "The amount of green coloration to add to the arrow arc."
	defaultValue: 255.0
	interval: 1
	min: 0.0
	max: 255.0
	displayFormat: "{0}"
	page: " Crosshair"
}
SliderSetting arrowArcColorB -> {
	settingName: "ArrowArcColorB"
	displayName: "Arrow Arc Color: Blue"
	desc: "The amount of blue coloration to add to the arrow arc."
	defaultValue: 255.0
	interval: 1
	min: 0.0
	max: 255.0
	displayFormat: "{0}"
	page: " Crosshair"
}
SliderSetting arrowArcColorA -> {
	settingName: "ArrowArcColorA"
	displayName: "Arrow Arc Color: Transparency"
	desc: "The amount of transparency coloration to add to the arrow arc."
	defaultValue: 127.0
	interval: 1
	min: 0.0
	max: 255.0
	displayFormat: "{0}"
	page: " Crosshair"
}


; Standing
DECLARE_OFFSET_GROUP_CONTROLS(Standing, " Standing")
; Walking
DECLARE_OFFSET_GROUP_CONTROLS(Walking, " Walking")
; Running
DECLARE_OFFSET_GROUP_CONTROLS(Running, " Running")
; Sprinting
DECLARE_OFFSET_GROUP_CONTROLS(Sprinting, " Sprinting")
; Sneaking
DECLARE_OFFSET_GROUP_CONTROLS(Sneaking, " Sneaking")
; Swimming
DECLARE_OFFSET_GROUP_CONTROLS(Swimming, " Swimming", NoMelee, NoRanged, NoMagic)
; Sitting
DECLARE_OFFSET_GROUP_CONTROLS(Sitting, " Sitting", NoMelee, NoRanged, NoMagic)
; Horseback
DECLARE_OFFSET_GROUP_CONTROLS(Horseback, " Horseback")
; Dragon
DECLARE_OFFSET_GROUP_CONTROLS(Dragon, " Dragon", NoMelee, NoRanged, NoMagic)
; Vampire Lord
DECLARE_OFFSET_GROUP_CONTROLS(VampireLord, " Vampire Lord", NoRanged)
; Werewolf
DECLARE_OFFSET_GROUP_CONTROLS(Werewolf, " Werewolf", NoRanged, NoMagic)
; Group edit
DECLARE_OFFSET_GROUP_CONTROLS(Group, " Group Edit", NoInterpToggles)

; Bow aiming
SliderSetting bowaim_sideOffset -> {
	settingName: "Bowaim:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
	page: " Bow Aiming"
}
SliderSetting bowaim_upOffset -> {
	settingName: "Bowaim:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
	page: " Bow Aiming"
}
SliderSetting bowaim_zoomOffset -> {
	settingName: "Bowaim:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
	page: " Bow Aiming"
}
SliderSetting bowaim_fovOffset -> {
	settingName: "Bowaim:FOVOffset"
	displayName: "FOV Offset"
	desc: "The amount to offset the camera FOV by. Note this will be clamped to a lower bound of 10 and an upper bound of 170."
	min: -120.0
	max: 120.0
	page: " Bow Aiming"
}
SliderSetting bowaim_sideOffsetHorseback -> {
	settingName: "BowaimHorse:SideOffset"
	displayName: "Horseback Side Offset"
	desc: "The amount to move the camera to the right when on horseback."
	defaultValue: 25.0
	page: " Bow Aiming"
}
SliderSetting bowaim_upOffsetHorseback -> {
	settingName: "BowaimHorse:UpOffset"
	displayName: "Horseback Up Offset"
	desc: "The amount to move the camera up when on horseback."
	page: " Bow Aiming"
}
SliderSetting bowaim_zoomOffsetHorseback -> {
	settingName: "BowaimHorse:ZoomOffset"
	displayName: "Horseback Zoom Offset"
	desc: "The amount to offset the camera zoom by when on horseback."
	min: -200.0
	max: 200.0
	page: " Bow Aiming"
}
SliderSetting bowaim_fovOffsetHorseback -> {
	settingName: "BowaimHorse:FOVOffset"
	displayName: "Horseback FOV Offset"
	desc: "The amount to offset the camera FOV by when on horseback. Note this will be clamped to a lower bound of 10 and an upper bound of 170."
	min: -120.0
	max: 120.0
	page: " Bow Aiming"
}
SliderSetting bowaim_sideOffsetSneaking -> {
	settingName: "BowaimSneak:SideOffset"
	displayName: "Sneaking Side Offset"
	desc: "The amount to move the camera to the right when sneaking."
	defaultValue: 25.0
	page: " Bow Aiming"
}
SliderSetting bowaim_upOffsetSneaking -> {
	settingName: "BowaimSneak:UpOffset"
	displayName: "Sneaking Up Offset"
	desc: "The amount to move the camera up when sneaking."
	page: " Bow Aiming"
}
SliderSetting bowaim_zoomOffsetSneaking -> {
	settingName: "BowaimSneak:ZoomOffset"
	displayName: "Sneaking Zoom Offset"
	desc: "The amount to offset the camera zoom by when sneaking."
	min: -200.0
	max: 200.0
	page: " Bow Aiming"
}
SliderSetting bowaim_fovOffsetSneaking -> {
	settingName: "BowaimSneak:FOVOffset"
	displayName: "Sneaking FOV Offset"
	desc: "The amount to offset the camera FOV by when sneaking. Note this will be clamped to a lower bound of 10 and an upper bound of 170."
	min: -120.0
	max: 120.0
	page: " Bow Aiming"
}
ToggleSetting bowaim_interp -> {
	settingName: "InterpBowAim"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
	page: " Bow Aiming"
}
ToggleSetting bowaim_interpHorseback -> {
	settingName: "InterpBowAimHorseback"
	displayName: "Enable Horseback Interpolation"
	desc: "Enables interpolation in this state."
	page: " Bow Aiming"
}
ToggleSetting bowaim_interpSneaking -> {
	settingName: "InterpBowAimSneaking"
	displayName: "Enable Sneaking Interpolation"
	desc: "Enables interpolation in this state."
	page: " Bow Aiming"
}

int Function GetVersion()
	return scriptMetaInfo.version
endFunction

event OnConfigInit()
	Pages = new string[] -> {
		" Info", " Compatibility", " Following", " Crosshair", " Standing",
		" Walking", " Running", " Sprinting", " Sneaking",
		" Swimming", " Bow Aiming", " Sitting", " Horseback",
		" Dragon", " Vampire Lord", " Werewolf", " Group Edit", " Presets"
	}
	interpMethods = new string[] -> {
		"linear", "quadraticEaseIn", "quadraticEaseOut",
		"quadraticEaseInOut", "cubicEaseIn", "cubicEaseOut",
		"cubicEaseInOut", "quarticEaseIn", "quarticEaseOut",
		"quarticEaseInOut", "quinticEaseIn", "quinticEaseOut",
		"quinticEaseInOut", "sineEaseIn", "sineEaseOut",
		"sineEaseInOut", "circularEaseIn", "circularEaseOut",
		"circularEaseInOut", "exponentialEaseIn", "exponentialEaseOut",
		"exponentialEaseInOut"
	}
	crosshairTypes = new string[] -> {
		"Skyrim", "Dot"
	}
endEvent

event OnVersionUpdate(int version)
	OnConfigInit()
endEvent

event OnPageReset(string a_page)
	SetCursorFillMode(TOP_TO_BOTTOM)
	activePage = a_page

	if (a_page == " Info")
		int version_T = AddTextOption("DLL Version", GetPluginVersion("SmoothCam"), OPTION_FLAG_DISABLED)
		int s_version_T = AddTextOption("MCM Script Version", scriptMetaInfo.version, OPTION_FLAG_DISABLED)
		int hasDXContext = AddTextOption("D3D11 Hooked", SmoothCam_GetBoolConfig("D3DHooked"), OPTION_FLAG_DISABLED)

	elseIf (a_page == " Compatibility")
		AddHeaderOption("General")
		accCompat->!implControl
		icCompat->!implControl
		ifpvCompat->!implControl
		agoCompat->!implControl

	elseIf (a_page == " Following")
		AddHeaderOption("Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			interpMethod, interpEnabled, minCameraFollowDistance, minCameraFollowRate, maxCameraFollowRate, maxSmoothingInterpDistance
		})

		AddHeaderOption("Separate Z Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			sepZInterpMethod, sepZInterpEnabled, minSepZFollowRate, maxSepZFollowRate, maxSepZSmoothingDistance
		})

		AddHeaderOption("Local-Space Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			sepLocalInterpEnabled, sepLocalInterpMethod, sepLocalSpaceInterpRate
		})

		AddHeaderOption("Offset Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			offsetInterpEnabled, offsetInterpMethod, offsetTransitionDuration
		})

		AddHeaderOption("Zoom Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			zoomInterpEnabled, zoomInterpMethod, zoomTransitionDuration
		})

		AddHeaderOption("FOV Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			fovInterpEnabled, fovInterpMethod, fovTransitionDuration
		})

		SetCursorPosition(1)
		AddHeaderOption("Distance Clamping")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			cameraDistanceClampXEnable, cameraDistanceClampXMin, cameraDistanceClampXMax,
			cameraDistanceClampYEnable, cameraDistanceClampYMin, cameraDistanceClampYMax,
			cameraDistanceClampZEnable, cameraDistanceClampZMin, cameraDistanceClampZMax
		})

		AddHeaderOption("Misc")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			shoulderSwapKey, swapDistanceClampXAxis, zoomMul, disableDeltaTime, reset
		})

	elseIf (a_page == " Crosshair")
		AddHeaderOption("3D Crosshair Settings")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			crosshair3DBowEnabled, crosshair3DMagicEnabled,
			crosshair3DWorldEnabled, worldCrosshairType, worldCrosshairDepthTest,
			enableCrosshairSizeManip, crosshairMinDistSize,
			crosshairMaxDistSize, crosshairNPCGrowSize
		})

		AddHeaderOption("Crosshair Hiding")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			hideCrosshairOutOfCombat, hideCrosshairMeleeCombat,
		})

		SetCursorPosition(1)
		AddHeaderOption("Archery Features")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			enableArrowPrediction, drawArrowArc, maxArrowPredictionRange,
			arrowArcColorR, arrowArcColorG, arrowArcColorB,
			arrowArcColorA,
		})

	elseIf (a_page == " Standing")
		IMPL_OFFSET_GROUP_PAGE(Standing)

	elseIf (a_page == " Walking")
		IMPL_OFFSET_GROUP_PAGE(Walking)

	elseIf (a_page == " Running")
		IMPL_OFFSET_GROUP_PAGE(Running)

	elseIf (a_page == " Sprinting")
		IMPL_OFFSET_GROUP_PAGE(Sprinting)

	elseIf (a_page == " Sneaking")
		IMPL_OFFSET_GROUP_PAGE(Sneaking)

	elseIf (a_page == " Swimming")
		IMPL_OFFSET_GROUP_PAGE(Swimming)

	elseIf (a_page == " Bow Aiming")
		AddHeaderOption("Bow Aiming Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			bowaim_sideOffset,
			bowaim_upOffset,
			bowaim_zoomOffset,
			bowaim_fovOffset,
			bowaim_sideOffsetHorseback,
			bowaim_upOffsetHorseback,
			bowaim_zoomOffsetHorseback,
			bowaim_fovOffsetHorseback,
			bowaim_sideOffsetSneaking,
			bowaim_upOffsetSneaking,
			bowaim_zoomOffsetSneaking,
			bowaim_fovOffsetSneaking
		})

		SetCursorPosition(1)
		AddHeaderOption("Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			bowaim_interp,
			bowaim_interpHorseback,
			bowaim_interpSneaking
		})

	elseIf (a_page == " Sitting")
		IMPL_OFFSET_GROUP_PAGE(Sitting)

	elseIf (a_page == " Horseback")
		IMPL_OFFSET_GROUP_PAGE(Horseback)

	elseIf (a_page == " Dragon")
		IMPL_OFFSET_GROUP_PAGE(Dragon)

	elseIf (a_page == " Vampire Lord")
		IMPL_OFFSET_GROUP_PAGE(VampireLord)

	elseIf (a_page == " Werewolf")
		IMPL_OFFSET_GROUP_PAGE(Werewolf)

	elseIf (a_page == " Group Edit")
		AddHeaderOption("Edit All Offset Groups")
		IMPL_OFFSET_GROUP_PAGE(Group, NoSliderHeader, NoInterpToggles)

	elseIf (a_page == " Presets")
		AddHeaderOption("Save Preset")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			savePresetSlot1,
			savePresetSlot2,
			savePresetSlot3,
			savePresetSlot4,
			savePresetSlot5,
			savePresetSlot6
		})

		SetCursorPosition(1)
		AddHeaderOption("Load Preset")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			loadPresetSlot1,
			loadPresetSlot2,
			loadPresetSlot3,
			loadPresetSlot4,
			loadPresetSlot5,
			loadPresetSlot6
		})

	endIf
endEvent

event OnOptionSelect(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implSelectHandler, {
		IMPL_ALL_IMPLS_OF_STRUCT(ToggleSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(ResetSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(LoadPresetSetting)
	}, activePage)
endEvent

event OnOptionSliderOpen(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(
		a_option, ref, implOpenHandler,
		{IMPL_ALL_IMPLS_OF_STRUCT(SliderSetting)},
		activePage
	)
endEvent

event OnOptionSliderAccept(int a_option, float a_value)
	IMPL_IFCHAIN_MACRO_INVOKE(
		a_option, ref, implAcceptHandler,
		{IMPL_ALL_IMPLS_OF_STRUCT(SliderSetting)}, activePage
	)
endEvent

event OnOptionMenuOpen(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(
		a_option, ref, implOpenHandler,
		{IMPL_ALL_IMPLS_OF_STRUCT(ListSetting)}, activePage
	)
endEvent

event OnOptionMenuAccept(int a_option, int a_index)
	IMPL_IFCHAIN_MACRO_INVOKE(
		a_option, ref, implAcceptHandler,
		{IMPL_ALL_IMPLS_OF_STRUCT(ListSetting)}, activePage
	)
endEvent

event OnOptionInputOpen(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(
		a_option, ref, implOpenHandler,
		{IMPL_ALL_IMPLS_OF_STRUCT(SavePresetSetting)}, activePage
	)
endEvent

event OnOptionInputAccept(int a_option, string a_input)
	IMPL_IFCHAIN_MACRO_INVOKE(
		a_option, ref, implAcceptHandler,
		{IMPL_ALL_IMPLS_OF_STRUCT(SavePresetSetting)}, activePage
	)
endEvent

event OnOptionKeyMapChange(int a_option, int a_keyCode, string a_conflictControl, string a_conflictName)
	IMPL_IFCHAIN_MACRO_INVOKE(
		a_option, ref, implSelectHandler,
		{IMPL_ALL_IMPLS_OF_STRUCT(KeyBindSetting)}, activePage
	)
endEvent

event OnOptionHighlight(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implDesc, {
		IMPL_ALL_IMPLS_OF_STRUCT(SliderSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(ToggleSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(ResetSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(ListSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(SavePresetSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(LoadPresetSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(KeyBindSetting)
	}, activePage)
endEvent