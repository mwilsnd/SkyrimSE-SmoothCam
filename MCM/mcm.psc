ScriptName SmoothCamMCM extends SKI_ConfigBase
Import SKSE
; I hate everything about this
; This used to be way worse until I wrote the code generation tools

string[] interpMethods

int Function GetVersion()
	return 1
endFunction

Function SmoothCam_SetStringConfig(string member, string value) global native
Function SmoothCam_SetBoolConfig(string member, bool value) global native
Function SmoothCam_SetFloatConfig(string member, float value) global native

string Function SmoothCam_GetStringConfig(string member) global native
bool Function SmoothCam_GetBoolConfig(string member) global native
float Function SmoothCam_GetFloatConfig(string member) global native

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
ToggleSetting crosshair3DEnabled -> {
	settingName: "Enable3DCrosshair"
	displayName: "3D Crosshair Enabled"
	desc: "Enable the raycasted 3D crosshair when in combat."
}
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
	max: 64.0
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
SliderSetting standing_sideOffsetCombat -> {
	settingName: "StandingCombat:SideOffset"
	displayName: "Combat Side Offset"
	desc: "The amount to move the camera to the right when in combat."
	defaultValue: 25.0
}
SliderSetting standing_upOffsetCombat -> {
	settingName: "StandingCombat:UpOffset"
	displayName: "Combat Up Offset"
	desc: "The amount to move the camera up when in combat."
}
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
SliderSetting walking_sideOffsetCombat -> {
	settingName: "WalkingCombat:SideOffset"
	displayName: "Combat Side Offset"
	desc: "The amount to move the camera to the right when in combat."
	defaultValue: 25.0
}
SliderSetting walking_upOffsetCombat -> {
	settingName: "WalkingCombat:UpOffset"
	displayName: "Combat Up Offset"
	desc: "The amount to move the camera up when in combat."
}
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
SliderSetting running_sideOffsetCombat -> {
	settingName: "RunningCombat:SideOffset"
	displayName: "Combat Side Offset"
	desc: "The amount to move the camera to the right when in combat."
	defaultValue: 25.0
}
SliderSetting running_upOffsetCombat -> {
	settingName: "RunningCombat:UpOffset"
	displayName: "Combat Up Offset"
	desc: "The amount to move the camera up when in combat."
}
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
SliderSetting sprinting_sideOffsetCombat -> {
	settingName: "SprintingCombat:SideOffset"
	displayName: "Combat Side Offset"
	desc: "The amount to move the camera to the right when in combat."
	defaultValue: 25.0
}
SliderSetting sprinting_upOffsetCombat -> {
	settingName: "SprintingCombat:UpOffset"
	displayName: "Combat Up Offset"
	desc: "The amount to move the camera up when in combat."
}
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
SliderSetting sneaking_sideOffsetCombat -> {
	settingName: "SneakingCombat:SideOffset"
	displayName: "Combat Side Offset"
	desc: "The amount to move the camera to the right when in combat."
	defaultValue: 25.0
}
SliderSetting sneaking_upOffsetCombat -> {
	settingName: "SneakingCombat:UpOffset"
	displayName: "Combat Up Offset"
	desc: "The amount to move the camera up when in combat."
}
SliderSetting swimming_sideOffset -> {
	settingName: "Sneaking:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
}
SliderSetting swimming_upOffset -> {
	settingName: "Sneaking:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
}
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

event OnConfigInit()
	Pages = new string[] -> {
		" Info", " Compatibility", " Following", " Standing",
		" Walking", " Running", " Sprinting", " Sneaking",
		" Swimming", " Bow Aiming", " Sitting", " Horseback",
		" Dragon"
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
	elseIf (a_page == " Compatibility")
		AddHeaderOption("Improved Camera Patches")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			icFirstPersonHorse, icFirstPersonDragon, icFirstPersonSitting
		})
	elseIf (a_page == " Following")
		AddHeaderOption("Following Settings")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			interpMethod, interpEnabled, sepZInterpMethod, sepZInterpEnabled, minCameraFollowDistance,
			minCameraFollowRate, maxCameraFollowRate, maxSmoothingInterpDistance, zoomMul, minSepZFollowRate,
			maxSepZFollowRate, maxSepZSmoothingDistance, crosshair3DEnabled
		})
	elseIf (a_page == " Standing")
		AddHeaderOption("Standing Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			standing_sideOffset, standing_upOffset, standing_sideOffsetCombat, standing_upOffsetCombat
		})
	elseIf (a_page == " Walking")
		AddHeaderOption("Walking Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			walking_sideOffset, walking_upOffset, walking_sideOffsetCombat, walking_upOffsetCombat
		})
	elseIf (a_page == " Running")
		AddHeaderOption("Running Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			running_sideOffset, running_upOffset, running_sideOffsetCombat, running_upOffsetCombat
		})
	elseIf (a_page == " Sprinting")
		AddHeaderOption("Sprinting Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			sprinting_sideOffset, sprinting_upOffset, sprinting_sideOffsetCombat, sprinting_upOffsetCombat
		})
	elseIf (a_page == " Sneaking")
		AddHeaderOption("Sneaking Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			sneaking_sideOffset, sneaking_upOffset, sneaking_sideOffsetCombat, sneaking_upOffsetCombat
		})
	elseIf (a_page == " Swimming")
		AddHeaderOption("Swimming Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {swimming_sideOffset, swimming_upOffset})
	elseIf (a_page == " Bow Aiming")
		AddHeaderOption("Bow Aiming Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {
			bowaim_sideOffset, bowaim_upOffset, bowaim_sideOffsetHorseback, bowaim_upOffsetHorseback
		})
	elseIf (a_page == " Sitting")
		AddHeaderOption("Sitting Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {sitting_sideOffset, sitting_upOffset})
	elseIf (a_page == " Horseback")
		AddHeaderOption("Horseback Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {horseback_sideOffset, horseback_upOffset})
	elseIf (a_page == " Dragon")
		AddHeaderOption("Dragon Offsets")
		IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {dragon_sideOffset, dragon_upOffset})
	endIf
endEvent

event OnOptionSelect(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implSelectHandler, {IMPL_ALL_IMPLS_OF_STRUCT(ToggleSetting)})
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

event OnOptionHighlight(int a_option)
	IMPL_IFCHAIN_MACRO_INVOKE(a_option, ref, implDesc, {
		IMPL_ALL_IMPLS_OF_STRUCT(SliderSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(ToggleSetting),
		IMPL_ALL_IMPLS_OF_STRUCT(ListSetting)
	})
endEvent