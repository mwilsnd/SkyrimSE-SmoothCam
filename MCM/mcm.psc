ScriptName SmoothCamMCM extends SKI_ConfigBase
Import SKSE
; I hate everything about this
; This used to be way worse until I wrote the code generation tools

string[] interpMethods
string[] presets

Function SmoothCam_SetStringConfig(string member, string value) global native
Function SmoothCam_SetBoolConfig(string member, bool value) global native
Function SmoothCam_SetFloatConfig(string member, float value) global native

string Function SmoothCam_GetStringConfig(string member) global native
bool Function SmoothCam_GetBoolConfig(string member) global native
float Function SmoothCam_GetFloatConfig(string member) global native

string Function SmoothCam_SaveAsPreset(int index, string name) global native
bool Function SmoothCam_LoadPreset(int index) global native
string Function SmoothCam_GetPresetNameAtIndex(int index) global native

int Function GetCurrentInterpIndex(string setting)
	string value = SmoothCam_GetStringConfig(setting)
	
	Int i = interpMethods.Length
	While i
		i -= 1
		if (interpMethods[i] == value)
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

#constexpr_struct ListSetting {
	real_int ref = 0
	string settingName = ""
	string displayName = ""
	string desc = ""

	MACRO implControl = {
		this->ref = AddMenuOption(this->displayName, interpMethods[GetCurrentInterpIndex(this->settingName)])
	}

	MACRO implOpenHandler = {
		SetMenuDialogStartIndex(GetCurrentInterpIndex(this->settingName))
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(interpMethods)
	}

	MACRO implAcceptHandler = {
		SetMenuOptionValue(a_option, interpMethods[a_index])
		SmoothCam_SetStringConfig(this->settingName, interpMethods[a_index])
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

ScriptMeta scriptMetaInfo -> {
	version: 8
}

; Presets
SavePresetSetting savePresetSlot1 -> {
	presetIndex: 0
	displayName: "Slot 1:"
}
SavePresetSetting savePresetSlot2 -> {
	presetIndex: 1
	displayName: "Slot 2:"
}
SavePresetSetting savePresetSlot3 -> {
	presetIndex: 2
	displayName: "Slot 3:"
}
SavePresetSetting savePresetSlot4 -> {
	presetIndex: 3
	displayName: "Slot 4:"
}
SavePresetSetting savePresetSlot5 -> {
	presetIndex: 4
	displayName: "Slot 5:"
}
SavePresetSetting savePresetSlot6 -> {
	presetIndex: 5
	displayName: "Slot 6:"
}

LoadPresetSetting loadPresetSlot1 -> {
	presetIndex: 0
	displayName: "Slot 1:"
}
LoadPresetSetting loadPresetSlot2 -> {
	presetIndex: 1
	displayName: "Slot 2:"
}
LoadPresetSetting loadPresetSlot3 -> {
	presetIndex: 2
	displayName: "Slot 3:"
}
LoadPresetSetting loadPresetSlot4 -> {
	presetIndex: 3
	displayName: "Slot 4:"
}
LoadPresetSetting loadPresetSlot5 -> {
	presetIndex: 4
	displayName: "Slot 5:"
}
LoadPresetSetting loadPresetSlot6 -> {
	presetIndex: 5
	displayName: "Slot 6:"
}

; Compat
ToggleSetting icFirstPersonHorse -> {
	settingName: "FirstPersonHorse"
	displayName: "First Person Horse"
	desc: "Enable compat fixes for Improved Camera."
}
ToggleSetting icFirstPersonDragon -> {
	settingName: "FirstPersonDragon"
	displayName: "First Person Dragon"
	desc: "Enable compat fixes for Improved Camera."
}
ToggleSetting icFirstPersonSitting -> {
	settingName: "FirstPersonSitting"
	displayName: "First Person Sitting"
	desc: "Enable compat fixes for Improved Camera."
}
ToggleSetting disableDuringDialog -> {
	settingName: "DisableDuringDialog"
	displayName: "Disable During Dialog"
	desc: "Disables SmoothCam when the dialog menu is open."
}

; Following
ToggleSetting interpEnabled -> {
	settingName: "InterpolationEnabled"
	displayName: "Interpolation Enabled"
	desc: "Enable camera smoothing."
}
ToggleSetting sepZInterpEnabled -> {
	settingName: "SeparateZInterpEnabled"
	displayName: "Separate Z Interpolation Enabled"
	desc: "Enable the separate Z smoothing settings for smoothing camera height differently."
}
ToggleSetting sepLocalInterpEnabled -> {
	settingName: "SeparateLocalInterpolation"
	displayName: "Local-Space Interpolation Enabled"
	desc: "Enable separate local-space camera smoothing (Camera rotation)."
}
ToggleSetting offsetInterpEnabled -> {
	settingName: "OffsetTransitionEnabled"
	displayName: "Offset Interpolation Enabled"
	desc: "Enable smoothing of camera offset state transitions."
}
ToggleSetting zoomInterpEnabled -> {
	settingName: "ZoomTransitionEnabled"
	displayName: "Zoom Interpolation Enabled"
	desc: "Enable smoothing of camera zoom state transitions."
}
ToggleSetting disableDeltaTime -> {
	settingName: "DisableDeltaTime"
	displayName: "Disable Delta Time Factoring"
	desc: "Remove time from interpolation math. May result in less jitter but can cause speed to vary with frame rate."
}
ToggleSetting cameraDistanceClampXEnable -> {
	settingName: "CameraDistanceClampXEnable"
	displayName: "Enable X Distance Clamp"
	desc: "Clamp the maximum distance the camera may move away from the target position along the X (side) axis."
}
ToggleSetting cameraDistanceClampYEnable -> {
	settingName: "CameraDistanceClampYEnable"
	displayName: "Enable Y Distance Clamp"
	desc: "Clamp the maximum distance the camera may move away from the target position along the Y (forward) axis."
}
ToggleSetting cameraDistanceClampZEnable -> {
	settingName: "CameraDistanceClampZEnable"
	displayName: "Enable Z Distance Clamp"
	desc: "Clamp the maximum distance the camera may move away from the target position along the Z (up) axis."
}

ListSetting interpMethod -> {
	settingName: "InterpolationMethod"
	displayName: "Interpolation Method"
	desc: "The scalar method to use for camera smoothing."
}
ListSetting sepZInterpMethod -> {
	settingName: "SeparateZInterpMethod"
	displayName: "Sep. Z Interpolation Method"
	desc: "The scalar method to use for smoothing the camera height (If enabled)."
}
ListSetting sepLocalInterpMethod -> {
	settingName: "SepLocalInterpMethod"
	displayName: "Local-Space Interpolation Method"
	desc: "The scalar method to use for local-space smoothing (If enabled)."
}
ListSetting offsetInterpMethod -> {
	settingName: "OffsetTransitionMethod"
	displayName: "Offset Interpolation Method"
	desc: "The scalar method to use for offset transition smoothing (If enabled)."
}
ListSetting zoomInterpMethod -> {
	settingName: "ZoomTransitionMethod"
	displayName: "Zoom Interpolation Method"
	desc: "The scalar method to use for zoom transition smoothing (If enabled)."
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
}
SliderSetting minCameraFollowDistance -> {
	settingName: "MinFollowDistance"
	displayName: "Min Follow Distance"
	desc: "The closest the camera may get to the player when at the lowest zoom level."
	defaultValue: 64.0
	interval: 0.1
	min: 0.0
	max: 256.0
}
SliderSetting zoomMul -> {
	settingName: "ZoomMul"
	displayName: "Zoom Multiplier"
	desc: "The amount of distance to add to the camera for each zoom level."
	defaultValue: 500.0
	interval: 1.0
	min: 1.0
	max: 500.0
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
}
SliderSetting zoomTransitionDuration -> {
	settingName: "ZoomTransitionDuration"
	displayName: "Zoom Interpolation Duration"
	desc: "The smoothing duration to use when the camera changes zoom distance (In seconds)."
	defaultValue: 0.2
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
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
}

; Crosshair
ToggleSetting crosshair3DBowEnabled -> {
	settingName: "Enable3DBowCrosshair"
	displayName: "3D Bow Crosshair Enabled"
	desc: "Enable the raycasted 3D crosshair when aiming with the bow."
}
ToggleSetting crosshair3DMagicEnabled -> {
	settingName: "Enable3DMagicCrosshair"
	displayName: "3D Magic Crosshair Enabled"
	desc: "Enable the raycasted 3D crosshair when using magic."
}
ToggleSetting hideCrosshairOutOfCombat -> {
	settingName: "HideCrosshairOutOfCombat"
	displayName: "Hide Non-Combat Crosshair"
	desc: "Hide the crosshair when not in combat."
}
ToggleSetting hideCrosshairMeleeCombat -> {
	settingName: "HideCrosshairMeleeCombat"
	displayName: "Hide Melee Combat Crosshair"
	desc: "Hide the crosshair when in melee combat."
}
ToggleSetting enableCrosshairSizeManip -> {
	settingName: "EnableCrosshairSizeManip"
	displayName: "Enable Size Manipulation"
	desc: "Changes the size of the crosshair, based on 'Min/Max Size' and 'NPC Hit Size'. Disable if this causes conflicts with other mods. May require a game restart."
}
SliderSetting crosshairNPCGrowSize -> {
	settingName: "CrosshairNPCGrowSize"
	displayName: "NPC Hit Size"
	desc: "When the 3D crosshair is over an NPC, grow the size of the crosshair by this amount."
	defaultValue: 16
	interval: 1
	min: 0
	max: 32
}
SliderSetting crosshairMinDistSize -> {
	settingName: "CrosshairMinDistSize"
	displayName: "Min Crosshair Size"
	desc: "Sets the size of the 3D crosshair when the player's aim ray is at the maximum distance."
	defaultValue: 16
	interval: 1
	min: 8
	max: 32
}
SliderSetting crosshairMaxDistSize -> {
	settingName: "CrosshairMaxDistSize"
	displayName: "Max Crosshair Size"
	desc: "Sets the size of the 3D crosshair when the player's aim ray is at the minimum distance."
	defaultValue: 24
	interval: 1
	min: 8
	max: 32
}

; Standing
SliderSetting standing_sideOffset -> {
	settingName: "Standing:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting standing_upOffset -> {
	settingName: "Standing:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
SliderSetting standing_zoomOffset -> {
	settingName: "Standing:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
}
SliderSetting standing_sideOffsetRangedCombat -> {
	settingName: "StandingCombat:Ranged:SideOffset"
	displayName: "Ranged Combat Side Offset"
	desc: "The amount to move the camera to the right when in ranged combat."
	defaultValue: 25.0
}
SliderSetting standing_upOffsetRangedCombat -> {
	settingName: "StandingCombat:Ranged:UpOffset"
	displayName: "Ranged Combat Up Offset"
	desc: "The amount to move the camera up when in ranged combat."
}
SliderSetting standing_zoomOffsetRangedCombat -> {
	settingName: "StandingCombat:Ranged:ZoomOffset"
	displayName: "Ranged Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in ranged combat."
	min: -200.0
	max: 200.0
}
SliderSetting standing_sideOffsetMagicCombat -> {
	settingName: "StandingCombat:Magic:SideOffset"
	displayName: "Magic Combat Side Offset"
	desc: "The amount to move the camera to the right when in magic combat."
	defaultValue: 25.0
}
SliderSetting standing_upOffsetMagicCombat -> {
	settingName: "StandingCombat:Magic:UpOffset"
	displayName: "Magic Combat Up Offset"
	desc: "The amount to move the camera up when in magic combat."
}
SliderSetting standing_zoomOffsetMagicCombat -> {
	settingName: "StandingCombat:Magic:ZoomOffset"
	displayName: "Magic Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in magic combat."
	min: -200.0
	max: 200.0
}
SliderSetting standing_sideOffsetMeleeCombat -> {
	settingName: "StandingCombat:Melee:SideOffset"
	displayName: "Melee Combat Side Offset"
	desc: "The amount to move the camera to the right when in melee combat."
	defaultValue: 25.0
}
SliderSetting standing_upOffsetMeleeCombat -> {
	settingName: "StandingCombat:Melee:UpOffset"
	displayName: "Melee Combat Up Offset"
	desc: "The amount to move the camera up when in melee combat."
}
SliderSetting standing_zoomOffsetMeleeCombat -> {
	settingName: "StandingCombat:Melee:ZoomOffset"
	displayName: "Melee Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in melee combat."
	min: -200.0
	max: 200.0
}
ToggleSetting standing_interp -> {
	settingName: "InterpStanding"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting standing_interpRanged -> {
	settingName: "InterpStandingRangedCombat"
	displayName: "Enable Ranged Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting standing_interpMagic -> {
	settingName: "InterpStandingMagicCombat"
	displayName: "Enable Magic Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting standing_interpMelee -> {
	settingName: "InterpStandingMeleeCombat"
	displayName: "Enable Melee Interpolation"
	desc: "Enables interpolation in this state."
}

; Walking
SliderSetting walking_sideOffset -> {
	settingName: "Walking:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting walking_upOffset -> {
	settingName: "Walking:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
SliderSetting walking_zoomOffset -> {
	settingName: "Walking:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
}
SliderSetting walking_sideOffsetRangedCombat -> {
	settingName: "WalkingCombat:Ranged:SideOffset"
	displayName: "Ranged Combat Side Offset"
	desc: "The amount to move the camera to the right when in ranged combat."
	defaultValue: 25.0
}
SliderSetting walking_upOffsetRangedCombat -> {
	settingName: "WalkingCombat:Ranged:UpOffset"
	displayName: "Ranged Combat Up Offset"
	desc: "The amount to move the camera up when in ranged combat."
}
SliderSetting walking_zoomOffsetRangedCombat -> {
	settingName: "WalkingCombat:Ranged:ZoomOffset"
	displayName: "Ranged Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in ranged combat."
	min: -200.0
	max: 200.0
}
SliderSetting walking_sideOffsetMagicCombat -> {
	settingName: "WalkingCombat:Magic:SideOffset"
	displayName: "Magic Combat Side Offset"
	desc: "The amount to move the camera to the right when in magic combat."
	defaultValue: 25.0
}
SliderSetting walking_upOffsetMagicCombat -> {
	settingName: "WalkingCombat:Magic:UpOffset"
	displayName: "Magic Combat Up Offset"
	desc: "The amount to move the camera up when in magic combat."
}
SliderSetting walking_zoomOffsetMagicCombat -> {
	settingName: "WalkingCombat:Magic:ZoomOffset"
	displayName: "Magic Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in magic combat."
	min: -200.0
	max: 200.0
}
SliderSetting walking_sideOffsetMeleeCombat -> {
	settingName: "WalkingCombat:Melee:SideOffset"
	displayName: "Melee Combat Side Offset"
	desc: "The amount to move the camera to the right when in melee combat."
	defaultValue: 25.0
}
SliderSetting walking_upOffsetMeleeCombat -> {
	settingName: "WalkingCombat:Melee:UpOffset"
	displayName: "Melee Combat Up Offset"
	desc: "The amount to move the camera up when in melee combat."
}
SliderSetting walking_zoomOffsetMeleeCombat -> {
	settingName: "WalkingCombat:Melee:ZoomOffset"
	displayName: "Melee Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in melee combat."
	min: -200.0
	max: 200.0
}
ToggleSetting walking_interp -> {
	settingName: "InterpWalking"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting walking_interpRanged -> {
	settingName: "InterpWalkingRangedCombat"
	displayName: "Enable Ranged Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting walking_interpMagic -> {
	settingName: "InterpWalkingMagicCombat"
	displayName: "Enable Magic Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting walking_interpMelee -> {
	settingName: "InterpWalkingMeleeCombat"
	displayName: "Enable Melee Interpolation"
	desc: "Enables interpolation in this state."
}

; Running
SliderSetting running_sideOffset -> {
	settingName: "Running:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting running_upOffset -> {
	settingName: "Running:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
SliderSetting running_zoomOffset -> {
	settingName: "Running:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
}
SliderSetting running_sideOffsetRangedCombat -> {
	settingName: "RunningCombat:Ranged:SideOffset"
	displayName: "Ranged Combat Side Offset"
	desc: "The amount to move the camera to the right when in ranged combat."
	defaultValue: 25.0
}
SliderSetting running_upOffsetRangedCombat -> {
	settingName: "RunningCombat:Ranged:UpOffset"
	displayName: "Ranged Combat Up Offset"
	desc: "The amount to move the camera up when in ranged combat."
}
SliderSetting running_zoomOffsetRangedCombat -> {
	settingName: "RunningCombat:Ranged:ZoomOffset"
	displayName: "Ranged Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in ranged combat."
	min: -200.0
	max: 200.0
}
SliderSetting running_sideOffsetMagicCombat -> {
	settingName: "RunningCombat:Magic:SideOffset"
	displayName: "Magic Combat Side Offset"
	desc: "The amount to move the camera to the right when in magic combat."
	defaultValue: 25.0
}
SliderSetting running_upOffsetMagicCombat -> {
	settingName: "RunningCombat:Magic:UpOffset"
	displayName: "Magic Combat Up Offset"
	desc: "The amount to move the camera up when in magic combat."
}
SliderSetting running_zoomOffsetMagicCombat -> {
	settingName: "RunningCombat:Magic:ZoomOffset"
	displayName: "Magic Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in magic combat."
	min: -200.0
	max: 200.0
}
SliderSetting running_sideOffsetMeleeCombat -> {
	settingName: "RunningCombat:Melee:SideOffset"
	displayName: "Melee Combat Side Offset"
	desc: "The amount to move the camera to the right when in melee combat."
	defaultValue: 25.0
}
SliderSetting running_upOffsetMeleeCombat -> {
	settingName: "RunningCombat:Melee:UpOffset"
	displayName: "Melee Combat Up Offset"
	desc: "The amount to move the camera up when in melee combat."
}
SliderSetting running_zoomOffsetMeleeCombat -> {
	settingName: "RunningCombat:Melee:ZoomOffset"
	displayName: "Melee Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in melee combat."
	min: -200.0
	max: 200.0
}
ToggleSetting running_interp -> {
	settingName: "InterpRunning"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting running_interpRanged -> {
	settingName: "InterpRunningRangedCombat"
	displayName: "Enable Ranged Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting running_interpMagic -> {
	settingName: "InterpRunningMagicCombat"
	displayName: "Enable Magic Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting running_interpMelee -> {
	settingName: "InterpRunningMeleeCombat"
	displayName: "Enable Melee Interpolation"
	desc: "Enables interpolation in this state."
}

; Sprinting
SliderSetting sprinting_sideOffset -> {
	settingName: "Sprinting:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting sprinting_upOffset -> {
	settingName: "Sprinting:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
SliderSetting sprinting_zoomOffset -> {
	settingName: "Sprinting:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
}
SliderSetting sprinting_sideOffsetRangedCombat -> {
	settingName: "SprintingCombat:Ranged:SideOffset"
	displayName: "Ranged Combat Side Offset"
	desc: "The amount to move the camera to the right when in ranged combat."
	defaultValue: 25.0
}
SliderSetting sprinting_upOffsetRangedCombat -> {
	settingName: "SprintingCombat:Ranged:UpOffset"
	displayName: "Ranged Combat Up Offset"
	desc: "The amount to move the camera up when in ranged combat."
}
SliderSetting sprinting_zoomOffsetRangedCombat -> {
	settingName: "SprintingCombat:Ranged:ZoomOffset"
	displayName: "Ranged Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in ranged combat."
	min: -200.0
	max: 200.0
}
SliderSetting sprinting_sideOffsetMagicCombat -> {
	settingName: "SprintingCombat:Magic:SideOffset"
	displayName: "Magic Combat Side Offset"
	desc: "The amount to move the camera to the right when in magic combat."
	defaultValue: 25.0
}
SliderSetting sprinting_upOffsetMagicCombat -> {
	settingName: "SprintingCombat:Magic:UpOffset"
	displayName: "Magic Combat Up Offset"
	desc: "The amount to move the camera up when in magic combat."
}
SliderSetting sprinting_zoomOffsetMagicCombat -> {
	settingName: "SprintingCombat:Magic:ZoomOffset"
	displayName: "Magic Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in magic combat."
	min: -200.0
	max: 200.0
}
SliderSetting sprinting_sideOffsetMeleeCombat -> {
	settingName: "SprintingCombat:Melee:SideOffset"
	displayName: "Melee Combat Side Offset"
	desc: "The amount to move the camera to the right when in melee combat."
	defaultValue: 25.0
}
SliderSetting sprinting_upOffsetMeleeCombat -> {
	settingName: "SprintingCombat:Melee:UpOffset"
	displayName: "Melee Combat Up Offset"
	desc: "The amount to move the camera up when in melee combat."
}
SliderSetting sprinting_zoomOffsetMeleeCombat -> {
	settingName: "SprintingCombat:Melee:ZoomOffset"
	displayName: "Melee Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in melee combat."
	min: -200.0
	max: 200.0
}
ToggleSetting sprinting_interp -> {
	settingName: "InterpSprinting"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting sprinting_interpRanged -> {
	settingName: "InterpSprintingRangedCombat"
	displayName: "Enable Ranged Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting sprinting_interpMagic -> {
	settingName: "InterpSprintingMagicCombat"
	displayName: "Enable Magic Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting sprinting_interpMelee -> {
	settingName: "InterpSprintingMeleeCombat"
	displayName: "Enable Melee Interpolation"
	desc: "Enables interpolation in this state."
}

; Sneaking
SliderSetting sneaking_sideOffset -> {
	settingName: "Sneaking:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting sneaking_upOffset -> {
	settingName: "Sneaking:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
SliderSetting sneaking_zoomOffset -> {
	settingName: "Sneaking:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
}
SliderSetting sneaking_sideOffsetRangedCombat -> {
	settingName: "SneakingCombat:Ranged:SideOffset"
	displayName: "Ranged Combat Side Offset"
	desc: "The amount to move the camera to the right when in ranged combat."
	defaultValue: 25.0
}
SliderSetting sneaking_upOffsetRangedCombat -> {
	settingName: "SneakingCombat:Ranged:UpOffset"
	displayName: "Ranged Combat Up Offset"
	desc: "The amount to move the camera up when in ranged combat."
}
SliderSetting sneaking_zoomOffsetRangedCombat -> {
	settingName: "SneakingCombat:Ranged:ZoomOffset"
	displayName: "Ranged Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in ranged combat."
	min: -200.0
	max: 200.0
}
SliderSetting sneaking_sideOffsetMagicCombat -> {
	settingName: "SneakingCombat:Magic:SideOffset"
	displayName: "Magic Combat Side Offset"
	desc: "The amount to move the camera to the right when in magic combat."
	defaultValue: 25.0
}
SliderSetting sneaking_upOffsetMagicCombat -> {
	settingName: "SneakingCombat:Magic:UpOffset"
	displayName: "Magic Combat Up Offset"
	desc: "The amount to move the camera up when in magic combat."
}
SliderSetting sneaking_zoomOffsetMagicCombat -> {
	settingName: "SneakingCombat:Magic:ZoomOffset"
	displayName: "Magic Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in magic combat."
	min: -200.0
	max: 200.0
}
SliderSetting sneaking_sideOffsetMeleeCombat -> {
	settingName: "SneakingCombat:Melee:SideOffset"
	displayName: "Melee Combat Side Offset"
	desc: "The amount to move the camera to the right when in melee combat."
	defaultValue: 25.0
}
SliderSetting sneaking_upOffsetMeleeCombat -> {
	settingName: "SneakingCombat:Melee:UpOffset"
	displayName: "Melee Combat Up Offset"
	desc: "The amount to move the camera up when in melee combat."
}
SliderSetting sneaking_zoomOffsetMeleeCombat -> {
	settingName: "SneakingCombat:Melee:ZoomOffset"
	displayName: "Melee Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in melee combat."
	min: -200.0
	max: 200.0
}
ToggleSetting sneaking_interp -> {
	settingName: "InterpSneaking"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting sneaking_interpRanged -> {
	settingName: "InterpSneakingRangedCombat"
	displayName: "Enable Ranged Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting sneaking_interpMagic -> {
	settingName: "InterpSneakingMagicCombat"
	displayName: "Enable Magic Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting sneaking_interpMelee -> {
	settingName: "InterpSneakingMeleeCombat"
	displayName: "Enable Melee Interpolation"
	desc: "Enables interpolation in this state."
}

; Swimming
SliderSetting swimming_sideOffset -> {
	settingName: "Swimming:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting swimming_upOffset -> {
	settingName: "Swimming:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
SliderSetting swimming_zoomOffset -> {
	settingName: "Swimming:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
}
ToggleSetting swimming_interp -> {
	settingName: "InterpSwimming"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
}

; Bow aiming
SliderSetting bowaim_sideOffset -> {
	settingName: "Bowaim:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting bowaim_upOffset -> {
	settingName: "Bowaim:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
SliderSetting bowaim_zoomOffset -> {
	settingName: "Bowaim:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
}
SliderSetting bowaim_sideOffsetHorseback -> {
	settingName: "BowaimHorse:SideOffset"
	displayName: "Horseback Side Offset"
	desc: "The amount to move the camera to the right when on horseback."
	defaultValue: 25.0
}
SliderSetting bowaim_upOffsetHorseback -> {
	settingName: "BowaimHorse:UpOffset"
	displayName: "Horseback Up Offset"
	desc: "The amount to move the camera up when on horseback."
}
SliderSetting bowaim_zoomOffsetHorseback -> {
	settingName: "BowaimHorse:ZoomOffset"
	displayName: "Horseback Zoom Offset"
	desc: "The amount to offset the camera zoom by when on horseback."
	min: -200.0
	max: 200.0
}
ToggleSetting bowaim_interp -> {
	settingName: "InterpBowAim"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting bowaim_interpHorseback -> {
	settingName: "InterpBowAimHorseback"
	displayName: "Enable Horseback Interpolation"
	desc: "Enables interpolation in this state."
}

; Sitting
SliderSetting sitting_sideOffset -> {
	settingName: "Sitting:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting sitting_upOffset -> {
	settingName: "Sitting:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
SliderSetting sitting_zoomOffset -> {
	settingName: "Sitting:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
}
ToggleSetting sitting_interp -> {
	settingName: "InterpSitting"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
}

; Horseback
SliderSetting horseback_sideOffset -> {
	settingName: "Horseback:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting horseback_upOffset -> {
	settingName: "Horseback:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
SliderSetting horseback_zoomOffset -> {
	settingName: "Horseback:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
}
SliderSetting horseback_sideOffsetRangedCombat -> {
	settingName: "HorsebackCombat:Ranged:SideOffset"
	displayName: "Ranged Combat Side Offset"
	desc: "The amount to move the camera to the right when in ranged combat."
	defaultValue: 25.0
}
SliderSetting horseback_upOffsetRangedCombat -> {
	settingName: "HorsebackCombat:Ranged:UpOffset"
	displayName: "Ranged Combat Up Offset"
	desc: "The amount to move the camera up when in ranged combat."
}
SliderSetting horseback_zoomOffsetRangedCombat -> {
	settingName: "HorsebackCombat:Ranged:ZoomOffset"
	displayName: "Ranged Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in ranged combat."
	min: -200.0
	max: 200.0
}
SliderSetting horseback_sideOffsetMagicCombat -> {
	settingName: "HorsebackCombat:Magic:SideOffset"
	displayName: "Magic Combat Side Offset"
	desc: "The amount to move the camera to the right when in magic combat."
	defaultValue: 25.0
}
SliderSetting horseback_upOffsetMagicCombat -> {
	settingName: "HorsebackCombat:Magic:UpOffset"
	displayName: "Magic Combat Up Offset"
	desc: "The amount to move the camera up when in magic combat."
}
SliderSetting horseback_zoomOffsetMagicCombat -> {
	settingName: "HorsebackCombat:Magic:ZoomOffset"
	displayName: "Magic Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in magic combat."
	min: -200.0
	max: 200.0
}
SliderSetting horseback_sideOffsetMeleeCombat -> {
	settingName: "HorsebackCombat:Melee:SideOffset"
	displayName: "Melee Combat Side Offset"
	desc: "The amount to move the camera to the right when in melee combat."
	defaultValue: 25.0
}
SliderSetting horseback_upOffsetMeleeCombat -> {
	settingName: "HorsebackCombat:Melee:UpOffset"
	displayName: "Melee Combat Up Offset"
	desc: "The amount to move the camera up when in melee combat."
}
SliderSetting horseback_zoomOffsetMeleeCombat -> {
	settingName: "HorsebackCombat:Melee:ZoomOffset"
	displayName: "Melee Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in melee combat."
	min: -200.0
	max: 200.0
}
ToggleSetting horseback_interp -> {
	settingName: "InterpHorseback"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting horseback_interpRanged -> {
	settingName: "InterpHorsebackRangedCombat"
	displayName: "Enable Ranged Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting horseback_interpMagic -> {
	settingName: "InterpHorsebackMagicCombat"
	displayName: "Enable Magic Interpolation"
	desc: "Enables interpolation in this state."
}
ToggleSetting horseback_interpMelee -> {
	settingName: "InterpHorsebackMeleeCombat"
	displayName: "Enable Melee Interpolation"
	desc: "Enables interpolation in this state."
}

; Dragon
SliderSetting dragon_sideOffset -> {
	settingName: "Dragon:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting dragon_upOffset -> {
	settingName: "Dragon:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}

; Group edit
SliderSetting groupEdit_sideOffset -> {
	settingName: "Group:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting groupEdit_upOffset -> {
	settingName: "Group:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
SliderSetting groupEdit_zoomOffset -> {
	settingName: "Group:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
}
SliderSetting groupEdit_sideOffsetRangedCombat -> {
	settingName: "Group:Ranged:SideOffset"
	displayName: "Ranged Combat Side Offset"
	desc: "The amount to move the camera to the right when in ranged combat."
	defaultValue: 25.0
}
SliderSetting groupEdit_upOffsetRangedCombat -> {
	settingName: "Group:Ranged:UpOffset"
	displayName: "Ranged Combat Up Offset"
	desc: "The amount to move the camera up when in ranged combat."
}
SliderSetting groupEdit_zoomOffsetRangedCombat -> {
	settingName: "Group:Ranged:ZoomOffset"
	displayName: "Ranged Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in ranged combat."
	min: -200.0
	max: 200.0
}
SliderSetting groupEdit_sideOffsetMagicCombat -> {
	settingName: "Group:Magic:SideOffset"
	displayName: "Magic Combat Side Offset"
	desc: "The amount to move the camera to the right when in magic combat."
	defaultValue: 25.0
}
SliderSetting groupEdit_upOffsetMagicCombat -> {
	settingName: "Group:Magic:UpOffset"
	displayName: "Magic Combat Up Offset"
	desc: "The amount to move the camera up when in magic combat."
}
SliderSetting groupEdit_zoomOffsetMagicCombat -> {
	settingName: "Group:Magic:ZoomOffset"
	displayName: "Magic Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in magic combat."
	min: -200.0
	max: 200.0
}
SliderSetting groupEdit_sideOffsetMeleeCombat -> {
	settingName: "Group:Melee:SideOffset"
	displayName: "Melee Combat Side Offset"
	desc: "The amount to move the camera to the right when in melee combat."
	defaultValue: 25.0
}
SliderSetting groupEdit_upOffsetMeleeCombat -> {
	settingName: "Group:Melee:UpOffset"
	displayName: "Melee Combat Up Offset"
	desc: "The amount to move the camera up when in melee combat."
}
SliderSetting groupEdit_zoomOffsetMeleeCombat -> {
	settingName: "Group:Melee:ZoomOffset"
	displayName: "Melee Combat Zoom Offset"
	desc: "The amount to offset the camera zoom by when in melee combat."
	min: -200.0
	max: 200.0
}

int Function GetVersion()
	return scriptMetaInfo.version
endFunction

event OnConfigInit()
	Pages = new string[] -> {
		" Info", " Compatibility", " Following", " Crosshair", " Standing",
		" Walking", " Running", " Sprinting", " Sneaking",
		" Swimming", " Bow Aiming", " Sitting", " Horseback",
		" Dragon", " Group Edit", " Presets"
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
endEvent

event OnPageReset(string a_page)
	SetCursorFillMode(TOP_TO_BOTTOM)

	if (a_page == " Info")
		int version_T = AddTextOption("DLL Version", GetPluginVersion("SmoothCam"), OPTION_FLAG_DISABLED)
		int s_version_T = AddTextOption("MCM Script Version", scriptMetaInfo.version, OPTION_FLAG_DISABLED)
	elseIf (a_page == " Compatibility")
		AddHeaderOption("General")
		disableDuringDialog->!implControl

		AddHeaderOption("Improved Camera Patches")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			icFirstPersonHorse, icFirstPersonDragon, icFirstPersonSitting
		})
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

		SetCursorPosition(1)
		AddHeaderOption("Distance Clamping")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			cameraDistanceClampXEnable, cameraDistanceClampXMin, cameraDistanceClampXMax,
			cameraDistanceClampYEnable, cameraDistanceClampYMin, cameraDistanceClampYMax,
			cameraDistanceClampZEnable, cameraDistanceClampZMin, cameraDistanceClampZMax
		})

		AddHeaderOption("Misc")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			zoomMul, disableDeltaTime
		})
	elseIf (a_page == " Crosshair")
		AddHeaderOption("3D Crosshair Settings")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			crosshair3DBowEnabled, crosshair3DMagicEnabled, enableCrosshairSizeManip,
			crosshairNPCGrowSize, crosshairMinDistSize, crosshairMaxDistSize
		})

		AddHeaderOption("Crosshair Hiding")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			hideCrosshairOutOfCombat, hideCrosshairMeleeCombat,
		})
	elseIf (a_page == " Standing")
		AddHeaderOption("Standing Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			standing_sideOffset,
			standing_upOffset,
			standing_zoomOffset,
			standing_sideOffsetRangedCombat,
			standing_upOffsetRangedCombat,
			standing_zoomOffsetRangedCombat,
			standing_sideOffsetMagicCombat,
			standing_upOffsetMagicCombat,
			standing_zoomOffsetMagicCombat,
			standing_sideOffsetMeleeCombat,
			standing_upOffsetMeleeCombat,
			standing_zoomOffsetMeleeCombat
		})

		SetCursorPosition(1)
		AddHeaderOption("Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			standing_interp,
			standing_interpRanged,
			standing_interpMagic,
			standing_interpMelee
		})
	elseIf (a_page == " Walking")
		AddHeaderOption("Walking Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			walking_sideOffset,
			walking_upOffset,
			walking_zoomOffset,
			walking_sideOffsetRangedCombat,
			walking_upOffsetRangedCombat,
			walking_zoomOffsetRangedCombat,
			walking_sideOffsetMagicCombat,
			walking_upOffsetMagicCombat,
			walking_zoomOffsetMagicCombat,
			walking_sideOffsetMeleeCombat,
			walking_upOffsetMeleeCombat,
			walking_zoomOffsetMeleeCombat
		})

		SetCursorPosition(1)
		AddHeaderOption("Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			walking_interp,
			walking_interpRanged,
			walking_interpMagic,
			walking_interpMelee
		})
	elseIf (a_page == " Running")
		AddHeaderOption("Running Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			running_sideOffset,
			running_upOffset,
			running_zoomOffset,
			running_sideOffsetRangedCombat,
			running_upOffsetRangedCombat,
			running_zoomOffsetRangedCombat,
			running_sideOffsetMagicCombat,
			running_upOffsetMagicCombat,
			running_zoomOffsetMagicCombat,
			running_sideOffsetMeleeCombat,
			running_upOffsetMeleeCombat,
			running_zoomOffsetMeleeCombat
		})

		SetCursorPosition(1)
		AddHeaderOption("Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			running_interp,
			running_interpRanged,
			running_interpMagic,
			running_interpMelee
		})
	elseIf (a_page == " Sprinting")
		AddHeaderOption("Sprinting Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			sprinting_sideOffset,
			sprinting_upOffset,
			sprinting_zoomOffset,
			sprinting_sideOffsetRangedCombat,
			sprinting_upOffsetRangedCombat,
			sprinting_zoomOffsetRangedCombat,
			sprinting_sideOffsetMagicCombat,
			sprinting_upOffsetMagicCombat,
			sprinting_zoomOffsetMagicCombat,
			sprinting_sideOffsetMeleeCombat,
			sprinting_upOffsetMeleeCombat,
			sprinting_zoomOffsetMeleeCombat
		})

		SetCursorPosition(1)
		AddHeaderOption("Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			sprinting_interp,
			sprinting_interpRanged,
			sprinting_interpMagic,
			sprinting_interpMelee
		})
	elseIf (a_page == " Sneaking")
		AddHeaderOption("Sneaking Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			sneaking_sideOffset,
			sneaking_upOffset,
			sneaking_zoomOffset,
			sneaking_sideOffsetRangedCombat,
			sneaking_upOffsetRangedCombat,
			sneaking_zoomOffsetRangedCombat,
			sneaking_sideOffsetMagicCombat,
			sneaking_upOffsetMagicCombat,
			sneaking_zoomOffsetMagicCombat,
			sneaking_sideOffsetMeleeCombat,
			sneaking_upOffsetMeleeCombat,
			sneaking_zoomOffsetMeleeCombat
		})

		SetCursorPosition(1)
		AddHeaderOption("Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			sneaking_interp,
			sneaking_interpRanged,
			sneaking_interpMagic,
			sneaking_interpMelee
		})
	elseIf (a_page == " Swimming")
		AddHeaderOption("Swimming Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			swimming_sideOffset,
			swimming_upOffset,
			swimming_zoomOffset
		})

		SetCursorPosition(1)
		AddHeaderOption("Interpolation")
		swimming_interp->!implControl
	elseIf (a_page == " Bow Aiming")
		AddHeaderOption("Bow Aiming Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			bowaim_sideOffset,
			bowaim_upOffset,
			bowaim_zoomOffset,
			bowaim_sideOffsetHorseback,
			bowaim_upOffsetHorseback,
			bowaim_zoomOffsetHorseback
		})

		SetCursorPosition(1)
		AddHeaderOption("Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			bowaim_interp,
			bowaim_interpHorseback
		})
	elseIf (a_page == " Sitting")
		AddHeaderOption("Sitting Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			sitting_sideOffset,
			sitting_upOffset,
			sitting_zoomOffset
		})

		SetCursorPosition(1)
		AddHeaderOption("Interpolation")
		sitting_interp->!implControl
	elseIf (a_page == " Horseback")
		AddHeaderOption("Horseback Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			horseback_sideOffset,
			horseback_upOffset,
			horseback_zoomOffset,
			horseback_sideOffsetRangedCombat,
			horseback_upOffsetRangedCombat,
			horseback_zoomOffsetRangedCombat,
			horseback_sideOffsetMagicCombat,
			horseback_upOffsetMagicCombat,
			horseback_zoomOffsetMagicCombat,
			horseback_sideOffsetMeleeCombat,
			horseback_upOffsetMeleeCombat,
			horseback_zoomOffsetMeleeCombat
		})

		SetCursorPosition(1)
		AddHeaderOption("Interpolation")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			horseback_interp,
			horseback_interpRanged,
			horseback_interpMagic,
			horseback_interpMelee
		})
	elseIf (a_page == " Dragon")
		AddHeaderOption("Dragon Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			dragon_sideOffset,
			dragon_upOffset
		})
	elseIf (a_page == " Group Edit")
		AddHeaderOption("Edit All Offset Groups")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			groupEdit_sideOffset,
			groupEdit_upOffset,
			groupEdit_zoomOffset,
			groupEdit_sideOffsetRangedCombat,
			groupEdit_upOffsetRangedCombat,
			groupEdit_zoomOffsetRangedCombat,
			groupEdit_sideOffsetMagicCombat,
			groupEdit_upOffsetMagicCombat,
			groupEdit_zoomOffsetMagicCombat,
			groupEdit_sideOffsetMeleeCombat,
			groupEdit_upOffsetMeleeCombat,
			groupEdit_zoomOffsetMeleeCombat
		})
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
		IMPL_ALL_IMPLS_OF_STRUCT(LoadPresetSetting)
	})
endEvent

event OnOptionSliderOpen(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implOpenHandler, {IMPL_ALL_IMPLS_OF_STRUCT(SliderSetting)})
endEvent

event OnOptionSliderAccept(int a_option, float a_value)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implAcceptHandler, {IMPL_ALL_IMPLS_OF_STRUCT(SliderSetting)})
endEvent

event OnOptionMenuOpen(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implOpenHandler, {IMPL_ALL_IMPLS_OF_STRUCT(ListSetting)})
endEvent

event OnOptionMenuAccept(int a_option, int a_index)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implAcceptHandler, {IMPL_ALL_IMPLS_OF_STRUCT(ListSetting)})
endEvent

event OnOptionInputOpen(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implOpenHandler, {IMPL_ALL_IMPLS_OF_STRUCT(SavePresetSetting)})
endEvent

event OnOptionInputAccept(int a_option, string a_input)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implAcceptHandler, {IMPL_ALL_IMPLS_OF_STRUCT(SavePresetSetting)})
endEvent

event OnOptionHighlight(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implDesc, {
		IMPL_ALL_IMPLS_OF_STRUCT(SliderSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(ToggleSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(ListSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(SavePresetSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(LoadPresetSetting)
	})
endEvent