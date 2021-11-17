ToggleSetting modEnabled -> [
	settingName: "ModEnabled"
	displayName: "Disable SmoothCam"
	desc: "When selected, disables SmoothCam."
	page: " General"
]
ToggleSetting enableCrashDumps -> [
	settingName: "EnableCrashDumps"
	displayName: "Enable Crash Dump Handler"
	desc: "When enabled, SmoothCam will install a crash dump handler at game startup and will attempt to generate a mini-dump if the game crashes during SmoothCam code execution. You must restart the game for this setting to apply. If a crash does happen, follow the instructions in the message box to report the issue."
	page: " General"
]
ResetCrosshairSetting resetCrosshair -> [
	displayName: "Reset Crosshair"
	desc: "If your crosshair disappears and you can't get it back, try this option."
	page: " General"
]
ResetSetting reset -> [
	displayName: "Reset All Settings"
	desc: "Set all settings back to their default values."
	page: " General"
]
FixStateSetting forceCameraState -> [
	displayName: "Force camera to thirdperson"
	desc: "Force the camera state back to thirdperson, should the camera be stuck."
	page: " General"
]
KeyBindSetting modEnabledKey -> [
	settingName: "ModEnabledKeyCode"
	displayName: "Toggle SmoothCam Key"
	desc: "Toggles SmoothCam on and off, see 'Disable SmoothCam' setting."
	page: " General"
]
KeyBindSetting nextPresetKey -> [
	settingName: "NextPresetKeyCode"
	displayName: "Load Next Preset Key"
	desc: "Loads the next found valid preset. Note that SmoothCam doesn't save what preset you are currently using between games - When starting skyrim preset 1 will be assumed to be the active preset."
	page: " General"
]