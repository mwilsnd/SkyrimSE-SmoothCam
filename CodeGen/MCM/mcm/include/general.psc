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
KeyBindSetting toggleCustomZOffset -> [
	settingName: "ToggleCustomZKeyCode"
	displayName: "Toggle Z Offset Key"
	desc: "When pressed, applies/removes an offset to the camera height, the amount being set by the 'Z Offset Amount' slider."
	page: " General"
]
SliderSetting customZOffsetAmount -> [
	settingName: "CustomZOffsetAmount"
	displayName: "Z Offset Amount"
	desc: "Amount to offset the Z axis (camera height) by when using pressing the key bound to 'Toggle Z Offset'."
	defaultValue: 0
	interval: 1
	min: -256
	max: 256
	displayFormat: "{0}"
	page: " General"
]
KeyBindSetting toggleUserDefinedOffset -> [
	settingName: "ToggleUserDefinedOffsetKeyCode"
	displayName: "Toggle Custom Offset Group"
	desc: "When pressed, enables/disable the 'Custom' offset group, overloading the normal camera state."
	page: " General"
]