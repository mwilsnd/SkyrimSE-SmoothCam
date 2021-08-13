; Standing
#CreateOffsetGroup(Standing, " Standing")
; Walking
#CreateOffsetGroup(Walking, " Walking")
; Running
#CreateOffsetGroup(Running, " Running")
; Sprinting
#CreateOffsetGroup(Sprinting, " Sprinting")
; Sneaking
#CreateOffsetGroup(Sneaking, " Sneaking")
; Swimming
#CreateOffsetGroup(Swimming, " Swimming", NoMelee, NoRanged, NoMagic)
; Sitting
#CreateOffsetGroup(Sitting, " Sitting", NoMelee, NoRanged, NoMagic)
; Horseback
#CreateOffsetGroup(Horseback, " Horseback")
; Dragon
#CreateOffsetGroup(Dragon, " Dragon", NoMelee, NoRanged, NoMagic)
; Vampire Lord
#CreateOffsetGroup(VampireLord, " Vampire Lord", NoRanged)
; Werewolf
#CreateOffsetGroup(Werewolf, " Werewolf", NoRanged, NoMagic)
; User-defined
#CreateOffsetGroup(Custom, " Custom")
; Group edit
#CreateOffsetGroup(Group, " Group Edit", NoInterpToggles)
; Bow aiming
SliderSetting bowaim_sideOffset -> [
	settingName: "Bowaim:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
	displayFormat: "{0}"
	page: " Bow Aiming"
	header: "Bow Aiming Offsets"
]
SliderSetting bowaim_upOffset -> [
	settingName: "Bowaim:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
	displayFormat: "{0}"
	page: " Bow Aiming"
]
SliderSetting bowaim_zoomOffset -> [
	settingName: "Bowaim:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
	displayFormat: "{0}"
	page: " Bow Aiming"
]
SliderSetting bowaim_fovOffset -> [
	settingName: "Bowaim:FOVOffset"
	displayName: "FOV Offset"
	desc: "The amount to offset the camera FOV by. Note this will be clamped to a lower bound of 10 and an upper bound of 170."
	min: -120.0
	max: 120.0
	displayFormat: "{0}"
	page: " Bow Aiming"
]
SliderSetting bowaim_sideOffsetHorseback -> [
	settingName: "BowaimHorse:SideOffset"
	displayName: "Horseback Side Offset"
	desc: "The amount to move the camera to the right when on horseback."
	defaultValue: 25.0
	displayFormat: "{0}"
	page: " Bow Aiming"
	header: "Bow Aiming Horseback Offsets"
]
SliderSetting bowaim_upOffsetHorseback -> [
	settingName: "BowaimHorse:UpOffset"
	displayName: "Horseback Up Offset"
	desc: "The amount to move the camera up when on horseback."
	displayFormat: "{0}"
	page: " Bow Aiming"
]
SliderSetting bowaim_zoomOffsetHorseback -> [
	settingName: "BowaimHorse:ZoomOffset"
	displayName: "Horseback Zoom Offset"
	desc: "The amount to offset the camera zoom by when on horseback."
	min: -200.0
	max: 200.0
	displayFormat: "{0}"
	page: " Bow Aiming"
]
SliderSetting bowaim_fovOffsetHorseback -> [
	settingName: "BowaimHorse:FOVOffset"
	displayName: "Horseback FOV Offset"
	desc: "The amount to offset the camera FOV by when on horseback. Note this will be clamped to a lower bound of 10 and an upper bound of 170."
	min: -120.0
	max: 120.0
	displayFormat: "{0}"
	page: " Bow Aiming"
]
SliderSetting bowaim_sideOffsetSneaking -> [
	settingName: "BowaimSneak:SideOffset"
	displayName: "Sneaking Side Offset"
	desc: "The amount to move the camera to the right when sneaking."
	defaultValue: 25.0
	displayFormat: "{0}"
	page: " Bow Aiming"
	header: "Bow Aiming Sneaking Offsets"
]
SliderSetting bowaim_upOffsetSneaking -> [
	settingName: "BowaimSneak:UpOffset"
	displayName: "Sneaking Up Offset"
	desc: "The amount to move the camera up when sneaking."
	displayFormat: "{0}"
	page: " Bow Aiming"
]
SliderSetting bowaim_zoomOffsetSneaking -> [
	settingName: "BowaimSneak:ZoomOffset"
	displayName: "Sneaking Zoom Offset"
	desc: "The amount to offset the camera zoom by when sneaking."
	min: -200.0
	max: 200.0
	displayFormat: "{0}"
	page: " Bow Aiming"
]
SliderSetting bowaim_fovOffsetSneaking -> [
	settingName: "BowaimSneak:FOVOffset"
	displayName: "Sneaking FOV Offset"
	desc: "The amount to offset the camera FOV by when sneaking. Note this will be clamped to a lower bound of 10 and an upper bound of 170."
	min: -120.0
	max: 120.0
	displayFormat: "{0}"
	page: " Bow Aiming"
]

; Bow aim interp
ToggleSetting bowaim_interp -> [
	settingName: "InterpBowAim"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
	page: " Bow Aiming"
	header: "Interpolation"
]
ToggleSetting bowaim_overrideInterp -> [
	settingName: "OverrideInterpBowAim"
	displayName: "Override Interpolation"
	desc: "Overrides interpolation values in this state."
	page: " Bow Aiming"
]
SliderSetting bowaim_minFollowRate -> [
	settingName: "MinFollowRateBowAim"
	displayName: "Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaim_maxFollowRate -> [
	settingName: "MaxFollowRateBowAim"
	displayName: "Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaim_maxSmoothingInterpDistance -> [
	settingName: "MaxSmoothingInterpDistanceBowAim"
	displayName: "Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 650.0
	min: 1.0
	max: 650.0
	page: " Bow Aiming"
]
ListSetting bowaim_interpMethod -> [
	settingName: "SelectedScalarBowAim"
	displayName: "Method"
	desc: "The scalar method to use for camera smoothing."
	arrayType: interpMethods
	page: " Bow Aiming"
]

; Bow aim local interp
ToggleSetting bowaim_overrideLocalInterp -> [
	settingName: "OverrideLocalInterpBowAim"
	displayName: "Override Local Interpolation"
	desc: "Overrides local-space interpolation values in this state."
	page: " Bow Aiming"
	header: "Local Interpolation"
]
SliderSetting bowaim_localMinFollowRate -> [
	settingName: "MinSepLocalFollowRateBowAim"
	displayName: "Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaim_localMaxFollowRate -> [
	settingName: "MaxSepLocalFollowRateBowAim"
	displayName: "Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaim_localMaxSmoothingInterpDistance -> [
	settingName: "MaxSepLocalSmoothingInterpDistanceBowAim"
	displayName: "Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 200.0
	min: 1.0
	max: 300.0
	page: " Bow Aiming"
]
ListSetting bowaim_LocalnterpMethod -> [
	settingName: "SelectedLocalScalarBowAim"
	displayName: "Method"
	desc: "The scalar method to use for camera smoothing."
	arrayType: interpMethods
	page: " Bow Aiming"
]


; Bow aim interp - Horseback
ToggleSetting bowaimHorseback_interp -> [
	settingName: "InterpBowAimHorseback"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
	page: " Bow Aiming"
	header: "Horseback Interpolation"
]
ToggleSetting bowaimHorseback_overrideInterp -> [
	settingName: "OverrideInterpBowAimHorseback"
	displayName: "Override Interpolation"
	desc: "Overrides interpolation values in this state."
	page: " Bow Aiming"
]
SliderSetting bowaimHorseback_minFollowRate -> [
	settingName: "MinFollowRateBowAimHorseback"
	displayName: "Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaimHorseback_maxFollowRate -> [
	settingName: "MaxFollowRateBowAimHorseback"
	displayName: "Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaimHorseback_maxSmoothingInterpDistance -> [
	settingName: "MaxSmoothingInterpDistanceBowAimHorseback"
	displayName: "Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 650.0
	min: 1.0
	max: 650.0
	page: " Bow Aiming"
]
ListSetting bowaimHorseback_interpMethod -> [
	settingName: "SelectedScalarBowAimHorseback"
	displayName: "Method"
	desc: "The scalar method to use for camera smoothing."
	arrayType: interpMethods
	page: " Bow Aiming"
]

; Bow aim local interp - horseback
ToggleSetting bowaimHorseback_overrideLocalInterp -> [
	settingName: "OverrideLocalInterpBowAimHorseback"
	displayName: "Override Local Interpolation"
	desc: "Overrides local-space interpolation values in this state."
	page: " Bow Aiming"
	header: "Horseback Local Interpolation"
]
SliderSetting bowaimHorseback_localMinFollowRate -> [
	settingName: "MinSepLocalFollowRateBowAimHorseback"
	displayName: "Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaimHorseback_localMaxFollowRate -> [
	settingName: "MaxSepLocalFollowRateBowAimHorseback"
	displayName: "Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaimHorseback_localMaxSmoothingInterpDistance -> [
	settingName: "MaxSepLocalSmoothingInterpDistanceBowAimHorseback"
	displayName: "Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 200.0
	min: 1.0
	max: 300.0
	page: " Bow Aiming"
]
ListSetting bowaimHorseback_LocalnterpMethod -> [
	settingName: "SelectedLocalScalarBowAimHorseback"
	displayName: "Method"
	desc: "The scalar method to use for camera smoothing."
	arrayType: interpMethods
	page: " Bow Aiming"
]


; Bow aim interp - Sneaking
ToggleSetting bowaimSneaking_interp -> [
	settingName: "InterpBowAimSneak"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
	page: " Bow Aiming"
	header: "Sneaking Interpolation"
]
ToggleSetting bowaimSneaking_overrideInterp -> [
	settingName: "OverrideInterpBowAimSneak"
	displayName: "Override Interpolation"
	desc: "Overrides interpolation values in this state."
	page: " Bow Aiming"
]
SliderSetting bowaimSneaking_minFollowRate -> [
	settingName: "MinFollowRateBowAimSneak"
	displayName: "Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaimSneaking_maxFollowRate -> [
	settingName: "MaxFollowRateBowAimSneak"
	displayName: "Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaimSneaking_maxSmoothingInterpDistance -> [
	settingName: "MaxSmoothingInterpDistanceBowAimSneak"
	displayName: "Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 650.0
	min: 1.0
	max: 650.0
	page: " Bow Aiming"
]
ListSetting bowaimSneaking_interpMethod -> [
	settingName: "SelectedScalarBowAimSneak"
	displayName: "Method"
	desc: "The scalar method to use for camera smoothing."
	arrayType: interpMethods
	page: " Bow Aiming"
]

; Bow aim local interp - sneaking
ToggleSetting bowaimSneaking_overrideLocalInterp -> [
	settingName: "OverrideLocalInterpBowAimSneak"
	displayName: "Override Local Interpolation"
	desc: "Overrides local-space interpolation values in this state."
	page: " Bow Aiming"
	header: "Sneaking Local Interpolation"
]
SliderSetting bowaimSneaking_localMinFollowRate -> [
	settingName: "MinSepLocalFollowRateBowAimSneak"
	displayName: "Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaimSneaking_localMaxFollowRate -> [
	settingName: "MaxSepLocalFollowRateBowAimSneak"
	displayName: "Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the player."
	defaultValue: 0.5
	min: 0.01
	max: 1.0
	interval: 0.01
	displayFormat: "{2}"
	page: " Bow Aiming"
]
SliderSetting bowaimSneaking_localMaxSmoothingInterpDistance -> [
	settingName: "MaxSepLocalSmoothingInterpDistanceBowAimSneak"
	displayName: "Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 200.0
	min: 1.0
	max: 300.0
	page: " Bow Aiming"
]
ListSetting bowaimSneaking_LocalnterpMethod -> [
	settingName: "SelectedLocalScalarBowAimSneak"
	displayName: "Method"
	desc: "The scalar method to use for camera smoothing."
	arrayType: interpMethods
	page: " Bow Aiming"
]