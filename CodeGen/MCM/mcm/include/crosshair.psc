ToggleSetting crosshair3DBowEnabled -> [
	settingName: "Enable3DBowCrosshair"
	displayName: "3D Bow Crosshair Enabled"
	desc: "Enable the raycasted 3D crosshair when aiming with the bow."
	page: " Crosshair"
]
ToggleSetting crosshair3DMagicEnabled -> [
	settingName: "Enable3DMagicCrosshair"
	displayName: "3D Magic Crosshair Enabled"
	desc: "Enable the raycasted 3D crosshair when using magic."
	page: " Crosshair"
]
ToggleSetting crosshair3DWorldEnabled -> [
	settingName: "UseWorldCrosshair"
	displayName: "Use World-Space Crosshair"
	desc: "When your crosshair ray has hit something, use a crosshair mesh rendered in-world, not on the HUD."
	page: " Crosshair"
]
ToggleSetting use3DPicker -> [
	settingName: "Use3DPicker"
	displayName: "3D Activator Crosshair Enabled"
	desc: "Use the 3D crosshair when looking at items in the world."
	resetPage: true
	page: " Crosshair"
]
ListSetting worldCrosshairType -> [
	settingName: "WorldCrosshairType"
	displayName: "Crosshair Type"
	desc: "Select the style of world-space crosshair to use, if world-space crosshair is enabled."
	arrayType: crosshairTypes
	page: " Crosshair"
]
ToggleSetting worldCrosshairDepthTest -> [
	settingName: "WorldCrosshairDepthTest"
	displayName: "Crosshair Occlusion"
	desc: "When using the world-space crosshair, disable this option to make it draw on top of everything rather than allow other geometry to cover it."
	page: " Crosshair"
]
ToggleSetting hideCrosshairOutOfCombat -> [
	settingName: "HideCrosshairOutOfCombat"
	displayName: "Hide Non-Combat Crosshair"
	desc: "Hide the crosshair when not in combat."
	page: " Crosshair"
]
ToggleSetting hideCrosshairMeleeCombat -> [
	settingName: "HideCrosshairMeleeCombat"
	displayName: "Hide Melee Combat Crosshair"
	desc: "Hide the crosshair when in melee combat."
	page: " Crosshair"
]
ToggleSetting onlyShowCrosshairOnHit -> [
	settingName: "ShowCrosshairOnHtOnly"
	displayName: "Only Show Crosshair On Hit"
	desc: "Hide the crosshair at all times, unless the crosshair has hit something in the world. Overrides hide melee and hide non-combat options. Does nothing unless the 3D activator crosshair is also enabled."
	hasDependentControl: true
	dependency: use3DPicker
	page: " Crosshair"
]
ToggleSetting enableCrosshairSizeManip -> [
	settingName: "EnableCrosshairSizeManip"
	displayName: "Enable Size Manipulation"
	desc: "Changes the size of the crosshair, based on 'Min/Max Size' and 'NPC Hit Size'. Disable if this causes conflicts with other mods. May require a game restart."
	page: " Crosshair"
]
SliderSetting crosshairPickRadius -> [
	settingName: "ObjectPickRadius"
	displayName: "Activate Radius"
	desc: "Controls the activation distance between the crosshair and an object that can be interacted with."
	defaultValue: 16
	interval: 0.1
	min: 1
	max: 16
	displayFormat: "{1}"
	page: " Crosshair"
]
SliderSetting crosshairNPCGrowSize -> [
	settingName: "CrosshairNPCGrowSize"
	displayName: "NPC Hit Size"
	desc: "When the 3D crosshair is over an NPC, grow the size of the crosshair by this amount."
	defaultValue: 16
	interval: 1
	min: 0
	max: 64
	displayFormat: "{0}"
	page: " Crosshair"
]
SliderSetting crosshairMinDistSize -> [
	settingName: "CrosshairMinDistSize"
	displayName: "Min Crosshair Size"
	desc: "Sets the size of the 3D crosshair when the player's aim ray is at the maximum distance."
	defaultValue: 16
	interval: 1
	min: 8
	max: 64
	displayFormat: "{0}"
	page: " Crosshair"
]
SliderSetting crosshairMaxDistSize -> [
	settingName: "CrosshairMaxDistSize"
	displayName: "Max Crosshair Size"
	desc: "Sets the size of the 3D crosshair when the player's aim ray is at the minimum distance."
	defaultValue: 24
	interval: 1
	min: 8
	max: 128
	displayFormat: "{0}"
	page: " Crosshair"
]
ToggleSetting offsetStealthMeter -> [
	settingName: "OffsetStealthMeter"
	displayName: "Enable Stealth Meter Offset"
	desc: "When using the world-space crosshair and sneaking, offset the stealth meter by a set amount when the crosshair is active."
	page: " Crosshair"
]
ToggleSetting alwaysOffsetStealthMeter -> [
	settingName: "AlwaysOffsetStealthMeter"
	displayName: "Always Offset Stealth Meter"
	desc: "When selected, offset settings for the stealth meter will always be applied, even if the world-space crosshair is not active."
	page: " Crosshair"
]
SliderSetting stealthMeterOffsetX -> [
	settingName: "StealthMeterOffsetX"
	displayName: "Stealth Meter X Offset"
	desc: "Offset the position of the stealth meter by this amount when the world-space crosshair is active while sneaking."
	defaultValue: 0
	interval: 0.1
	min: -640
	max: 640
	displayFormat: "{1}"
	page: " Crosshair"
]
SliderSetting stealthMeterOffsetY -> [
	settingName: "StealthMeterOffsetY"
	displayName: "Stealth Meter Y Offset"
	desc: "Offset the position of the stealth meter by this amount when the world-space crosshair is active while sneaking."
	defaultValue: 0
	interval: 0.1
	min: -360
	max: 360
	displayFormat: "{1}"
	page: " Crosshair"
]
ToggleSetting enableArrowPrediction -> [
	settingName: "EnableArrowPrediction"
	displayName: "Enable Arrow Prediction"
	desc: "When the 3D crosshair is enabled for ranged combat, the crosshair will account for gravity when aiming with a bow."
	page: " Crosshair"
]
ToggleSetting drawArrowArc -> [
	settingName: "DrawArrowArc"
	displayName: "Draw Arrow Prediction Arc"
	desc: "When the 3D crosshair is enabled for ranged combat and 'Enable Arrow Prediction' is selected, an arc will be drawn while aiming with bows which indicates the flight path your arrow will take."
	page: " Crosshair"
]
ToggleSetting enableProjectileFixes -> [
	settingName: "EnableProjectileFixes"
	displayName: "Enable Projectile Fixes"
	desc: "When projectile fixes are enabled, projectile rotation will be modified to aim to where the crosshair points."
	page: " Crosshair"
]
SliderSetting maxArrowPredictionRange -> [
	settingName: "MaxArrowPredictionRange"
	displayName: "Max Arrow Prediction Distance"
	desc: "The furthest distance to allow arrow prediction, if enabled, to function."
	defaultValue: 10000
	interval: 1
	min: 500
	max: 12000
	displayFormat: "{0}"
	page: " Crosshair"
]
SliderSetting arrowArcColorR -> [
	settingName: "ArrowArcColorR"
	displayName: "Arrow Arc Color: Red"
	desc: "The amount of red coloration to add to the arrow arc."
	defaultValue: 255.0
	interval: 1
	min: 0.0
	max: 255.0
	displayFormat: "{0}"
	page: " Crosshair"
]
SliderSetting arrowArcColorG -> [
	settingName: "ArrowArcColorG"
	displayName: "Arrow Arc Color: Green"
	desc: "The amount of green coloration to add to the arrow arc."
	defaultValue: 255.0
	interval: 1
	min: 0.0
	max: 255.0
	displayFormat: "{0}"
	page: " Crosshair"
]
SliderSetting arrowArcColorB -> [
	settingName: "ArrowArcColorB"
	displayName: "Arrow Arc Color: Blue"
	desc: "The amount of blue coloration to add to the arrow arc."
	defaultValue: 255.0
	interval: 1
	min: 0.0
	max: 255.0
	displayFormat: "{0}"
	page: " Crosshair"
]
SliderSetting arrowArcColorA -> [
	settingName: "ArrowArcColorA"
	displayName: "Arrow Arc Color: Transparency"
	desc: "The amount of transparency coloration to add to the arrow arc."
	defaultValue: 127.0
	interval: 1
	min: 0.0
	max: 255.0
	displayFormat: "{0}"
	page: " Crosshair"
]