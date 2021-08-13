ToggleSetting interpEnabled -> [
	settingName: "InterpolationEnabled"
	displayName: "Interpolation Enabled"
	desc: "Enable camera smoothing."
	page: " Following"
]
ToggleSetting sepZInterpEnabled -> [
	settingName: "SeparateZInterpEnabled"
	displayName: "Interpolation Enabled"
	desc: "Enable the separate Z smoothing settings for smoothing camera height differently."
	page: " Following"
]
ToggleSetting sepLocalInterpEnabled -> [
	settingName: "SeparateLocalInterpolation"
	displayName: "Interpolation Enabled"
	desc: "Enable separate local-space camera smoothing (Camera rotation)."
	page: " Following"
]
ToggleSetting offsetInterpEnabled -> [
	settingName: "OffsetTransitionEnabled"
	displayName: "Interpolation Enabled"
	desc: "Enable smoothing of camera offset state transitions."
	page: " Following"
]
ToggleSetting zoomInterpEnabled -> [
	settingName: "ZoomTransitionEnabled"
	displayName: "Interpolation Enabled"
	desc: "Enable smoothing of camera zoom state transitions."
	page: " Following"
]
ToggleSetting fovInterpEnabled -> [
	settingName: "FOVTransitionEnabled"
	displayName: "Interpolation Enabled"
	desc: "Enable smoothing of camera FOV state transitions."
	page: " Following"
]
ToggleSetting disableDeltaTime -> [
	settingName: "DisableDeltaTime"
	displayName: "Disable Delta Time Factoring"
	desc: "Remove time from interpolation math. May result in less jitter but can cause speed to vary with frame rate."
	page: " Following"
]
ToggleSetting cameraDistanceClampXEnable -> [
	settingName: "CameraDistanceClampXEnable"
	displayName: "Enable X Distance Clamp"
	desc: "Clamp the maximum distance the camera may move away from the target position along the X (side) axis."
	page: " Following"
]
ToggleSetting cameraDistanceClampYEnable -> [
	settingName: "CameraDistanceClampYEnable"
	displayName: "Enable Y Distance Clamp"
	desc: "Clamp the maximum distance the camera may move away from the target position along the Y (forward) axis."
	page: " Following"
]
ToggleSetting cameraDistanceClampZEnable -> [
	settingName: "CameraDistanceClampZEnable"
	displayName: "Enable Z Distance Clamp"
	desc: "Clamp the maximum distance the camera may move away from the target position along the Z (up) axis."
	page: " Following"
]
ToggleSetting swapDistanceClampXAxis -> [
	settingName: "ShoulderSwapXClamping"
	displayName: "Also Swap X Axis Clamping"
	desc: "When shoulder swapping, will also swap the distance clamping X axis range."
	page: " Following"
]
ListSetting interpMethod -> [
	settingName: "InterpolationMethod"
	displayName: "Method"
	desc: "The scalar method to use for camera smoothing."
	arrayType: interpMethods
	page: " Following"
]
ListSetting sepZInterpMethod -> [
	settingName: "SeparateZInterpMethod"
	displayName: "Method"
	desc: "The scalar method to use for smoothing the camera height (If enabled)."
	arrayType: interpMethods
	page: " Following"
]
ListSetting sepLocalInterpMethod -> [
	settingName: "SepLocalInterpMethod"
	displayName: "Method"
	desc: "The scalar method to use for local-space smoothing (If enabled)."
	arrayType: interpMethods
	page: " Following"
]
ListSetting offsetInterpMethod -> [
	settingName: "OffsetTransitionMethod"
	displayName: "Method"
	desc: "The scalar method to use for offset transition smoothing (If enabled)."
	arrayType: interpMethods
	page: " Following"
]
ListSetting zoomInterpMethod -> [
	settingName: "ZoomTransitionMethod"
	displayName: "Method"
	desc: "The scalar method to use for zoom transition smoothing (If enabled)."
	arrayType: interpMethods
	page: " Following"
]
ListSetting fovInterpMethod -> [
	settingName: "FOVTransitionMethod"
	displayName: "Method"
	desc: "The scalar method to use for FOV transition smoothing (If enabled)."
	arrayType: interpMethods
	page: " Following"
]
SliderSetting minCameraFollowRate -> [
	settingName: "MinCameraFollowRate"
	displayName: "Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the player."
	defaultValue: 0.5
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
]
SliderSetting maxCameraFollowRate -> [
	settingName: "MaxCameraFollowRate"
	displayName: "Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the player."
	defaultValue: 0.8
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
]
SliderSetting maxSmoothingInterpDistance -> [
	settingName: "MaxSmoothingInterpDistance"
	displayName: "Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 650.0
	interval: 1.0
	min: 1.0
	max: 650.0
	displayFormat: "{0}"
	page: " Following"
]
SliderSetting minCameraFollowDistance -> [
	settingName: "MinFollowDistance"
	displayName: "Min Follow Distance"
	desc: "The closest the camera may get to the player when at the lowest zoom level."
	defaultValue: 64.0
	interval: 0.1
	min: 0.0
	max: 256.0
	page: " Following"
]
SliderSetting zoomMul -> [
	settingName: "ZoomMul"
	displayName: "Zoom Multiplier"
	desc: "The amount of distance to add to the camera for each zoom level."
	defaultValue: 500.0
	interval: 1.0
	min: 1.0
	max: 500.0
	page: " Following"
]
SliderSetting minSepZFollowRate -> [
	settingName: "SepZMinFollowRate"
	displayName: "Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the player."
	defaultValue: 0.5
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
]
SliderSetting maxSepZFollowRate -> [
	settingName: "SepZMaxFollowRate"
	displayName: "Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the player."
	defaultValue: 0.5
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
]
SliderSetting maxSepZSmoothingDistance -> [
	settingName: "SepZMaxInterpDistance"
	displayName: "Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 100.0
	interval: 1.0
	min: 1.0
	max: 300.0
	displayFormat: "{0}"
	page: " Following"
]
SliderSetting offsetTransitionDuration -> [
	settingName: "OffsetTransitionDuration"
	displayName: "Interpolation Duration"
	desc: "The smoothing duration to use when the camera changes offsets (In seconds)."
	defaultValue: 1.0
	interval: 0.01
	min: 0.01
	max: 5.0
	displayFormat: "{2}"
	page: " Following"
]
SliderSetting zoomTransitionDuration -> [
	settingName: "ZoomTransitionDuration"
	displayName: "Interpolation Duration"
	desc: "The smoothing duration to use when the camera changes zoom distance (In seconds)."
	defaultValue: 0.2
	interval: 0.01
	min: 0.01
	max: 5.0
	displayFormat: "{2}"
	page: " Following"
]
SliderSetting fovTransitionDuration -> [
	settingName: "FOVTransitionDuration"
	displayName: "Interpolation Duration"
	desc: "The smoothing duration to use when the camera changes FOV (In seconds)."
	defaultValue: 0.2
	interval: 0.01
	min: 0.01
	max: 5.0
	displayFormat: "{2}"
	page: " Following"
]
SliderSetting cameraDistanceClampXMin -> [
	settingName: "CameraDistanceClampXMin"
	displayName: "Distance Clamp X Min"
	desc: "The minimal distance the camera may get from the target position along the X axis before being clamped."
	defaultValue: -75.0
	interval: 1.0
	min: -300.0
	max: 0.0
	displayFormat: "{0}"
	page: " Following"
]
SliderSetting cameraDistanceClampXMax -> [
	settingName: "CameraDistanceClampXMax"
	displayName: "Distance Clamp X Max"
	desc: "The maximal distance the camera may get from the target position along the X axis before being clamped."
	defaultValue: 75.0
	interval: 1.0
	min: 0.0
	max: 300.0
	displayFormat: "{0}"
	page: " Following"
]
SliderSetting cameraDistanceClampYMin -> [
	settingName: "CameraDistanceClampYMin"
	displayName: "Distance Clamp Y Min"
	desc: "The minimal distance the camera may get from the target position along the Y axis before being clamped."
	defaultValue: -100.0
	interval: 1.0
	min: -500.0
	max: 0.0
	displayFormat: "{0}"
	page: " Following"
]
SliderSetting cameraDistanceClampYMax -> [
	settingName: "CameraDistanceClampYMax"
	displayName: "Distance Clamp Y Max"
	desc: "The maximal distance the camera may get from the target position along the Y axis before being clamped."
	defaultValue: 100.0
	interval: 1.0
	min: 0.0
	max: 500.0
	displayFormat: "{0}"
	page: " Following"
]
SliderSetting cameraDistanceClampZMin -> [
	settingName: "CameraDistanceClampZMin"
	displayName: "Distance Clamp Z Min"
	desc: "The minimal distance the camera may get from the target position along the Z axis before being clamped."
	defaultValue: -50.0
	interval: 1.0
	min: -300.0
	max: 0.0
	displayFormat: "{0}"
	page: " Following"
]
SliderSetting cameraDistanceClampZMax -> [
	settingName: "CameraDistanceClampZMax"
	displayName: "Distance Clamp Z Max"
	desc: "The maximal distance the camera may get from the target position along the Z axis before being clamped."
	defaultValue: 50.0
	interval: 1.0
	min: 0.0
	max: 300.0
	displayFormat: "{0}"
	page: " Following"
]
SliderSetting minLocalFollowRate -> [
	settingName: "MinSepLocalFollowRate"
	displayName: "Min Follow Rate"
	desc: "The smoothing rate to use when the camera is close to the goal position."
	defaultValue: 1.0
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
]
SliderSetting maxLocalFollowRate -> [
	settingName: "MaxSepLocalFollowRate"
	displayName: "Max Follow Rate"
	desc: "The smoothing rate to use when the camera is far away from the goal position."
	defaultValue: 1.0
	interval: 0.01
	min: 0.01
	max: 1.0
	displayFormat: "{2}"
	page: " Following"
]
SliderSetting maxLocalSmoothingDistance -> [
	settingName: "SepLocalInterpDistance"
	displayName: "Max Interpolation Distance"
	desc: "The distance at which the max follow rate value is used for smoothing. Below this value a mix of min and max is used."
	defaultValue: 100.0
	interval: 1.0
	min: 1.0
	max: 300.0
	displayFormat: "{0}"
	page: " Following"
]
KeyBindSetting shoulderSwapKey -> [
	settingName: "ShoulderSwapKeyCode"
	displayName: "Shoulder Swap Key"
	desc: "Inverts the current X offset of the camera."
	page: " Following"
]
KeyBindSetting nextPresetKey -> [
	settingName: "NextPresetKeyCode"
	displayName: "Load Next Preset Key"
	desc: "Loads the next found valid preset. Note that SmoothCam doesn't save what preset you are currently using between games - When starting skyrim preset 1 will be assumed to be the active preset."
	page: " Following"
]
SliderSetting globalInterpDisableSmoothing -> [
	settingName: "GlobalInterpDisableSmoothing"
	displayName: "Interpolator Disable Smoothing"
	desc: "The smoothing duration (in seconds) to use when interpolation is disabled or enabled by a change in offset states."
	defaultValue: 2.0
	interval: 0.01
	min: 0.01
	max: 10.0
	displayFormat: "{2}"
	page: " Following"
]
ListSetting globalInterpDisableMethod -> [
	settingName: "GlobalInterpDisableMethod"
	displayName: "Interpolator Disable Method"
	desc: "The scalar method to use for smoothing transitions from interpolation to a disabled state and back."
	arrayType: interpMethods
	page: " Following"
]
SliderSetting globalInterpOverrideSmoothing -> [
	settingName: "GlobalInterpOverrideSmoothing"
	displayName: "Transition Smoothing"
	desc: "The smoothing duration (in seconds) to use when interpolation settings change (because of an override being made active or inactive)."
	defaultValue: 1.0
	interval: 0.01
	min: 0.01
	max: 10.0
	displayFormat: "{2}"
	page: " Following"
]
ListSetting globalInterpOverrideMethod -> [
	settingName: "GlobalInterpOverrideMethod"
	displayName: "Transition Method"
	desc: "The scalar method to use for smoothing interpolation setting changes."
	arrayType: interpMethods
	page: " Following"
]
SliderSetting localInterpOverrideSmoothing -> [
	settingName: "LocalInterpOverrideSmoothing"
	displayName: "Local Transition Smoothing"
	desc: "The smoothing duration (in seconds) to use when local-space interpolation settings change (because of an override being made active or inactive)."
	defaultValue: 0.66
	interval: 0.01
	min: 0.01
	max: 10.0
	displayFormat: "{2}"
	page: " Following"
]
ListSetting localInterpOverrideMethod -> [
	settingName: "LocalInterpOverrideMethod"
	displayName: "Local Transition Method"
	desc: "The scalar method to use for smoothing local-space interpolation setting changes."
	arrayType: interpMethods
	page: " Following"
]