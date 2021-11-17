ScriptName SmoothCamMCM extends SKI_ConfigBase
Import SKSE
string[] interpMethods
string[] presets
string[] crosshairTypes
string[] dialogueModes
string[] offsetGroups
string activePage
string activeOffsetGroup
string activeDialogueMode
#DeclArena<int, 128> pagePool

#include "include/native_functions.psc"
#include "include/util_functions.psc"
#include "include/presets.psc"
#include "include/general.psc"
#include "include/dialogue.psc"
#include "include/thirdperson.psc"
#include "include/crosshair.psc"
#include "include/offset_groups.psc"
#include "include/controls.psc"

#constexpr_struct ScriptMeta [
	int version = 0
	mangle string d3dHookString = "D3DHooked"
]

ScriptMeta scriptMetaInfo -> [
	version: 15
]

int Function GetVersion()
	return scriptMetaInfo.version
endFunction

event OnConfigInit()
	Pages = new string[] -> [
		" General", " Thirdperson", " Thirdperson Offsets", " Crosshair",
		" Dialogue", " Presets"
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
	;dialogueModes = new string[] -> [
	;	"Disabled", "Skyrim", "Oblivion", "Face To Face"
	;]
	dialogueModes = new string[] -> [
		"Disabled", "Skyrim"
	]
	offsetGroups = new string[] -> [
		" Standing", " Walking", " Running", " Sprinting", " Sneaking",
		" Swimming", " Bow Aiming", " Sitting", " Horseback", " Dragon",
		" Vampire Lord", " Werewolf", " Custom", " Vanity", " Group Edit"
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
			modEnabled, modEnabledKey, nextPresetKey
		])

		SetCursorPosition(1)
		AddHeaderOption("Bug Fixes & Misc Options")
		#StructInvokeOn(implControl, [
			forceCameraState, resetCrosshair,
			reset, enableCrashDumps
		])

		AddHeaderOption("Compatibility")
		AddTextOption("Archery Gameplay Overhaul", SmoothCam_IsModDetected(2), OPTION_FLAG_DISABLED)
		AddTextOption("Immersive First-Person View", SmoothCam_IsModDetected(1), OPTION_FLAG_DISABLED)
		AddTextOption("Improved Camera Beta4", SmoothCam_IsModDetected(0), OPTION_FLAG_DISABLED)

		int count = SmoothCam_NumAPIConsumers()
		While count
			count -= 1
			AddTextOption(SmoothCam_GetAPIConsumerName(count), "API Consumer", OPTION_FLAG_DISABLED)
		endWhile

	elseIf (a_page == " Thirdperson")
		AddHeaderOption("Interpolation")
		#StructInvokeOn(implControl, [
			interpEnabled, interpMethod, minCameraFollowRate, maxCameraFollowRate, maxSmoothingInterpDistance
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
		#StructInvokeOn(implControl, [
			globalInterpDisableSmoothing,
			globalInterpDisableMethod,
			globalInterpOverrideSmoothing,
			globalInterpOverrideMethod,
			localInterpOverrideSmoothing,
			localInterpOverrideMethod
		])

		AddHeaderOption("Pitch Zoom")
		#StructInvokeOn(implControl, [
			pitchZoomEnable, pitchZoomAfterInterp,
			pitchZoomMethod, pitchZoomMaxRange, pitchZoomMaxAngle
		])

		AddHeaderOption("Misc")
		#StructInvokeOn(implControl, [
			minCameraFollowDistance, zoomMul, shoulderSwapKey, swapDistanceClampXAxis,
			toggleCustomZOffset, customZOffsetAmount,
			toggleUserDefinedOffset, disableDeltaTime
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

	elseIf (a_page == " Dialogue")
		dialogueMode->!implControl
		activeDialogueMode = dialogueModes[GetCurrentArrayIndex(dialogueMode.settingName, dialogueModes)]

		if (activeDialogueMode == "Oblivion")
			AddHeaderOption("Oblivion Camera Settings")
			oblivionDialogueMaxFOV->!implControl
			oblivionDialogueFOVDurationIn->!implControl
			oblivionDialogueFOVDurationOut->!implControl
			oblivionDialogueRunFPV->!implControl
		elseIf (activeDialogueMode == "Face To Face")
			AddHeaderOption("Face To Face Camera Settings")
			faceToFaceSideOffset->!implControl
			faceToFaceUpOffset->!implControl
			faceToFaceZoomOffset->!implControl
			faceToFaceRotationDuration->!implControl
			faceToFaceDurationIn->!implControl
			faceToFaceDurationOut->!implControl
			faceToFaceNoSwitch->!implControl
			faceToFaceForceThirdperson->!implControl
		else
			AddTextOption("This mode has no options", "", OPTION_FLAG_DISABLED)
		endIf

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
	
	elseIf (a_page == " Thirdperson Offsets")
		if (activeOffsetGroup == "")
			activeOffsetGroup = " Standing"
		endIf

		offsetGroupPicker->!implControl

		if (activeOffsetGroup == " Standing")
			#ImplOffsetGroupPage(Standing)
		elseIf (activeOffsetGroup == " Walking")
			#ImplOffsetGroupPage(Walking)
		elseIf (activeOffsetGroup == " Running")
			#ImplOffsetGroupPage(Running)
		elseIf (activeOffsetGroup == " Sprinting")
			#ImplOffsetGroupPage(Sprinting)
		elseIf (activeOffsetGroup == " Sneaking")
			#ImplOffsetGroupPage(Sneaking)
		elseIf (activeOffsetGroup == " Swimming")
			#ImplOffsetGroupPage(Swimming)
		elseIf (activeOffsetGroup == " Bow Aiming")
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
		elseIf (activeOffsetGroup == " Sitting")
			#ImplOffsetGroupPage(Sitting)
		elseIf (activeOffsetGroup == " Horseback")
			#ImplOffsetGroupPage(Horseback)
		elseIf (activeOffsetGroup == " Dragon")
			#ImplOffsetGroupPage(Dragon)
		elseIf (activeOffsetGroup == " Vampire Lord")
			#ImplOffsetGroupPage(VampireLord)
		elseIf (activeOffsetGroup == " Werewolf")
			#ImplOffsetGroupPage(Werewolf)
		elseIf (activeOffsetGroup == " Vanity")
			#ImplOffsetGroupPage(Vanity, NoInterpToggles)
		elseIf (activeOffsetGroup == " Custom")
			#ImplOffsetGroupPage(Custom)
		elseIf (activeOffsetGroup == " Group Edit")
			AddHeaderOption("Edit All Offset Groups")
			#ImplOffsetGroupPage(Group, NoSliderHeader, NoInterpToggles)
		endIf
	endIf
endEvent
