#include "papyrus.h"
#include "camera.h"
#include "thirdperson.h"
#include "crosshair.h"
#include "compat.h"
#include "debug/eh.h"

#pragma warning(push)
#pragma warning(disable : 5103)

extern Compat::ModDetectionFlags modDetectionFlags;
extern Config::UserConfig currentConfig;
extern eastl::unique_ptr<Camera::Camera> g_theCamera;

using namespace PapyrusBindings;
#define PAPYRUS_MANGLE(VarName)			\
	"_^@"##VarName##"_SmoothCamSetting"

#define IMPL_GETTER(VarName, Var)                   \
    { PAPYRUS_MANGLE(VarName), &currentConfig.Var }

#define IMPL_NCONF_GETTER(VarName, Var) \
    { PAPYRUS_MANGLE(VarName), &Var }

#define IMPL_SCALAR_METHOD_GETTER(VarName, Var)             \
    { PAPYRUS_MANGLE(VarName), []() {                       \
        const auto it = Config::scalarMethodRevLookup.find( \
            Config::GetCurrentConfig()->Var                 \
        );                                                  \
        if (it != Config::scalarMethodRevLookup.end())      \
            return RE::BSFixedString(it->second.c_str());   \
        else                                                \
            return RE::BSFixedString("linear");             \
    } }

#define IMPL_SCALAR_METHOD_SETTER(VarName, Var)                     \
    { PAPYRUS_MANGLE(VarName), [](RE::BSFixedString& str) {         \
		const auto upper = Util::UpperCase(str);					\
        const auto it = Config::scalarMethods.find(upper.c_str());  \
        if (it != Config::scalarMethods.end()) {                    \
            Config::GetCurrentConfig()->Var = it->second;           \
        }                                                           \
    } }

#define IMPL_OFFSET_GROUP_BOOL_GETTERS(GroupVarName, GroupNamePrefix)															\
	IMPL_GETTER("Interp"##GroupNamePrefix,									GroupVarName.interp),								\
	IMPL_GETTER("OverrideInterp"##GroupNamePrefix,							GroupVarName.interpConf.overrideInterp),			\
	IMPL_GETTER("OverrideLocalInterp"##GroupNamePrefix,						GroupVarName.interpConf.overrideLocalInterp),		\
																																\
	IMPL_GETTER("Interp"##GroupNamePrefix##"RangedCombat",					GroupVarName.interpRangedCombat),					\
	IMPL_GETTER("OverrideInterp"##GroupNamePrefix##"RangedCombat",			GroupVarName.interpRangedConf.overrideInterp),		\
	IMPL_GETTER("OverrideLocalInterp"##GroupNamePrefix##"RangedCombat",		GroupVarName.interpRangedConf.overrideLocalInterp),	\
																																\
	IMPL_GETTER("Interp"##GroupNamePrefix##"MagicCombat",					GroupVarName.interpMagicCombat),					\
	IMPL_GETTER("OverrideInterp"##GroupNamePrefix##"MagicCombat",			GroupVarName.interpMagicConf.overrideInterp),		\
	IMPL_GETTER("OverrideLocalInterp"##GroupNamePrefix##"MagicCombat",		GroupVarName.interpMagicConf.overrideLocalInterp),	\
																																\
	IMPL_GETTER("Interp"##GroupNamePrefix##"MeleeCombat",					GroupVarName.interpMeleeCombat),					\
	IMPL_GETTER("OverrideInterp"##GroupNamePrefix##"MeleeCombat",			GroupVarName.interpMeleeConf.overrideInterp),		\
	IMPL_GETTER("OverrideLocalInterp"##GroupNamePrefix##"MeleeCombat",		GroupVarName.interpMeleeConf.overrideLocalInterp)

#define IMPL_INTERP_STATE_SLIDERS(GroupNamePrefix, GroupVarName, VaPrefix, InterpGroup)													\
	IMPL_GETTER(GroupNamePrefix##VaPrefix##":MinFollowRate",					GroupVarName.InterpGroup.minCameraFollowRate),			\
	IMPL_GETTER(GroupNamePrefix##VaPrefix##":MaxFollowRate",					GroupVarName.InterpGroup.maxCameraFollowRate),			\
	IMPL_GETTER(GroupNamePrefix##VaPrefix##":MaxSmoothingInterpDistance",		GroupVarName.InterpGroup.zoomMaxSmoothingDistance),		\
	IMPL_GETTER(GroupNamePrefix##VaPrefix##":MinSepLocalFollowRate",			GroupVarName.InterpGroup.localMinFollowRate),			\
	IMPL_GETTER(GroupNamePrefix##VaPrefix##":MaxSepLocalFollowRate",			GroupVarName.InterpGroup.localMaxFollowRate),			\
	IMPL_GETTER(GroupNamePrefix##VaPrefix##":SepLocalInterpDistance",			GroupVarName.InterpGroup.localMaxSmoothingDistance)

#define IMPL_OFFSET_GROUP_FLOAT_GETTERS(GroupVarName, GroupNamePrefix)											\
	IMPL_GETTER(GroupNamePrefix##":SideOffset", GroupVarName.sideOffset),										\
	IMPL_GETTER(GroupNamePrefix##":UpOffset", GroupVarName.upOffset),											\
	IMPL_GETTER(GroupNamePrefix##":ZoomOffset", GroupVarName.zoomOffset),										\
	IMPL_GETTER(GroupNamePrefix##":FOVOffset", GroupVarName.fovOffset),											\
	IMPL_GETTER(GroupNamePrefix##"Combat:Ranged:SideOffset", GroupVarName.combatRangedSideOffset),				\
	IMPL_GETTER(GroupNamePrefix##"Combat:Ranged:UpOffset", GroupVarName.combatRangedUpOffset),					\
	IMPL_GETTER(GroupNamePrefix##"Combat:Ranged:ZoomOffset", GroupVarName.combatRangedZoomOffset),				\
	IMPL_GETTER(GroupNamePrefix##"Combat:Ranged:FOVOffset", GroupVarName.combatRangedFOVOffset),				\
	IMPL_GETTER(GroupNamePrefix##"Combat:Magic:SideOffset", GroupVarName.combatMagicSideOffset),				\
	IMPL_GETTER(GroupNamePrefix##"Combat:Magic:UpOffset", GroupVarName.combatMagicUpOffset),					\
	IMPL_GETTER(GroupNamePrefix##"Combat:Magic:ZoomOffset", GroupVarName.combatMagicZoomOffset),				\
	IMPL_GETTER(GroupNamePrefix##"Combat:Magic:FOVOffset", GroupVarName.combatMagicFOVOffset),					\
	IMPL_GETTER(GroupNamePrefix##"Combat:Melee:SideOffset", GroupVarName.combatMeleeSideOffset),				\
	IMPL_GETTER(GroupNamePrefix##"Combat:Melee:UpOffset", GroupVarName.combatMeleeUpOffset),					\
	IMPL_GETTER(GroupNamePrefix##"Combat:Melee:ZoomOffset", GroupVarName.combatMeleeZoomOffset),				\
	IMPL_GETTER(GroupNamePrefix##"Combat:Melee:FOVOffset", GroupVarName.combatMeleeFOVOffset),					\
	IMPL_INTERP_STATE_SLIDERS(GroupNamePrefix, GroupVarName, "", interpConf),									\
	IMPL_INTERP_STATE_SLIDERS(GroupNamePrefix, GroupVarName, "RangedCombat", interpRangedConf),					\
	IMPL_INTERP_STATE_SLIDERS(GroupNamePrefix, GroupVarName, "MagicCombat", interpMagicConf),					\
	IMPL_INTERP_STATE_SLIDERS(GroupNamePrefix, GroupVarName, "MeleeCombat", interpMeleeConf),					\
	IMPL_INTERP_STATE_SLIDERS(GroupNamePrefix, GroupVarName, "Horseback", interpHorsebackConf)

#define IMPL_OFFSET_GROUP_SCALAR_GETTERS(GroupVarName, GroupNamePrefix)																		\
	IMPL_SCALAR_METHOD_GETTER("SelectedScalar"##GroupNamePrefix, GroupVarName.interpConf.currentScalar),									\
	IMPL_SCALAR_METHOD_GETTER("SelectedLocalScalar"##GroupNamePrefix, GroupVarName.interpConf.separateLocalScalar),							\
	IMPL_SCALAR_METHOD_GETTER("SelectedScalar"##GroupNamePrefix##"RangedCombat", GroupVarName.interpRangedConf.currentScalar),				\
	IMPL_SCALAR_METHOD_GETTER("SelectedLocalScalar"##GroupNamePrefix##"RangedCombat", GroupVarName.interpRangedConf.separateLocalScalar),	\
	IMPL_SCALAR_METHOD_GETTER("SelectedScalar"##GroupNamePrefix##"MagicCombat", GroupVarName.interpMagicConf.currentScalar),				\
	IMPL_SCALAR_METHOD_GETTER("SelectedLocalScalar"##GroupNamePrefix##"MagicCombat", GroupVarName.interpMagicConf.separateLocalScalar),		\
	IMPL_SCALAR_METHOD_GETTER("SelectedScalar"##GroupNamePrefix##"MeleeCombat", GroupVarName.interpMeleeConf.currentScalar),				\
	IMPL_SCALAR_METHOD_GETTER("SelectedLocalScalar"##GroupNamePrefix##"MeleeCombat", GroupVarName.interpMeleeConf.separateLocalScalar),		\
	IMPL_SCALAR_METHOD_GETTER("SelectedScalar"##GroupNamePrefix##"Horseback", GroupVarName.interpHorsebackConf.currentScalar),				\
	IMPL_SCALAR_METHOD_GETTER("SelectedLocalScalar"##GroupNamePrefix##"Horseback", GroupVarName.interpHorsebackConf.separateLocalScalar)

#define IMPL_OFFSET_GROUP_SCALAR_SETTERS(GroupVarName, GroupNamePrefix)																		\
	IMPL_SCALAR_METHOD_SETTER("SelectedScalar"##GroupNamePrefix, GroupVarName.interpConf.currentScalar),									\
	IMPL_SCALAR_METHOD_SETTER("SelectedLocalScalar"##GroupNamePrefix, GroupVarName.interpConf.separateLocalScalar),							\
	IMPL_SCALAR_METHOD_SETTER("SelectedScalar"##GroupNamePrefix##"RangedCombat", GroupVarName.interpRangedConf.currentScalar),				\
	IMPL_SCALAR_METHOD_SETTER("SelectedLocalScalar"##GroupNamePrefix##"RangedCombat", GroupVarName.interpRangedConf.separateLocalScalar),	\
	IMPL_SCALAR_METHOD_SETTER("SelectedScalar"##GroupNamePrefix##"MagicCombat", GroupVarName.interpMagicConf.currentScalar),				\
	IMPL_SCALAR_METHOD_SETTER("SelectedLocalScalar"##GroupNamePrefix##"MagicCombat", GroupVarName.interpMagicConf.separateLocalScalar),		\
	IMPL_SCALAR_METHOD_SETTER("SelectedScalar"##GroupNamePrefix##"MeleeCombat", GroupVarName.interpMeleeConf.currentScalar),				\
	IMPL_SCALAR_METHOD_SETTER("SelectedLocalScalar"##GroupNamePrefix##"MeleeCombat", GroupVarName.interpMeleeConf.separateLocalScalar),		\
	IMPL_SCALAR_METHOD_SETTER("SelectedScalar"##GroupNamePrefix##"Horseback", GroupVarName.interpHorsebackConf.currentScalar),				\
	IMPL_SCALAR_METHOD_SETTER("SelectedLocalScalar"##GroupNamePrefix##"Horseback", GroupVarName.interpHorsebackConf.separateLocalScalar)

#define IMPL_GROUP_SETTER(VarName, Var, Type)   \
    { PAPYRUS_MANGLE(VarName), [](Type arg) {   \
        auto cfg = Config::GetCurrentConfig();  \
        cfg->standing.Var = arg;                \
        cfg->walking.Var = arg;                 \
        cfg->running.Var = arg;                 \
        cfg->sprinting.Var = arg;               \
        cfg->sneaking.Var = arg;                \
        cfg->swimming.Var = arg;                \
        cfg->bowAim.Var = arg;                  \
        cfg->sitting.Var = arg;                 \
        cfg->horseback.Var = arg;               \
        cfg->dragon.Var = arg;                  \
		cfg->vampireLord.Var = arg;				\
		cfg->werewolf.Var = arg;				\
    } }

// Bool
constexpr auto boolGetters = mapbox::eternal::hash_map<mapbox::eternal::string, bool*>({
	// Misc
	IMPL_GETTER("ModEnabled",						  modDisabled),
	IMPL_GETTER("EnableCrashDumps",					  enableCrashDumps),
	// Comapt
	IMPL_NCONF_GETTER("ICCompat",					  modDetectionFlags.bImprovedCamera),
	IMPL_NCONF_GETTER("IFPVCompat",					  modDetectionFlags.bIFPV),
	IMPL_NCONF_GETTER("AGOCompat",					  modDetectionFlags.bAGO),
	// Crosshair
	IMPL_GETTER("Enable3DBowCrosshair",				  use3DBowAimCrosshair),
	IMPL_GETTER("Enable3DMagicCrosshair",			  use3DMagicCrosshair),
	IMPL_GETTER("UseWorldCrosshair",				  useWorldCrosshair),
	IMPL_GETTER("WorldCrosshairDepthTest",			  worldCrosshairDepthTest),
	IMPL_GETTER("EnableArrowPrediction",			  useArrowPrediction),
	IMPL_GETTER("DrawArrowArc",						  drawArrowArc),
	IMPL_GETTER("EnableProjectileFixes",			  useProjectileFixes),
	IMPL_GETTER("EnableCrosshairSizeManip",			  enableCrosshairSizeManip),
	IMPL_GETTER("HideCrosshairOutOfCombat",			  hideNonCombatCrosshair),
	IMPL_GETTER("HideCrosshairMeleeCombat",			  hideCrosshairMeleeCombat),
	IMPL_GETTER("OffsetStealthMeter",				  offsetStealthMeter),
	IMPL_GETTER("AlwaysOffsetStealthMeter",			  alwaysOffsetStealthMeter),
	IMPL_GETTER("Use3DPicker",						  use3DPicker),
	IMPL_GETTER("ShowCrosshairOnHtOnly",			  onlyShowCrosshairOnHit),
	// Primary interpolation
	IMPL_GETTER("InterpolationEnabled",				  enableInterp),
	IMPL_GETTER("DisableDeltaTime",					  disableDeltaTime),
	// Separate local interpolation
	IMPL_GETTER("SeparateLocalInterpolation",		  separateLocalInterp),
	// Separate Z interpolation
	IMPL_GETTER("SeparateZInterpEnabled",			  separateZInterp),
	// Offset interpolation
	IMPL_GETTER("OffsetTransitionEnabled",			  enableOffsetInterpolation),
	// Zoom interpolation
	IMPL_GETTER("ZoomTransitionEnabled",			  enableZoomInterpolation),
	// FOV interpolation
	IMPL_GETTER("FOVTransitionEnabled",				  enableFOVInterpolation),
	// Dialogue
	IMPL_GETTER("OblivionDialogueRunFPV",             oblivionDialogue.runInFirstPerson),
	IMPL_GETTER("FaceToFaceDialogueForceThirdperson", faceToFaceDialogue.forceThirdPerson),
	// Distance clamping
	IMPL_GETTER("CameraDistanceClampXEnable",		  cameraDistanceClampXEnable),
	IMPL_GETTER("CameraDistanceClampYEnable",		  cameraDistanceClampYEnable),
	IMPL_GETTER("CameraDistanceClampZEnable",		  cameraDistanceClampZEnable),
	IMPL_GETTER("ShoulderSwapXClamping",			  swapXClamping),
	// Pitch zoom
	IMPL_GETTER("PitchZoomEnabled",			          enablePitchZoom),
	IMPL_GETTER("PitchZoomAfterInterp",			      pitchZoomAfterInterp),
	// Offset groups
	IMPL_OFFSET_GROUP_BOOL_GETTERS(standing, "Standing"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(walking, "Walking"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(running, "Running"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(sprinting, "Sprinting"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(sneaking, "Sneaking"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(swimming, "Swimming"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(sitting, "Sitting"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(horseback, "Horseback"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(dragon, "Dragon"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(vampireLord, "VampireLord"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(werewolf, "Werewolf"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(vanity, "Vanity"),
	IMPL_OFFSET_GROUP_BOOL_GETTERS(userDefined, "Custom"),
	// BowAim
	IMPL_GETTER("InterpBowAim",							bowAim.interpRangedCombat),
	IMPL_GETTER("OverrideInterpBowAim",					bowAim.interpRangedConf.overrideInterp),
	IMPL_GETTER("OverrideLocalInterpBowAim",			bowAim.interpRangedConf.overrideLocalInterp),
	IMPL_GETTER("InterpBowAimHorseback",				bowAim.interpHorseback),
	IMPL_GETTER("OverrideInterpBowAimHorseback",		bowAim.interpHorsebackConf.overrideInterp),
	IMPL_GETTER("OverrideLocalInterpBowAimHorseback",	bowAim.interpHorsebackConf.overrideLocalInterp),
	// @Note:BowAim: Just repurpose another combat group for sneaking
	IMPL_GETTER("InterpBowAimSneak",					bowAim.interpMeleeCombat),
	IMPL_GETTER("OverrideInterpBowAimSneak",			bowAim.interpMeleeConf.overrideInterp),
	IMPL_GETTER("OverrideLocalInterpBowAimSneak",		bowAim.interpMeleeConf.overrideLocalInterp),
});

const eastl::unordered_map<eastl::string_view, eastl::function<bool(void)>> boolGetterFNs = {
	{
		PAPYRUS_MANGLE("D3DHooked"), []() noexcept {
			return Render::HasContext();
		}
	},
};

// Float
const auto floatGetters = eastl::unordered_map<eastl::string_view, float*>({
	// Misc
	IMPL_GETTER("CustomZOffsetAmount",					customZOffset),
	// Primary interpolation
	IMPL_GETTER("MinFollowDistance",					minCameraFollowDistance),
	IMPL_GETTER("MinCameraFollowRate",					minCameraFollowRate),
	IMPL_GETTER("MaxCameraFollowRate",					maxCameraFollowRate),
	IMPL_GETTER("MaxSmoothingInterpDistance",			zoomMaxSmoothingDistance),
	IMPL_GETTER("ZoomMul",								zoomMul),
	// Crosshair
	IMPL_GETTER("CrosshairNPCGrowSize",					crosshairNPCHitGrowSize),
	IMPL_GETTER("CrosshairMinDistSize",					crosshairMinDistSize),
	IMPL_GETTER("CrosshairMaxDistSize",					crosshairMaxDistSize),
	IMPL_GETTER("ArrowArcColorR",						arrowArcColor.r),
	IMPL_GETTER("ArrowArcColorG",						arrowArcColor.g),
	IMPL_GETTER("ArrowArcColorB",						arrowArcColor.b),
	IMPL_GETTER("ArrowArcColorA",						arrowArcColor.a),
	IMPL_GETTER("MaxArrowPredictionRange",				maxArrowPredictionRange),
	IMPL_GETTER("StealthMeterOffsetX",					stealthMeterXOffset),
	IMPL_GETTER("StealthMeterOffsetY",					stealthMeterYOffset),
	IMPL_GETTER("ObjectPickRadius",						crosshairPickRadius),
	// Separate local interpolation
	IMPL_GETTER("MinSepLocalFollowRate",				localMinFollowRate),
	IMPL_GETTER("MaxSepLocalFollowRate",				localMaxFollowRate),
	IMPL_GETTER("SepLocalInterpDistance",				localMaxSmoothingDistance),
	// Separate Z interpolation
	IMPL_GETTER("SepZMaxInterpDistance",				separateZMaxSmoothingDistance),
	IMPL_GETTER("SepZMinFollowRate",					separateZMinFollowRate),
	IMPL_GETTER("SepZMaxFollowRate",					separateZMaxFollowRate),
	// Offset interpolation
	IMPL_GETTER("OffsetTransitionDuration",				offsetInterpDurationSecs),
	// Zoom interpolation
	IMPL_GETTER("ZoomTransitionDuration",				zoomInterpDurationSecs),
	// FOV interpolation
	IMPL_GETTER("FOVTransitionDuration",				fovInterpDurationSecs),
	// Dialogue
	IMPL_GETTER("OblivionDialogueMaxFOV",               oblivionDialogue.fovOffset),
	IMPL_GETTER("OblivionDialogueFOVDurationIn",        oblivionDialogue.zoomInDuration),
	IMPL_GETTER("OblivionDialogueFOVDurationOut",       oblivionDialogue.zoomOutDuration),
	IMPL_GETTER("FaceToFaceDialogueSideOffset",         faceToFaceDialogue.sideOffset),
	IMPL_GETTER("FaceToFaceDialogueUpOffset",           faceToFaceDialogue.upOffset),
	IMPL_GETTER("FaceToFaceDialogueZoomOffset",         faceToFaceDialogue.zoomOffset),
	IMPL_GETTER("FaceToFaceDialogueRotationDuration",   faceToFaceDialogue.rotationDuration),
	IMPL_GETTER("FaceToFaceDialogueDurationIn",         faceToFaceDialogue.zoomInDuration),
	IMPL_GETTER("FaceToFaceDialogueDurationOut",        faceToFaceDialogue.zoomOutDuration),
	// Pitch zoom
	IMPL_GETTER("PitchZoomMaxRange",				    pitchZoomMax),
	IMPL_GETTER("PitchZoomMaxAngle",				    pitchZoomMaxAngle),
	// Distance clamping
	IMPL_GETTER("CameraDistanceClampXMin",				cameraDistanceClampXMin),
	IMPL_GETTER("CameraDistanceClampXMax",				cameraDistanceClampXMax),
	IMPL_GETTER("CameraDistanceClampYMin",				cameraDistanceClampYMin),
	IMPL_GETTER("CameraDistanceClampYMax",				cameraDistanceClampYMax),
	IMPL_GETTER("CameraDistanceClampZMin",				cameraDistanceClampZMin),
	IMPL_GETTER("CameraDistanceClampZMax",				cameraDistanceClampZMax),
	// Interp smoothing/blending
	IMPL_GETTER("GlobalInterpDisableSmoothing",			globalInterpDisableSmoothing),
	IMPL_GETTER("GlobalInterpOverrideSmoothing",		globalInterpOverrideSmoothing),
	IMPL_GETTER("LocalInterpOverrideSmoothing",			localInterpOverrideSmoothing),
	// Offset groups
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(standing, "Standing"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(walking, "Walking"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(running, "Running"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(sprinting, "Sprinting"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(sneaking, "Sneaking"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(horseback, "Horseback"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(swimming, "Swimming"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(sitting, "Sitting"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(dragon, "Dragon"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(vampireLord, "VampireLord"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(werewolf, "Werewolf"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(vanity, "Vanity"),
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(userDefined, "Custom"),
	// BowAim
	IMPL_GETTER("Bowaim:SideOffset",						bowAim.sideOffset),
	IMPL_GETTER("Bowaim:UpOffset",							bowAim.upOffset),
	IMPL_GETTER("Bowaim:ZoomOffset",						bowAim.zoomOffset),
	IMPL_GETTER("Bowaim:FOVOffset",							bowAim.fovOffset),
	IMPL_GETTER("MinFollowRateBowAim",						bowAim.interpRangedConf.minCameraFollowRate),
	IMPL_GETTER("MaxFollowRateBowAim",						bowAim.interpRangedConf.maxCameraFollowRate),
	IMPL_GETTER("MaxSmoothingInterpDistanceBowAim",			bowAim.interpRangedConf.zoomMaxSmoothingDistance),
	IMPL_GETTER("MinSepLocalFollowRateBowAim",				bowAim.interpRangedConf.localMinFollowRate),
	IMPL_GETTER("MaxSepLocalFollowRateBowAim",				bowAim.interpRangedConf.localMaxFollowRate),
	IMPL_GETTER("MaxSepLocalSmoothingInterpDistanceBowAim",	bowAim.interpRangedConf.localMaxSmoothingDistance),

	IMPL_GETTER("BowaimHorse:SideOffset",								bowAim.horseSideOffset),
	IMPL_GETTER("BowaimHorse:UpOffset",									bowAim.horseUpOffset),
	IMPL_GETTER("BowaimHorse:ZoomOffset",								bowAim.horseZoomOffset),
	IMPL_GETTER("BowaimHorse:FOVOffset",								bowAim.horseFOVOffset),
	IMPL_GETTER("MinFollowRateBowAimHorseback",							bowAim.interpHorsebackConf.minCameraFollowRate),
	IMPL_GETTER("MaxFollowRateBowAimHorseback",							bowAim.interpHorsebackConf.maxCameraFollowRate),
	IMPL_GETTER("MaxSmoothingInterpDistanceBowAimHorseback",			bowAim.interpHorsebackConf.zoomMaxSmoothingDistance),
	IMPL_GETTER("MinSepLocalFollowRateBowAimHorseback",					bowAim.interpHorsebackConf.localMinFollowRate),
	IMPL_GETTER("MaxSepLocalFollowRateBowAimHorseback",					bowAim.interpHorsebackConf.localMaxFollowRate),
	IMPL_GETTER("MaxSepLocalSmoothingInterpDistanceBowAimHorseback",	bowAim.interpHorsebackConf.localMaxSmoothingDistance),

	// @Note:BowAim: Just repurpose another combat group for sneaking
	IMPL_GETTER("BowaimSneak:SideOffset",							bowAim.combatMeleeSideOffset),
	IMPL_GETTER("BowaimSneak:UpOffset",								bowAim.combatMeleeUpOffset),
	IMPL_GETTER("BowaimSneak:ZoomOffset",							bowAim.combatMeleeZoomOffset),
	IMPL_GETTER("BowaimSneak:FOVOffset",							bowAim.combatMeleeFOVOffset),
	IMPL_GETTER("MinFollowRateBowAimSneak",							bowAim.interpMeleeConf.minCameraFollowRate),
	IMPL_GETTER("MaxFollowRateBowAimSneak",							bowAim.interpMeleeConf.maxCameraFollowRate),
	IMPL_GETTER("MaxSmoothingInterpDistanceBowAimSneak",			bowAim.interpMeleeConf.zoomMaxSmoothingDistance),
	IMPL_GETTER("MinSepLocalFollowRateBowAimSneak",					bowAim.interpMeleeConf.localMinFollowRate),
	IMPL_GETTER("MaxSepLocalFollowRateBowAimSneak",					bowAim.interpMeleeConf.localMaxFollowRate),
	IMPL_GETTER("MaxSepLocalSmoothingInterpDistanceBowAimSneak",	bowAim.interpMeleeConf.localMaxSmoothingDistance),
});

const eastl::unordered_map<eastl::string_view, eastl::function<void(float)>> floatSetterFN = {
	// We can ignore getters for these as we just return 0.0f if not found, which is what we want in this case
	IMPL_GROUP_SETTER("Group:SideOffset",				sideOffset, float),
	IMPL_GROUP_SETTER("Group:UpOffset",					upOffset, float),
	IMPL_GROUP_SETTER("Group:ZoomOffset",				zoomOffset, float),
	IMPL_GROUP_SETTER("Group:FOVOffset",				fovOffset, float),

	IMPL_GROUP_SETTER("GroupCombat:Ranged:SideOffset",	combatRangedSideOffset, float),
	IMPL_GROUP_SETTER("GroupCombat:Ranged:UpOffset",	combatRangedUpOffset, float),
	IMPL_GROUP_SETTER("GroupCombat:Ranged:ZoomOffset",	combatRangedZoomOffset, float),
	IMPL_GROUP_SETTER("GroupCombat:Ranged:FOVOffset",	combatRangedFOVOffset, float),

	IMPL_GROUP_SETTER("GroupCombat:Magic:SideOffset",	combatMagicSideOffset, float),
	IMPL_GROUP_SETTER("GroupCombat:Magic:UpOffset",		combatMagicUpOffset, float),
	IMPL_GROUP_SETTER("GroupCombat:Magic:ZoomOffset",	combatMagicZoomOffset, float),
	IMPL_GROUP_SETTER("GroupCombat:Magic:FOVOffset",	combatMagicFOVOffset, float),

	IMPL_GROUP_SETTER("GroupCombat:Melee:SideOffset",	combatMeleeSideOffset, float),
	IMPL_GROUP_SETTER("GroupCombat:Melee:UpOffset",		combatMeleeUpOffset, float),
	IMPL_GROUP_SETTER("GroupCombat:Melee:ZoomOffset",	combatMeleeZoomOffset, float),
	IMPL_GROUP_SETTER("GroupCombat:Melee:FOVOffset",	combatMeleeFOVOffset, float),
};

// Int
constexpr auto intGetters = mapbox::eternal::hash_map<mapbox::eternal::string, int*>({
	IMPL_GETTER("ShoulderSwapKeyCode", shoulderSwapKey),
	IMPL_GETTER("NextPresetKeyCode", nextPresetKey),
	IMPL_GETTER("ModEnabledKeyCode", modToggleKey),
	IMPL_GETTER("ToggleCustomZKeyCode", applyZOffsetKey),
	IMPL_GETTER("ToggleUserDefinedOffsetKeyCode", toggleUserDefinedOffsetGroupKey),
});

// String
const eastl::unordered_map<eastl::string_view, eastl::function<RE::BSFixedString(void)>> stringGetters = {
	IMPL_SCALAR_METHOD_GETTER("InterpolationMethod", currentScalar),
	IMPL_SCALAR_METHOD_GETTER("SeparateZInterpMethod", separateZScalar),
	IMPL_SCALAR_METHOD_GETTER("SepLocalInterpMethod", separateLocalScalar),
	IMPL_SCALAR_METHOD_GETTER("OffsetTransitionMethod", offsetScalar),
	IMPL_SCALAR_METHOD_GETTER("ZoomTransitionMethod", zoomScalar),
	IMPL_SCALAR_METHOD_GETTER("FOVTransitionMethod", fovScalar),

	IMPL_SCALAR_METHOD_GETTER("GlobalInterpDisableMethod", globalInterpDisableMehtod),
	IMPL_SCALAR_METHOD_GETTER("GlobalInterpOverrideMethod", globalInterpOverrideMethod),
	IMPL_SCALAR_METHOD_GETTER("LocalInterpOverrideMethod", localInterpOverrideMethod),

	IMPL_SCALAR_METHOD_GETTER("PitchZoomMethod", pitchZoomMethod),

	IMPL_OFFSET_GROUP_SCALAR_GETTERS(standing, "Standing"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(walking, "Walking"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(running, "Running"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(sprinting, "Sprinting"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(sneaking, "Sneaking"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(swimming, "Swimming"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(sitting, "Sitting"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(horseback, "Horseback"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(dragon, "Dragon"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(vampireLord, "VampireLord"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(werewolf, "Werewolf"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(vanity, "Vanity"),
	IMPL_OFFSET_GROUP_SCALAR_GETTERS(userDefined, "Custom"),

	IMPL_SCALAR_METHOD_GETTER("SelectedScalarBowAim", bowAim.interpRangedConf.currentScalar),
	IMPL_SCALAR_METHOD_GETTER("SelectedLocalScalarBowAim", bowAim.interpRangedConf.separateLocalScalar),
	IMPL_SCALAR_METHOD_GETTER("SelectedScalarBowAimHorseback", bowAim.interpHorsebackConf.currentScalar),
	IMPL_SCALAR_METHOD_GETTER("SelectedLocalScalarBowAimHorseback", bowAim.interpHorsebackConf.separateLocalScalar),
	IMPL_SCALAR_METHOD_GETTER("SelectedScalarBowAimSneak", bowAim.interpMeleeConf.currentScalar),
	IMPL_SCALAR_METHOD_GETTER("SelectedLocalScalarBowAimSneak", bowAim.interpMeleeConf.separateLocalScalar),

	{ PAPYRUS_MANGLE("WorldCrosshairType"), []() {
		const auto it = Config::crosshairTypeRevLookup.find(Config::GetCurrentConfig()->worldCrosshairType);
		if (it != Config::crosshairTypeRevLookup.end()) {
			return RE::BSFixedString(it->second.c_str());
		}
		return RE::BSFixedString("");
	}},

	{ PAPYRUS_MANGLE("DialogueMode"), [] {
		const auto it = Config::dialogueTypeRevLookup.find(Config::GetCurrentConfig()->dialogueMode);
		if (it != Config::dialogueTypeRevLookup.end()) {
			return RE::BSFixedString(it->second.c_str());
		}
		return RE::BSFixedString("");
	}},
};

const eastl::unordered_map<eastl::string_view, eastl::function<void(RE::BSFixedString&)>> stringSetters = {
	IMPL_SCALAR_METHOD_SETTER("InterpolationMethod", currentScalar),
	IMPL_SCALAR_METHOD_SETTER("SeparateZInterpMethod", separateZScalar),
	IMPL_SCALAR_METHOD_SETTER("SepLocalInterpMethod", separateLocalScalar),
	IMPL_SCALAR_METHOD_SETTER("OffsetTransitionMethod", offsetScalar),
	IMPL_SCALAR_METHOD_SETTER("ZoomTransitionMethod", zoomScalar),
	IMPL_SCALAR_METHOD_SETTER("FOVTransitionMethod", fovScalar),

	IMPL_SCALAR_METHOD_SETTER("GlobalInterpDisableMethod", globalInterpDisableMehtod),
	IMPL_SCALAR_METHOD_SETTER("GlobalInterpOverrideMethod", globalInterpOverrideMethod),
	IMPL_SCALAR_METHOD_SETTER("LocalInterpOverrideMethod", localInterpOverrideMethod),

	IMPL_SCALAR_METHOD_SETTER("PitchZoomMethod", pitchZoomMethod),

	IMPL_OFFSET_GROUP_SCALAR_SETTERS(standing, "Standing"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(walking, "Walking"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(running, "Running"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(sprinting, "Sprinting"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(sneaking, "Sneaking"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(swimming, "Swimming"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(sitting, "Sitting"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(horseback, "Horseback"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(dragon, "Dragon"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(vampireLord, "VampireLord"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(werewolf, "Werewolf"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(vanity, "Vanity"),
	IMPL_OFFSET_GROUP_SCALAR_SETTERS(userDefined, "Custom"),

	IMPL_SCALAR_METHOD_SETTER("SelectedScalarBowAim", bowAim.interpRangedConf.currentScalar),
	IMPL_SCALAR_METHOD_SETTER("SelectedLocalScalarBowAim", bowAim.interpRangedConf.separateLocalScalar),
	IMPL_SCALAR_METHOD_SETTER("SelectedScalarBowAimHorseback", bowAim.interpHorsebackConf.currentScalar),
	IMPL_SCALAR_METHOD_SETTER("SelectedLocalScalarBowAimHorseback", bowAim.interpHorsebackConf.separateLocalScalar),
	IMPL_SCALAR_METHOD_SETTER("SelectedScalarBowAimSneak", bowAim.interpMeleeConf.currentScalar),
	IMPL_SCALAR_METHOD_SETTER("SelectedLocalScalarBowAimSneak", bowAim.interpMeleeConf.separateLocalScalar),

	{ PAPYRUS_MANGLE("WorldCrosshairType"), [](RE::BSFixedString& str) {
		const auto upper = Util::UpperCase(str);
		const auto it = Config::crosshairTypeLookup.find(upper.c_str());
		if (it != Config::crosshairTypeLookup.end()) {
			Config::GetCurrentConfig()->worldCrosshairType = it->second;
			Config::SaveCurrentConfig();
		}
	}},

	{ PAPYRUS_MANGLE("DialogueMode"), [](RE::BSFixedString& str) {
		const auto upper = Util::UpperCase(str);
		const auto it = Config::dialogueTypeLookup.find(upper.c_str());
		if (it != Config::dialogueTypeLookup.end()) {
			Config::GetCurrentConfig()->dialogueMode = it->second;
			Config::SaveCurrentConfig();
		}
	}},
};

static void SmoothCam_SetBoolConfig(RE::StaticFunctionTag*, RE::BSFixedString var, bool value) {
	const auto mdmp = Debug::MiniDumpScope();
	const auto it = boolGetters.find(var.c_str());
	if (it != boolGetters.end()) {
		*it->second = value;
		Config::SaveCurrentConfig();
	}
}

static bool SmoothCam_GetBoolConfig(RE::StaticFunctionTag*, RE::BSFixedString var) {
	const auto mdmp = Debug::MiniDumpScope();
	const auto it = boolGetters.find(var.c_str());
	if (it != boolGetters.end()) {
		return *it->second;
	} else {
		const auto itf = boolGetterFNs.find(var.c_str());
		if (itf != boolGetterFNs.end())
			return itf->second();
	}
	return false;
}

static void SmoothCam_SetFloatConfig(RE::StaticFunctionTag*, RE::BSFixedString var, float value) {
	const auto mdmp = Debug::MiniDumpScope();
	const auto it = floatGetters.find(var.c_str());
	if (it != floatGetters.end()) {
		*it->second = value;
		Config::SaveCurrentConfig();
	} else {
		const auto itf = floatSetterFN.find(var.c_str());
		if (itf != floatSetterFN.end()) {
			itf->second(value);
			Config::SaveCurrentConfig();
		}	
	}
}

static float SmoothCam_GetFloatConfig(RE::StaticFunctionTag*, RE::BSFixedString var) {
	const auto mdmp = Debug::MiniDumpScope();
	const auto it = floatGetters.find(var.c_str());
	if (it != floatGetters.end())
		return *it->second;
	else
		return 0.0f;
}

static void SmoothCam_SetStringConfig(RE::StaticFunctionTag*, RE::BSFixedString var, RE::BSFixedString value) {
	const auto mdmp = Debug::MiniDumpScope();
	const auto it = stringSetters.find(var.c_str());
	if (it != stringSetters.end()) {
		it->second(value);
		Config::SaveCurrentConfig();
	}
}

static RE::BSFixedString SmoothCam_GetStringConfig(RE::StaticFunctionTag*, RE::BSFixedString var) {
	const auto mdmp = Debug::MiniDumpScope();
	const auto it = stringGetters.find(var.c_str());
	if (it != stringGetters.end())
		return it->second();
	else
		return RE::BSFixedString("");
}

static void SmoothCam_SetIntConfig(RE::StaticFunctionTag*, RE::BSFixedString var, int32_t value) {
	const auto mdmp = Debug::MiniDumpScope();
	const auto it = intGetters.find(var.c_str());
	if (it != intGetters.end()) {
		*it->second = value;
		Config::SaveCurrentConfig();
	}
}

static int32_t SmoothCam_GetIntConfig(RE::StaticFunctionTag*, RE::BSFixedString var) {
	const auto mdmp = Debug::MiniDumpScope();
	const auto it = intGetters.find(var.c_str());
	if (it != intGetters.end())
		return static_cast<int32_t>(*it->second);
	else
		return static_cast<int32_t>(-1);
}

static RE::BSFixedString SmoothCam_SaveAsPreset(RE::StaticFunctionTag*, int32_t index, RE::BSFixedString name) {
	const auto mdmp = Debug::MiniDumpScope();
	return Config::SaveConfigAsPreset(index, name);
}

static bool SmoothCam_LoadPreset(RE::StaticFunctionTag*, int32_t index) {
	const auto mdmp = Debug::MiniDumpScope();
	return Config::LoadPreset(index);
}

static RE::BSFixedString SmoothCam_GetPresetNameAtIndex(RE::StaticFunctionTag*, int32_t index) {
	const auto mdmp = Debug::MiniDumpScope();
	return Config::GetPresetSlotName(index);
}

static void SmoothCam_ResetConfig(RE::StaticFunctionTag*) {
	const auto mdmp = Debug::MiniDumpScope();
	Config::ResetConfig();
}

static void SmoothCam_ResetCrosshair(RE::StaticFunctionTag*) {
	const auto mdmp = Debug::MiniDumpScope();
	g_theCamera->GetThirdpersonCamera()->GetCrosshairManager()->Reset(true);
}

static void SmoothCam_FixCameraState(RE::StaticFunctionTag*) {
	const auto mdmp = Debug::MiniDumpScope();
	g_theCamera->SetShouldForceCameraState(true, RE::CameraState::kThirdPerson);
}

static int32_t SmoothCam_NumAPIConsumers(RE::StaticFunctionTag*) {
	const auto mdmp = Debug::MiniDumpScope();
	return static_cast<int32_t>(Messaging::SmoothCamInterface::GetInstance()->GetConsumers().size());
}

static RE::BSFixedString SmoothCam_GetAPIConsumerName(RE::StaticFunctionTag*, int32_t index) {
	const auto mdmp = Debug::MiniDumpScope();
	const auto& arr = Messaging::SmoothCamInterface::GetInstance()->GetConsumers();
	if (index >= arr.size()) return RE::BSFixedString("");
	return RE::BSFixedString(arr.at(index).c_str());
}

static RE::BSFixedString SmoothCam_IsModDetected(RE::StaticFunctionTag*, int32_t modID) {
	const auto mdmp = Debug::MiniDumpScope();
	switch (static_cast<Compat::Mod>(modID)) {
		case Compat::Mod::ArcheryGameplayOverhaul:
			return Compat::IsPresent(Compat::Mod::ArcheryGameplayOverhaul) ?
				RE::BSFixedString("Detected") : RE::BSFixedString("Not Detected");

		case Compat::Mod::ImmersiveFirstPersonView:
			return Compat::IsPresent(Compat::Mod::ImmersiveFirstPersonView) ?
				RE::BSFixedString("Detected") : RE::BSFixedString("Not Detected");

		case Compat::Mod::ImprovedCamera: {
			if (Compat::GetICDetectReason() == Compat::ICCheckResult::OK)
				return RE::BSFixedString("Detected");
			else if (Compat::GetICDetectReason() == Compat::ICCheckResult::NOT_FOUND)
				return RE::BSFixedString("Not Detected");
			else
				return RE::BSFixedString("Version Mismatch");
		}
		default:
			return RE::BSFixedString("Not Detected");
	}
}

bool PapyrusBindings::Bind([[maybe_unused]] RE::BSScript::IVirtualMachine* vm) {
	vm->RegisterFunction("SmoothCam_SetBoolConfig", ScriptClassName, SmoothCam_SetBoolConfig);
	vm->RegisterFunction("SmoothCam_GetBoolConfig", ScriptClassName, SmoothCam_GetBoolConfig);
	vm->RegisterFunction("SmoothCam_SetFloatConfig", ScriptClassName, SmoothCam_SetFloatConfig);
	vm->RegisterFunction("SmoothCam_GetFloatConfig", ScriptClassName, SmoothCam_GetFloatConfig);
	vm->RegisterFunction("SmoothCam_SetStringConfig", ScriptClassName, SmoothCam_SetStringConfig);
	vm->RegisterFunction("SmoothCam_GetStringConfig", ScriptClassName, SmoothCam_GetStringConfig);
	vm->RegisterFunction("SmoothCam_SetIntConfig", ScriptClassName, SmoothCam_SetIntConfig);
	vm->RegisterFunction("SmoothCam_GetIntConfig", ScriptClassName, SmoothCam_GetIntConfig);
	vm->RegisterFunction("SmoothCam_SaveAsPreset", ScriptClassName, SmoothCam_SaveAsPreset);
	vm->RegisterFunction("SmoothCam_LoadPreset", ScriptClassName, SmoothCam_LoadPreset);
	vm->RegisterFunction("SmoothCam_GetPresetNameAtIndex", ScriptClassName, SmoothCam_GetPresetNameAtIndex);
	vm->RegisterFunction("SmoothCam_ResetConfig", ScriptClassName, SmoothCam_ResetConfig);
	vm->RegisterFunction("SmoothCam_ResetCrosshair", ScriptClassName, SmoothCam_ResetCrosshair);
	vm->RegisterFunction("SmoothCam_FixCameraState", ScriptClassName, SmoothCam_FixCameraState);
	vm->RegisterFunction("SmoothCam_NumAPIConsumers", ScriptClassName, SmoothCam_NumAPIConsumers);
	vm->RegisterFunction("SmoothCam_GetAPIConsumerName", ScriptClassName, SmoothCam_GetAPIConsumerName);
	vm->RegisterFunction("SmoothCam_IsModDetected", ScriptClassName, SmoothCam_IsModDetected);
	return true;
}
#pragma warning(pop)