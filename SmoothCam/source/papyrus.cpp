#include "papyrus.h"

using namespace PapyrusBindings;
#define IMPL_GETTER(VarName, Var)                   \
    { VarName, []() noexcept {                      \
        return Config::GetCurrentConfig()->Var;     \
    } },

#define IMPL_SETTER(VarName, Var, Type)         \
    { VarName, [](Type arg) {                   \
        Config::GetCurrentConfig()->Var = arg;  \
        Config::SaveCurrentConfig();            \
    } },

#define IMPL_SCALAR_METHOD_GETTER(VarName, Var)             \
    { VarName, []() {                                       \
        const auto it = Config::scalarMethodRevLookup.find( \
            Config::GetCurrentConfig()->Var                 \
        );                                                  \
        if (it != Config::scalarMethodRevLookup.end())      \
            return BSFixedString(it->second.c_str());       \
        else                                                \
            return BSFixedString("linear");                 \
    } },

#define IMPL_SCALAR_METHOD_SETTER(VarName, Var)                     \
    { VarName, [](BSFixedString& str) {                             \
        const auto it = Config::scalarMethods.find(str.c_str());    \
        if (it != Config::scalarMethods.end()) {                    \
            Config::GetCurrentConfig()->Var = it->second;           \
            Config::SaveCurrentConfig();                            \
        }                                                           \
    } },

#define IMPL_GROUP_SETTER(VarName, Var, Type)   \
    { VarName, [](Type arg) {                   \
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
        Config::SaveCurrentConfig();            \
    } },

#define IMPL_OFFSET_GROUP_FLOAT_GETTERS(GroupVarName, GroupNamePrefix)                            \
	IMPL_GETTER(GroupNamePrefix##":SideOffset", GroupVarName.sideOffset)                          \
	IMPL_GETTER(GroupNamePrefix##":UpOffset", GroupVarName.upOffset)                              \
	IMPL_GETTER(GroupNamePrefix##":ZoomOffset", GroupVarName.zoomOffset)                          \
	IMPL_GETTER(GroupNamePrefix##":FOVOffset", GroupVarName.fovOffset)                            \
	IMPL_GETTER(GroupNamePrefix##"Combat:Ranged:SideOffset", GroupVarName.combatRangedSideOffset) \
	IMPL_GETTER(GroupNamePrefix##"Combat:Ranged:UpOffset", GroupVarName.combatRangedUpOffset)     \
	IMPL_GETTER(GroupNamePrefix##"Combat:Ranged:ZoomOffset", GroupVarName.combatRangedZoomOffset) \
	IMPL_GETTER(GroupNamePrefix##"Combat:Ranged:FOVOffset", GroupVarName.combatRangedFOVOffset)   \
	IMPL_GETTER(GroupNamePrefix##"Combat:Magic:SideOffset", GroupVarName.combatMagicSideOffset)   \
	IMPL_GETTER(GroupNamePrefix##"Combat:Magic:UpOffset", GroupVarName.combatMagicUpOffset)       \
	IMPL_GETTER(GroupNamePrefix##"Combat:Magic:ZoomOffset", GroupVarName.combatMagicZoomOffset)   \
	IMPL_GETTER(GroupNamePrefix##"Combat:Magic:FOVOffset", GroupVarName.combatMagicFOVOffset)     \
	IMPL_GETTER(GroupNamePrefix##"Combat:Melee:SideOffset", GroupVarName.combatMeleeSideOffset)   \
	IMPL_GETTER(GroupNamePrefix##"Combat:Melee:UpOffset", GroupVarName.combatMeleeUpOffset)       \
	IMPL_GETTER(GroupNamePrefix##"Combat:Melee:ZoomOffset", GroupVarName.combatMeleeZoomOffset)   \
	IMPL_GETTER(GroupNamePrefix##"Combat:Melee:FOVOffset", GroupVarName.combatMeleeFOVOffset)

#define IMPL_OFFSET_GROUP_FLOAT_SETTERS(GroupVarName, GroupNamePrefix)                                   \
	IMPL_SETTER(GroupNamePrefix##":SideOffset", GroupVarName.sideOffset, float)                          \
	IMPL_SETTER(GroupNamePrefix##":UpOffset", GroupVarName.upOffset, float)                              \
	IMPL_SETTER(GroupNamePrefix##":ZoomOffset", GroupVarName.zoomOffset, float)                          \
	IMPL_SETTER(GroupNamePrefix##":FOVOffset", GroupVarName.fovOffset, float)                            \
	IMPL_SETTER(GroupNamePrefix##"Combat:Ranged:SideOffset", GroupVarName.combatRangedSideOffset, float) \
	IMPL_SETTER(GroupNamePrefix##"Combat:Ranged:UpOffset", GroupVarName.combatRangedUpOffset, float)     \
	IMPL_SETTER(GroupNamePrefix##"Combat:Ranged:ZoomOffset", GroupVarName.combatRangedZoomOffset, float) \
	IMPL_SETTER(GroupNamePrefix##"Combat:Ranged:FOVOffset", GroupVarName.combatRangedFOVOffset, float)   \
	IMPL_SETTER(GroupNamePrefix##"Combat:Magic:SideOffset", GroupVarName.combatMagicSideOffset, float)   \
	IMPL_SETTER(GroupNamePrefix##"Combat:Magic:UpOffset", GroupVarName.combatMagicUpOffset, float)       \
	IMPL_SETTER(GroupNamePrefix##"Combat:Magic:ZoomOffset", GroupVarName.combatMagicZoomOffset, float)   \
	IMPL_SETTER(GroupNamePrefix##"Combat:Magic:FOVOffset", GroupVarName.combatMagicFOVOffset, float)     \
	IMPL_SETTER(GroupNamePrefix##"Combat:Melee:SideOffset", GroupVarName.combatMeleeSideOffset, float)   \
	IMPL_SETTER(GroupNamePrefix##"Combat:Melee:UpOffset", GroupVarName.combatMeleeUpOffset, float)       \
	IMPL_SETTER(GroupNamePrefix##"Combat:Melee:ZoomOffset", GroupVarName.combatMeleeZoomOffset, float)   \
	IMPL_SETTER(GroupNamePrefix##"Combat:Melee:FOVOffset", GroupVarName.combatMeleeFOVOffset, float)

#define IMPL_OFFSET_GROUP_BOOL_GETTERS(GroupVarName, GroupNamePrefix)                         \
	IMPL_GETTER("Interp"##GroupNamePrefix,					GroupVarName.interp)              \
	IMPL_GETTER("Interp"##GroupNamePrefix##"RangedCombat",	GroupVarName.interpRangedCombat)  \
	IMPL_GETTER("Interp"##GroupNamePrefix##"MagicCombat",	GroupVarName.interpMagicCombat)   \
	IMPL_GETTER("Interp"##GroupNamePrefix##"MeleeCombat",	GroupVarName.interpMeleeCombat)   

#define IMPL_OFFSET_GROUP_BOOL_SETTERS(GroupVarName, GroupNamePrefix)                               \
	IMPL_SETTER("Interp"##GroupNamePrefix,					GroupVarName.interp, bool)              \
	IMPL_SETTER("Interp"##GroupNamePrefix##"RangedCombat",	GroupVarName.interpRangedCombat, bool)  \
	IMPL_SETTER("Interp"##GroupNamePrefix##"MagicCombat",	GroupVarName.interpMagicCombat, bool)   \
	IMPL_SETTER("Interp"##GroupNamePrefix##"MeleeCombat",	GroupVarName.interpMeleeCombat, bool)   

const std::unordered_map<std::string_view, std::function<BSFixedString(void)>> stringGetters = {
	IMPL_SCALAR_METHOD_GETTER("InterpolationMethod", currentScalar)
	IMPL_SCALAR_METHOD_GETTER("SeparateZInterpMethod", separateZScalar)
	IMPL_SCALAR_METHOD_GETTER("SepLocalInterpMethod", separateLocalScalar)
	IMPL_SCALAR_METHOD_GETTER("OffsetTransitionMethod", offsetScalar)
	IMPL_SCALAR_METHOD_GETTER("ZoomTransitionMethod", zoomScalar)
	IMPL_SCALAR_METHOD_GETTER("FOVTransitionMethod", fovScalar)

	{ "WorldCrosshairType", []() {
		const auto it = Config::crosshairTypeRevLookup.find(Config::GetCurrentConfig()->worldCrosshairType);
		if (it != Config::crosshairTypeRevLookup.end()) {
			return BSFixedString(it->second.c_str());
		}
		return BSFixedString("");
	}},
};

const std::unordered_map<std::string_view, std::function<bool(void)>> boolGetters = {
	{
		"D3DHooked", []() noexcept {
		return Render::HasContext();
		}
	},
	
	// Comapt
	IMPL_GETTER("ACCCompat",						compatACC)
	IMPL_GETTER("ICCompat",							compatIC)
	IMPL_GETTER("IFPVCompat",						compatIFPV)
	IMPL_GETTER("AGOCompat",						compatAGO)

	// Crosshair
	IMPL_GETTER("Enable3DBowCrosshair",				use3DBowAimCrosshair)
	IMPL_GETTER("Enable3DMagicCrosshair",			use3DMagicCrosshair)
	IMPL_GETTER("UseWorldCrosshair",				useWorldCrosshair)
	IMPL_GETTER("WorldCrosshairDepthTest",			worldCrosshairDepthTest)
	IMPL_GETTER("EnableArrowPrediction",			useArrowPrediction)
	IMPL_GETTER("DrawArrowArc",						drawArrowArc)
	IMPL_GETTER("EnableCrosshairSizeManip",			enableCrosshairSizeManip)
	IMPL_GETTER("HideCrosshairOutOfCombat",			hideNonCombatCrosshair)
	IMPL_GETTER("HideCrosshairMeleeCombat",			hideCrosshairMeleeCombat)

	// Primary interpolation
	IMPL_GETTER("InterpolationEnabled",				enableInterp)
	IMPL_GETTER("DisableDeltaTime",					disableDeltaTime)

	// Separate local interpolation
	IMPL_GETTER("SeparateLocalInterpolation",		separateLocalInterp)

	// Separate Z interpolation
	IMPL_GETTER("SeparateZInterpEnabled",			separateZInterp)

	// Offset interpolation
	IMPL_GETTER("OffsetTransitionEnabled",			enableOffsetInterpolation)

	// Zoom interpolation
	IMPL_GETTER("ZoomTransitionEnabled",			enableZoomInterpolation)

	// FOV interpolation
	IMPL_GETTER("FOVTransitionEnabled",				enableFOVInterpolation)

	// Distance clamping
	IMPL_GETTER("CameraDistanceClampXEnable",		cameraDistanceClampXEnable)
	IMPL_GETTER("CameraDistanceClampYEnable",		cameraDistanceClampYEnable)
	IMPL_GETTER("CameraDistanceClampZEnable",		cameraDistanceClampZEnable)
	IMPL_GETTER("ShoulderSwapXClamping",			swapXClamping)

	// Offset groups
	IMPL_OFFSET_GROUP_BOOL_GETTERS(standing, "Standing")
	IMPL_OFFSET_GROUP_BOOL_GETTERS(walking, "Walking")
	IMPL_OFFSET_GROUP_BOOL_GETTERS(running, "Running")
	IMPL_OFFSET_GROUP_BOOL_GETTERS(sprinting, "Sprinting")
	IMPL_OFFSET_GROUP_BOOL_GETTERS(sneaking, "Sneaking")
	IMPL_OFFSET_GROUP_BOOL_GETTERS(swimming, "Swimming")
	IMPL_OFFSET_GROUP_BOOL_GETTERS(sitting, "Sitting")
	IMPL_OFFSET_GROUP_BOOL_GETTERS(horseback, "Horseback")
	IMPL_OFFSET_GROUP_BOOL_GETTERS(dragon, "Dragon")
	IMPL_OFFSET_GROUP_BOOL_GETTERS(vampireLord, "VampireLord")
	IMPL_OFFSET_GROUP_BOOL_GETTERS(werewolf, "Werewolf")

	IMPL_GETTER("InterpBowAim",						bowAim.interpRangedCombat)
	IMPL_GETTER("InterpBowAimHorseback",			bowAim.interpHorseback)
	// @Note:BowAim: Just repurpose another combat group for sneaking
	IMPL_GETTER("InterpBowAimSneaking",				bowAim.interpMeleeCombat)
};

const std::unordered_map<std::string_view, std::function<float(void)>> floatGetters = {
	// Primary interpolation
	IMPL_GETTER("MinFollowDistance",					minCameraFollowDistance)
	IMPL_GETTER("MinCameraFollowRate",					minCameraFollowRate)
	IMPL_GETTER("MaxCameraFollowRate",					maxCameraFollowRate)
	IMPL_GETTER("MaxSmoothingInterpDistance",			zoomMaxSmoothingDistance)
	IMPL_GETTER("ZoomMul",								zoomMul)

	// Crosshair
	IMPL_GETTER("CrosshairNPCGrowSize",					crosshairNPCHitGrowSize)
	IMPL_GETTER("CrosshairMinDistSize",					crosshairMinDistSize)
	IMPL_GETTER("CrosshairMaxDistSize",					crosshairMaxDistSize)
	IMPL_GETTER("ArrowArcColorR",						arrowArcColor.r)
	IMPL_GETTER("ArrowArcColorG",						arrowArcColor.g)
	IMPL_GETTER("ArrowArcColorB",						arrowArcColor.b)
	IMPL_GETTER("ArrowArcColorA",						arrowArcColor.a)
	IMPL_GETTER("MaxArrowPredictionRange",				maxArrowPredictionRange)

	// Separate local interpolation
	IMPL_GETTER("SepLocalInterpRate",					localScalarRate)
	
	// Separate Z interpolation
	IMPL_GETTER("SepZMaxInterpDistance",				separateZMaxSmoothingDistance)
	IMPL_GETTER("SepZMinFollowRate",					separateZMinFollowRate)
	IMPL_GETTER("SepZMaxFollowRate",					separateZMaxFollowRate)
	
	// Offset interpolation
	IMPL_GETTER("OffsetTransitionDuration",				offsetInterpDurationSecs)

	// Zoom interpolation
	IMPL_GETTER("ZoomTransitionDuration",				zoomInterpDurationSecs)

	// FOV interpolation
	IMPL_GETTER("FOVTransitionDuration",				fovInterpDurationSecs)

	// Distance clamping
	IMPL_GETTER("CameraDistanceClampXMin",				cameraDistanceClampXMin)
	IMPL_GETTER("CameraDistanceClampXMax",				cameraDistanceClampXMax)
	IMPL_GETTER("CameraDistanceClampYMin",				cameraDistanceClampYMin)
	IMPL_GETTER("CameraDistanceClampYMax",				cameraDistanceClampYMax)
	IMPL_GETTER("CameraDistanceClampZMin",				cameraDistanceClampZMin)
	IMPL_GETTER("CameraDistanceClampZMax",				cameraDistanceClampZMax)

	// Offset groups
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(standing, "Standing")
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(walking, "Walking")
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(running, "Running")
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(sprinting, "Sprinting")
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(sneaking, "Sneaking")
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(horseback, "Horseback")
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(swimming, "Swimming")
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(sitting, "Sitting")
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(dragon, "Dragon")
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(vampireLord, "VampireLord")
	IMPL_OFFSET_GROUP_FLOAT_GETTERS(werewolf, "Werewolf")

	IMPL_GETTER("Bowaim:SideOffset",					bowAim.sideOffset)
	IMPL_GETTER("Bowaim:UpOffset",						bowAim.upOffset)
	IMPL_GETTER("Bowaim:ZoomOffset",					bowAim.zoomOffset)
	IMPL_GETTER("Bowaim:FOVOffset",						bowAim.fovOffset)
	IMPL_GETTER("BowaimHorse:SideOffset",				bowAim.horseSideOffset)
	IMPL_GETTER("BowaimHorse:UpOffset",					bowAim.horseUpOffset)
	IMPL_GETTER("BowaimHorse:ZoomOffset",				bowAim.horseZoomOffset)
	IMPL_GETTER("BowaimHorse:FOVOffset",				bowAim.horseFOVOffset)
	// @Note:BowAim: Just repurpose another combat group for sneaking
	IMPL_GETTER("BowaimSneak:SideOffset",				bowAim.combatMeleeSideOffset)
	IMPL_GETTER("BowaimSneak:UpOffset",					bowAim.combatMeleeUpOffset)
	IMPL_GETTER("BowaimSneak:ZoomOffset",				bowAim.combatMeleeZoomOffset)
	IMPL_GETTER("BowaimSneak:FOVOffset",				bowAim.combatMeleeFOVOffset)
};

const std::unordered_map<std::string_view, std::function<int(void)>> intGetters = {
	IMPL_GETTER("ShoulderSwapKeyCode", shoulderSwapKey)
};

const std::unordered_map<std::string_view, std::function<void(BSFixedString)>> stringSetters = {
	IMPL_SCALAR_METHOD_SETTER("InterpolationMethod", currentScalar)
	IMPL_SCALAR_METHOD_SETTER("SeparateZInterpMethod", separateZScalar)
	IMPL_SCALAR_METHOD_SETTER("SepLocalInterpMethod", separateLocalScalar)
	IMPL_SCALAR_METHOD_SETTER("OffsetTransitionMethod", offsetScalar)
	IMPL_SCALAR_METHOD_SETTER("ZoomTransitionMethod", zoomScalar)
	IMPL_SCALAR_METHOD_SETTER("FOVTransitionMethod", fovScalar)
	
	{ "WorldCrosshairType", [](BSFixedString& str) {
		const auto it = Config::crosshairTypeLookup.find(str.c_str());
		if (it != Config::crosshairTypeLookup.end()) {
			Config::GetCurrentConfig()->worldCrosshairType = it->second;
			Config::SaveCurrentConfig();
		}
	}},
};

const std::unordered_map<std::string_view, std::function<void(bool)>> boolSetters = {
	// Compat
	IMPL_SETTER("ACCCompat",						compatACC, bool)
	IMPL_SETTER("ICCompat",							compatIC, bool)
	IMPL_SETTER("IFPVCompat",						compatIFPV, bool)
	IMPL_SETTER("AGOCompat",						compatAGO, bool)

	// Crosshair
	IMPL_SETTER("Enable3DBowCrosshair",				use3DBowAimCrosshair, bool)
	IMPL_SETTER("Enable3DMagicCrosshair",			use3DMagicCrosshair, bool)
	IMPL_SETTER("UseWorldCrosshair",				useWorldCrosshair, bool)
	IMPL_SETTER("WorldCrosshairDepthTest",			worldCrosshairDepthTest, bool)
	IMPL_SETTER("EnableArrowPrediction",			useArrowPrediction, bool)
	IMPL_SETTER("DrawArrowArc",						drawArrowArc, bool)
	IMPL_SETTER("EnableCrosshairSizeManip",			enableCrosshairSizeManip, bool)
	IMPL_SETTER("HideCrosshairOutOfCombat",			hideNonCombatCrosshair, bool)
	IMPL_SETTER("HideCrosshairMeleeCombat",			hideCrosshairMeleeCombat, bool)

	// Primary interpolation
	IMPL_SETTER("InterpolationEnabled",				enableInterp, bool)
	IMPL_SETTER("DisableDeltaTime",					disableDeltaTime, bool)
	
	// Separate local interpolation
	IMPL_SETTER("SeparateLocalInterpolation",		separateLocalInterp, bool)

	// Separate Z interpolation
	IMPL_SETTER("SeparateZInterpEnabled",			separateZInterp, bool)

	// Offset interpolation
	IMPL_SETTER("OffsetTransitionEnabled",			enableOffsetInterpolation, bool)

	// Zoom interpolation
	IMPL_SETTER("ZoomTransitionEnabled",			enableZoomInterpolation, bool)

	// FOV interpolation
	IMPL_SETTER("FOVTransitionEnabled",				enableFOVInterpolation, bool)

	// Distance clamping
	IMPL_SETTER("CameraDistanceClampXEnable",		cameraDistanceClampXEnable, bool)
	IMPL_SETTER("CameraDistanceClampYEnable",		cameraDistanceClampYEnable, bool)
	IMPL_SETTER("CameraDistanceClampZEnable",		cameraDistanceClampZEnable, bool)
	IMPL_SETTER("ShoulderSwapXClamping",			swapXClamping, bool)

	// Offset groups
	IMPL_OFFSET_GROUP_BOOL_SETTERS(standing, "Standing")
	IMPL_OFFSET_GROUP_BOOL_SETTERS(walking, "Walking")
	IMPL_OFFSET_GROUP_BOOL_SETTERS(running, "Running")
	IMPL_OFFSET_GROUP_BOOL_SETTERS(sprinting, "Sprinting")
	IMPL_OFFSET_GROUP_BOOL_SETTERS(sneaking, "Sneaking")
	IMPL_OFFSET_GROUP_BOOL_SETTERS(swimming, "Swimming")
	IMPL_OFFSET_GROUP_BOOL_SETTERS(sitting, "Sitting")
	IMPL_OFFSET_GROUP_BOOL_SETTERS(horseback, "Horseback")
	IMPL_OFFSET_GROUP_BOOL_SETTERS(dragon, "Dragon")
	IMPL_OFFSET_GROUP_BOOL_SETTERS(vampireLord, "VampireLord")
	IMPL_OFFSET_GROUP_BOOL_SETTERS(werewolf, "Werewolf")

	IMPL_SETTER("InterpBowAim",						bowAim.interpRangedCombat, bool)
	IMPL_SETTER("InterpBowAimHorseback",			bowAim.interpHorseback, bool)
	// @Note:BowAim: Just repurpose another combat group for sneaking
	IMPL_SETTER("InterpBowAimSneaking",				bowAim.interpMeleeCombat, bool)
};

const std::unordered_map<std::string_view, std::function<void(float)>> floatSetters = {
	// Primary interpolation
	IMPL_SETTER("MinFollowDistance",					minCameraFollowDistance, float)
	IMPL_SETTER("MinCameraFollowRate",					minCameraFollowRate, float)
	IMPL_SETTER("MaxCameraFollowRate",					maxCameraFollowRate, float)
	IMPL_SETTER("MaxSmoothingInterpDistance",			zoomMaxSmoothingDistance, float)
	IMPL_SETTER("ZoomMul",								zoomMul, float)

	// Crosshair
	IMPL_SETTER("CrosshairNPCGrowSize",					crosshairNPCHitGrowSize, float)
	IMPL_SETTER("CrosshairMinDistSize",					crosshairMinDistSize, float)
	IMPL_SETTER("CrosshairMaxDistSize",					crosshairMaxDistSize, float)
	IMPL_SETTER("ArrowArcColorR",						arrowArcColor.r, float)
	IMPL_SETTER("ArrowArcColorG",						arrowArcColor.g, float)
	IMPL_SETTER("ArrowArcColorB",						arrowArcColor.b, float)
	IMPL_SETTER("ArrowArcColorA",						arrowArcColor.a, float)
	IMPL_SETTER("MaxArrowPredictionRange",				maxArrowPredictionRange, float)

	// Separate local interpolation
	IMPL_SETTER("SepLocalInterpRate",					localScalarRate, float)

	// Separate Z interpolation
	IMPL_SETTER("SepZMaxInterpDistance",				separateZMaxSmoothingDistance, float)
	IMPL_SETTER("SepZMinFollowRate",					separateZMinFollowRate, float)
	IMPL_SETTER("SepZMaxFollowRate",					separateZMaxFollowRate, float)
	
	// Offset interpolation
	IMPL_SETTER("OffsetTransitionDuration",				offsetInterpDurationSecs, float)

	// Zoom interpolation
	IMPL_SETTER("ZoomTransitionDuration",				zoomInterpDurationSecs, float)

	// FOV interpolation
	IMPL_SETTER("FOVTransitionDuration",				fovInterpDurationSecs, float)

	// Distance clamping
	IMPL_SETTER("CameraDistanceClampXMin",				cameraDistanceClampXMin, float)
	IMPL_SETTER("CameraDistanceClampXMax",				cameraDistanceClampXMax, float)
	IMPL_SETTER("CameraDistanceClampYMin",				cameraDistanceClampYMin, float)
	IMPL_SETTER("CameraDistanceClampYMax",				cameraDistanceClampYMax, float)
	IMPL_SETTER("CameraDistanceClampZMin",				cameraDistanceClampZMin, float)
	IMPL_SETTER("CameraDistanceClampZMax",				cameraDistanceClampZMax, float)

	// Offset groups
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(standing, "Standing")
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(walking, "Walking")
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(running, "Running")
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(sprinting, "Sprinting")
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(sneaking, "Sneaking")
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(horseback, "Horseback")
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(swimming, "Swimming")
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(sitting, "Sitting")
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(dragon, "Dragon")
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(vampireLord, "VampireLord")
	IMPL_OFFSET_GROUP_FLOAT_SETTERS(werewolf, "Werewolf")

	IMPL_SETTER("Bowaim:SideOffset",					bowAim.sideOffset, float)
	IMPL_SETTER("Bowaim:UpOffset",						bowAim.upOffset, float)
	IMPL_SETTER("Bowaim:ZoomOffset",					bowAim.zoomOffset, float)
	IMPL_SETTER("Bowaim:FOVOffset",						bowAim.fovOffset, float)
	IMPL_SETTER("BowaimHorse:SideOffset",				bowAim.horseSideOffset, float)
	IMPL_SETTER("BowaimHorse:UpOffset",					bowAim.horseUpOffset, float)
	IMPL_SETTER("BowaimHorse:ZoomOffset",				bowAim.horseZoomOffset, float)
	IMPL_SETTER("BowaimHorse:FOVOffset",				bowAim.horseFOVOffset, float)
	// @Note:BowAim: Just repurpose another combat group for sneaking
	IMPL_SETTER("BowaimSneak:SideOffset",				bowAim.combatMeleeSideOffset, float)
	IMPL_SETTER("BowaimSneak:UpOffset",					bowAim.combatMeleeUpOffset, float)
	IMPL_SETTER("BowaimSneak:ZoomOffset",				bowAim.combatMeleeZoomOffset, float)
	IMPL_SETTER("BowaimSneak:FOVOffset",				bowAim.combatMeleeFOVOffset, float)

	// We can ignore getters for these as we just return 0.0f if not found, which is what we want in this case
	IMPL_GROUP_SETTER("Group:SideOffset",				sideOffset, float)
	IMPL_GROUP_SETTER("Group:UpOffset",					upOffset, float)
	IMPL_GROUP_SETTER("Group:ZoomOffset",				zoomOffset, float)
	IMPL_GROUP_SETTER("Group:FOVOffset",				fovOffset, float)

	IMPL_GROUP_SETTER("GroupCombat:Ranged:SideOffset",	combatRangedSideOffset, float)
	IMPL_GROUP_SETTER("GroupCombat:Ranged:UpOffset",	combatRangedUpOffset, float)
	IMPL_GROUP_SETTER("GroupCombat:Ranged:ZoomOffset",	combatRangedZoomOffset, float)
	IMPL_GROUP_SETTER("GroupCombat:Ranged:FOVOffset",	combatRangedFOVOffset, float)

	IMPL_GROUP_SETTER("GroupCombat:Magic:SideOffset",	combatMagicSideOffset, float)
	IMPL_GROUP_SETTER("GroupCombat:Magic:UpOffset",		combatMagicUpOffset, float)
	IMPL_GROUP_SETTER("GroupCombat:Magic:ZoomOffset",	combatMagicZoomOffset, float)
	IMPL_GROUP_SETTER("GroupCombat:Magic:FOVOffset",	combatMagicFOVOffset, float)

	IMPL_GROUP_SETTER("GroupCombat:Melee:SideOffset",	combatMeleeSideOffset, float)
	IMPL_GROUP_SETTER("GroupCombat:Melee:UpOffset",		combatMeleeUpOffset, float)
	IMPL_GROUP_SETTER("GroupCombat:Melee:ZoomOffset",	combatMeleeZoomOffset, float)
	IMPL_GROUP_SETTER("GroupCombat:Melee:FOVOffset",	combatMeleeFOVOffset, float)
};

const std::unordered_map<std::string_view, std::function<void(int)>> intSetters = {
	IMPL_SETTER("ShoulderSwapKeyCode", shoulderSwapKey, int)
};

void PapyrusBindings::Bind(VMClassRegistry* registry) {
	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, void, BSFixedString, BSFixedString>(
			"SmoothCam_SetStringConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, BSFixedString var, BSFixedString value) {
				const auto it = stringSetters.find(var.c_str());
				if (it != stringSetters.end())
					it->second(value);
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, void, BSFixedString, bool>(
			"SmoothCam_SetBoolConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, BSFixedString var, bool value) {
				const auto it = boolSetters.find(var.c_str());
				if (it != boolSetters.end())
					it->second(value);
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, void, BSFixedString, float>(
			"SmoothCam_SetFloatConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, BSFixedString var, float value) {
				const auto it = floatSetters.find(var.c_str());
				if (it != floatSetters.end())
					it->second(value);
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, void, BSFixedString, SInt32>(
			"SmoothCam_SetIntConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, BSFixedString var, SInt32 value) {
				const auto it = intSetters.find(var.c_str());
				if (it != intSetters.end())
					it->second(static_cast<int>(value));
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, BSFixedString, BSFixedString>(
			"SmoothCam_GetStringConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, BSFixedString var) {
				const auto it = stringGetters.find(var.c_str());
				if (it != stringGetters.end())
					return it->second();
				else
					return BSFixedString("");
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, bool, BSFixedString>(
			"SmoothCam_GetBoolConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, BSFixedString var) {
				const auto it = boolGetters.find(var.c_str());
				if (it != boolGetters.end())
					return it->second();
				else
					return false;
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, float, BSFixedString>(
			"SmoothCam_GetFloatConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, BSFixedString var) {
				const auto it = floatGetters.find(var.c_str());
				if (it != floatGetters.end())
					return it->second();
				else
					return 0.0f;
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, SInt32, BSFixedString>(
			"SmoothCam_GetIntConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, BSFixedString var) {
				const auto it = intGetters.find(var.c_str());
				if (it != intGetters.end())
					return static_cast<SInt32>(it->second());
				else
					return static_cast<SInt32>(-1);
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, BSFixedString, SInt32, BSFixedString>(
			"SmoothCam_SaveAsPreset",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, SInt32 index, BSFixedString name) {
				return Config::SaveConfigAsPreset(index, name);
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, bool, SInt32>(
			"SmoothCam_LoadPreset",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, SInt32 index) {
				return Config::LoadPreset(index);
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, BSFixedString, SInt32>(
			"SmoothCam_GetPresetNameAtIndex",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, SInt32 index) {
				return Config::GetPresetSlotName(index);
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, void>(
			"SmoothCam_ResetConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput) {
				Config::ResetConfig();
			},
			registry
		)
	);
}