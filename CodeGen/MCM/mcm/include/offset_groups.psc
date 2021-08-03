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
; Group edit
#CreateOffsetGroup(Group, " Group Edit", NoInterpToggles)
; Bow aiming
SliderSetting bowaim_sideOffset -> [
	settingName: "Bowaim:SideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
	page: " Bow Aiming"
]
SliderSetting bowaim_upOffset -> [
	settingName: "Bowaim:UpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
	page: " Bow Aiming"
]
SliderSetting bowaim_zoomOffset -> [
	settingName: "Bowaim:ZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -200.0
	max: 200.0
	page: " Bow Aiming"
]
SliderSetting bowaim_fovOffset -> [
	settingName: "Bowaim:FOVOffset"
	displayName: "FOV Offset"
	desc: "The amount to offset the camera FOV by. Note this will be clamped to a lower bound of 10 and an upper bound of 170."
	min: -120.0
	max: 120.0
	page: " Bow Aiming"
]
SliderSetting bowaim_sideOffsetHorseback -> [
	settingName: "BowaimHorse:SideOffset"
	displayName: "Horseback Side Offset"
	desc: "The amount to move the camera to the right when on horseback."
	defaultValue: 25.0
	page: " Bow Aiming"
]
SliderSetting bowaim_upOffsetHorseback -> [
	settingName: "BowaimHorse:UpOffset"
	displayName: "Horseback Up Offset"
	desc: "The amount to move the camera up when on horseback."
	page: " Bow Aiming"
]
SliderSetting bowaim_zoomOffsetHorseback -> [
	settingName: "BowaimHorse:ZoomOffset"
	displayName: "Horseback Zoom Offset"
	desc: "The amount to offset the camera zoom by when on horseback."
	min: -200.0
	max: 200.0
	page: " Bow Aiming"
]
SliderSetting bowaim_fovOffsetHorseback -> [
	settingName: "BowaimHorse:FOVOffset"
	displayName: "Horseback FOV Offset"
	desc: "The amount to offset the camera FOV by when on horseback. Note this will be clamped to a lower bound of 10 and an upper bound of 170."
	min: -120.0
	max: 120.0
	page: " Bow Aiming"
]
SliderSetting bowaim_sideOffsetSneaking -> [
	settingName: "BowaimSneak:SideOffset"
	displayName: "Sneaking Side Offset"
	desc: "The amount to move the camera to the right when sneaking."
	defaultValue: 25.0
	page: " Bow Aiming"
]
SliderSetting bowaim_upOffsetSneaking -> [
	settingName: "BowaimSneak:UpOffset"
	displayName: "Sneaking Up Offset"
	desc: "The amount to move the camera up when sneaking."
	page: " Bow Aiming"
]
SliderSetting bowaim_zoomOffsetSneaking -> [
	settingName: "BowaimSneak:ZoomOffset"
	displayName: "Sneaking Zoom Offset"
	desc: "The amount to offset the camera zoom by when sneaking."
	min: -200.0
	max: 200.0
	page: " Bow Aiming"
]
SliderSetting bowaim_fovOffsetSneaking -> [
	settingName: "BowaimSneak:FOVOffset"
	displayName: "Sneaking FOV Offset"
	desc: "The amount to offset the camera FOV by when sneaking. Note this will be clamped to a lower bound of 10 and an upper bound of 170."
	min: -120.0
	max: 120.0
	page: " Bow Aiming"
]
ToggleSetting bowaim_interp -> [
	settingName: "InterpBowAim"
	displayName: "Enable Interpolation"
	desc: "Enables interpolation in this state."
	page: " Bow Aiming"
]
ToggleSetting bowaim_interpHorseback -> [
	settingName: "InterpBowAimHorseback"
	displayName: "Enable Horseback Interpolation"
	desc: "Enables interpolation in this state."
	page: " Bow Aiming"
]
ToggleSetting bowaim_interpSneaking -> [
	settingName: "InterpBowAimSneaking"
	displayName: "Enable Sneaking Interpolation"
	desc: "Enables interpolation in this state."
	page: " Bow Aiming"
]