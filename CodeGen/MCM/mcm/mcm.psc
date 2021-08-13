ScriptName SmoothCamMCM extends SKI_ConfigBase
Import SKSE
string[] interpMethods
string[] presets
string[] crosshairTypes
string activePage
#DeclArena<int, 128> pagePool;

#include "include/native_functions.psc"

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

#include "include/presets.psc"
#include "include/general.psc"
#include "include/following.psc"
#include "include/crosshair.psc"
#include "include/offset_groups.psc"
#include "include/controls.psc"

#constexpr_struct ScriptMeta [
	int version = 0
	mangle string d3dHookString = "D3DHooked"
]

ScriptMeta scriptMetaInfo -> [
	version: 14
]

int Function GetVersion()
	return scriptMetaInfo.version
endFunction

event OnConfigInit()
	Pages = new string[] -> [
		" General", " Following", " Crosshair", " Standing",
		" Walking", " Running", " Sprinting", " Sneaking",
		" Swimming", " Bow Aiming", " Sitting", " Horseback",
		" Dragon", " Vampire Lord", " Werewolf", " Custom", " Group Edit", " Presets"
	]
	interpMethods = new string[] -> [
		"linear", "quadraticEaseIn", "quadraticEaseOut",
		"quadraticEaseInOut", "cubicEaseIn", "cubicEaseOut",
		"cubicEaseInOut", "quarticEaseIn", "quarticEaseOut",
		"quarticEaseInOut", "quinticEaseIn", "quinticEaseOut",
		"quinticEaseInOut", "sineEaseIn", "sineEaseOut",
		"sineEaseInOut", "circularEaseIn", "circularEaseOut",
		"circularEaseInOut", "exponentialEaseIn", "exponentialEaseOut",
		"exponentialEaseInOut"
	]
	crosshairTypes = new string[] -> [
		"Skyrim", "Dot"
	]
	#ImplArena(pagePool)
endEvent

event OnVersionUpdate(int version)
	OnConfigInit()
endEvent

event OnPageReset(string a_page)
	SetCursorFillMode(TOP_TO_BOTTOM)
	activePage = a_page

	if (a_page == " General")
		AddHeaderOption("Plugin Info")
		int version_T = AddTextOption("DLL Version", GetPluginVersion("SmoothCam"), OPTION_FLAG_DISABLED)
		int s_version_T = AddTextOption("MCM Script Version", scriptMetaInfo.version, OPTION_FLAG_DISABLED)
		int hasDXContext = AddTextOption("D3D11 Hooked", SmoothCam_GetBoolConfig(scriptMetaInfo.d3dHookString), OPTION_FLAG_DISABLED)

		AddHeaderOption("General")
		#StructInvokeOn(implControl, [
			modEnabled, modEnabledKey,
			toggleCustomZOffset, customZOffsetAmount,
			toggleUserDefinedOffset
		])

		SetCursorPosition(1)
		AddHeaderOption("Bug Fixes & Misc Options")
		#StructInvokeOn(implControl, [
			forceCameraState, resetCrosshair,
			reset, enableCrashDumps
		])

		AddHeaderOption("Compatibility")
		AddTextOption("Alternate Conversation Camera", SmoothCam_IsModDetected(2), OPTION_FLAG_DISABLED)
		AddTextOption("Archery Gameplay Overhaul", SmoothCam_IsModDetected(3), OPTION_FLAG_DISABLED)
		AddTextOption("Immersive First-Person View", SmoothCam_IsModDetected(1), OPTION_FLAG_DISABLED)
		AddTextOption("Improved Camera Beta4", SmoothCam_IsModDetected(0), OPTION_FLAG_DISABLED)

		int count = SmoothCam_NumAPIConsumers()
		While count
			count -= 1
			AddTextOption(SmoothCam_GetAPIConsumerName(count), "API Consumer", OPTION_FLAG_DISABLED)
		endWhile

	elseIf (a_page == " Following")
		AddHeaderOption("Interpolation")
		#StructInvokeOn(implControl, [
			interpEnabled, interpMethod, minCameraFollowDistance, minCameraFollowRate, maxCameraFollowRate, maxSmoothingInterpDistance
		])

		AddHeaderOption("Separate Z Interpolation")
		#StructInvokeOn(implControl, [
			sepZInterpEnabled, sepZInterpMethod, minSepZFollowRate, maxSepZFollowRate, maxSepZSmoothingDistance
		])

		AddHeaderOption("Local-Space Interpolation")
		#StructInvokeOn(implControl, [
			sepLocalInterpEnabled, sepLocalInterpMethod, minLocalFollowRate,
			maxLocalFollowRate, maxLocalSmoothingDistance
		])

		AddHeaderOption("Offset Interpolation")
		#StructInvokeOn(implControl, [
			offsetInterpEnabled, offsetInterpMethod, offsetTransitionDuration
		])

		AddHeaderOption("Zoom Interpolation")
		#StructInvokeOn(implControl, [
			zoomInterpEnabled, zoomInterpMethod, zoomTransitionDuration
		])

		AddHeaderOption("FOV Interpolation")
		#StructInvokeOn(implControl, [
			fovInterpEnabled, fovInterpMethod, fovTransitionDuration
		])

		SetCursorPosition(1)
		AddHeaderOption("Distance Clamping")
		#StructInvokeOn(implControl, [
			cameraDistanceClampXEnable, cameraDistanceClampXMin, cameraDistanceClampXMax,
			cameraDistanceClampYEnable, cameraDistanceClampYMin, cameraDistanceClampYMax,
			cameraDistanceClampZEnable, cameraDistanceClampZMin, cameraDistanceClampZMax
		])

		AddHeaderOption("Interpolator Blending")
		globalInterpDisableSmoothing->!implControl
		globalInterpDisableMethod->!implControl
		globalInterpOverrideSmoothing->!implControl
		globalInterpOverrideMethod->!implControl
		localInterpOverrideSmoothing->!implControl
		localInterpOverrideMethod->!implControl

		AddHeaderOption("Misc")
		#StructInvokeOn(implControl, [
			shoulderSwapKey, swapDistanceClampXAxis, nextPresetKey,
			zoomMul, disableDeltaTime
		])

	elseIf (a_page == " Crosshair")
		AddHeaderOption("3D Crosshair Settings")
		#StructInvokeOn(implControl, [
			crosshair3DBowEnabled, crosshair3DMagicEnabled,
			crosshair3DWorldEnabled, worldCrosshairType, worldCrosshairDepthTest,
			enableCrosshairSizeManip, crosshairMinDistSize,
			crosshairMaxDistSize, crosshairNPCGrowSize
		])

		AddHeaderOption("Crosshair Hiding")
		#StructInvokeOn(implControl, [
			hideCrosshairOutOfCombat, hideCrosshairMeleeCombat,
		])

		SetCursorPosition(1)
		AddHeaderOption("Archery Features")
		#StructInvokeOn(implControl, [
			enableArrowPrediction, drawArrowArc, maxArrowPredictionRange,
			arrowArcColorR, arrowArcColorG, arrowArcColorB,
			arrowArcColorA,
		])

		AddHeaderOption("Sneak Settings")
		#StructInvokeOn(implControl, [
			offsetStealthMeter, alwaysOffsetStealthMeter, stealthMeterOffsetX, stealthMeterOffsetY
		])

	elseIf (a_page == " Standing")
		#ImplOffsetGroupPage(Standing)

	elseIf (a_page == " Walking")
		#ImplOffsetGroupPage(Walking)

	elseIf (a_page == " Running")
		#ImplOffsetGroupPage(Running)

	elseIf (a_page == " Sprinting")
		#ImplOffsetGroupPage(Sprinting)

	elseIf (a_page == " Sneaking")
		#ImplOffsetGroupPage(Sneaking)

	elseIf (a_page == " Swimming")
		#ImplOffsetGroupPage(Swimming)

	elseIf (a_page == " Bow Aiming")
		#StructInvokeOn(implControl, [
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
		])

		SetCursorPosition(1)
		#StructInvokeOn(implControl, [
			bowaim_interp,
			bowaim_overrideInterp,
			bowaim_interpMethod,
			bowaim_minFollowRate,
			bowaim_maxFollowRate,
			bowaim_maxSmoothingInterpDistance,

			bowaim_overrideLocalInterp,
			bowaim_LocalnterpMethod,
			bowaim_localMinFollowRate,
			bowaim_localMaxFollowRate,
			bowaim_localMaxSmoothingInterpDistance,
			
			bowaimHorseback_interp,
			bowaimHorseback_overrideInterp,
			bowaimHorseback_interpMethod,
			bowaimHorseback_minFollowRate,
			bowaimHorseback_maxFollowRate,
			bowaimHorseback_maxSmoothingInterpDistance,

			bowaimHorseback_overrideLocalInterp,
			bowaimHorseback_LocalnterpMethod,
			bowaimHorseback_localMinFollowRate,
			bowaimHorseback_localMaxFollowRate,
			bowaimHorseback_localMaxSmoothingInterpDistance,
			
			bowaimSneaking_interp,
			bowaimSneaking_overrideInterp,
			bowaimSneaking_interpMethod,
			bowaimSneaking_minFollowRate,
			bowaimSneaking_maxFollowRate,
			bowaimSneaking_maxSmoothingInterpDistance,

			bowaimSneaking_overrideLocalInterp,
			bowaimSneaking_LocalnterpMethod,
			bowaimSneaking_localMinFollowRate,
			bowaimSneaking_localMaxFollowRate,
			bowaimSneaking_localMaxSmoothingInterpDistance,
		])

	elseIf (a_page == " Sitting")
		#ImplOffsetGroupPage(Sitting)

	elseIf (a_page == " Horseback")
		#ImplOffsetGroupPage(Horseback)

	elseIf (a_page == " Dragon")
		#ImplOffsetGroupPage(Dragon)

	elseIf (a_page == " Vampire Lord")
		#ImplOffsetGroupPage(VampireLord)

	elseIf (a_page == " Werewolf")
		#ImplOffsetGroupPage(Werewolf)

	elseIf (a_page == " Custom")
		#ImplOffsetGroupPage(Custom)

	elseIf (a_page == " Group Edit")
		AddHeaderOption("Edit All Offset Groups")
		#ImplOffsetGroupPage(Group, NoSliderHeader, NoInterpToggles)

	elseIf (a_page == " Presets")
		AddHeaderOption("Save Preset")
		#StructInvokeOn(implControl, [
			savePresetSlot1,
			savePresetSlot2,
			savePresetSlot3,
			savePresetSlot4,
			savePresetSlot5,
			savePresetSlot6
		])

		SetCursorPosition(1)
		AddHeaderOption("Load Preset")
		#StructInvokeOn(implControl, [
			loadPresetSlot1,
			loadPresetSlot2,
			loadPresetSlot3,
			loadPresetSlot4,
			loadPresetSlot5,
			loadPresetSlot6
		])

	endIf
endEvent
