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

const std::unordered_map<std::string_view, std::function<BSFixedString(void)>> stringGetters = {
	IMPL_SCALAR_METHOD_GETTER("InterpolationMethod", currentScalar)
	IMPL_SCALAR_METHOD_GETTER("SeparateZInterpMethod", separateZScalar)
	IMPL_SCALAR_METHOD_GETTER("SepLocalInterpMethod", separateLocalScalar)
	IMPL_SCALAR_METHOD_GETTER("OffsetTransitionMethod", offsetScalar)
	IMPL_SCALAR_METHOD_GETTER("ZoomTransitionMethod", zoomScalar)
};

const std::unordered_map<std::string_view, std::function<bool(void)>> boolGetters = {
	IMPL_GETTER("FirstPersonHorse",					comaptIC_FirstPersonHorse)
	IMPL_GETTER("FirstPersonDragon",				comaptIC_FirstPersonDragon)
	IMPL_GETTER("FirstPersonSitting",				compatIC_FirstPersonSitting)
	IMPL_GETTER("IFPVCompat",						compatIFPV)
	IMPL_GETTER("InterpolationEnabled",				enableInterp)
	IMPL_GETTER("SeparateLocalInterpolation",		separateLocalInterp)
	IMPL_GETTER("DisableDeltaTime",					disableDeltaTime)
	IMPL_GETTER("DisableDuringDialog",				disableDuringDialog)
	IMPL_GETTER("Enable3DBowCrosshair",				use3DBowAimCrosshair)
	IMPL_GETTER("Enable3DMagicCrosshair",			use3DMagicCrosshair)
	IMPL_GETTER("EnableCrosshairSizeManip",			enableCrosshairSizeManip)
	IMPL_GETTER("HideCrosshairOutOfCombat",			hideNonCombatCrosshair)
	IMPL_GETTER("HideCrosshairMeleeCombat",			hideCrosshairMeleeCombat)
	IMPL_GETTER("SeparateZInterpEnabled",			separateZInterp)
	IMPL_GETTER("OffsetTransitionEnabled",			enableOffsetInterpolation)
	IMPL_GETTER("ZoomTransitionEnabled",			enableZoomInterpolation)

	IMPL_GETTER("CameraDistanceClampXEnable",		cameraDistanceClampXEnable)
	IMPL_GETTER("CameraDistanceClampYEnable",		cameraDistanceClampYEnable)
	IMPL_GETTER("CameraDistanceClampZEnable",		cameraDistanceClampZEnable)
	IMPL_GETTER("ShoulderSwapXClamping",			swapXClamping)

	IMPL_GETTER("InterpStanding",					standing.interp)
	IMPL_GETTER("InterpStandingRangedCombat",		standing.interpRangedCombat)
	IMPL_GETTER("InterpStandingMagicCombat",		standing.interpMagicCombat)
	IMPL_GETTER("InterpStandingMeleeCombat",		standing.interpMeleeCombat)
	IMPL_GETTER("InterpWalking",					walking.interp)
	IMPL_GETTER("InterpWalkingRangedCombat",		walking.interpRangedCombat)
	IMPL_GETTER("InterpWalkingMagicCombat",			walking.interpMagicCombat)
	IMPL_GETTER("InterpWalkingMeleeCombat",			walking.interpMeleeCombat)
	IMPL_GETTER("InterpRunning",					running.interp)
	IMPL_GETTER("InterpRunningRangedCombat",		running.interpRangedCombat)
	IMPL_GETTER("InterpRunningMagicCombat",			running.interpMagicCombat)
	IMPL_GETTER("InterpRunningMeleeCombat",			running.interpMeleeCombat)
	IMPL_GETTER("InterpSprinting",					sprinting.interp)
	IMPL_GETTER("InterpSprintingRangedCombat",		sprinting.interpRangedCombat)
	IMPL_GETTER("InterpSprintingMagicCombat",		sprinting.interpMagicCombat)
	IMPL_GETTER("InterpSprintingMeleeCombat",		sprinting.interpMeleeCombat)
	IMPL_GETTER("InterpSneaking",					sneaking.interp)
	IMPL_GETTER("InterpSneakingRangedCombat",		sneaking.interpRangedCombat)
	IMPL_GETTER("InterpSneakingMagicCombat",		sneaking.interpMagicCombat)
	IMPL_GETTER("InterpSneakingMeleeCombat",		sneaking.interpMeleeCombat)
	IMPL_GETTER("InterpSwimming",					swimming.interp)
	IMPL_GETTER("InterpBowAim",						bowAim.interpRangedCombat)
	IMPL_GETTER("InterpBowAimHorseback",			bowAim.interpHorseback)
	IMPL_GETTER("InterpSitting",					sitting.interp)
	IMPL_GETTER("InterpHorseback",					horseback.interp)
	IMPL_GETTER("InterpHorsebackRangedCombat",		horseback.interpRangedCombat)
	IMPL_GETTER("InterpHorsebackMagicCombat",		horseback.interpMagicCombat)
	IMPL_GETTER("InterpHorsebackMeleeCombat",		horseback.interpMeleeCombat)
};

const std::unordered_map<std::string_view, std::function<float(void)>> floatGetters = {
	IMPL_GETTER("MinFollowDistance",					minCameraFollowDistance)
	IMPL_GETTER("MinCameraFollowRate",					minCameraFollowRate)
	IMPL_GETTER("MaxCameraFollowRate",					maxCameraFollowRate)
	IMPL_GETTER("MaxSmoothingInterpDistance",			zoomMaxSmoothingDistance)
	IMPL_GETTER("ZoomMul",								zoomMul)

	IMPL_GETTER("CrosshairNPCGrowSize",					crosshairNPCHitGrowSize)
	IMPL_GETTER("CrosshairMinDistSize",					crosshairMinDistSize)
	IMPL_GETTER("CrosshairMaxDistSize",					crosshairMaxDistSize)

	IMPL_GETTER("SepZMaxInterpDistance",				separateZMaxSmoothingDistance)
	IMPL_GETTER("SepZMinFollowRate",					separateZMinFollowRate)
	IMPL_GETTER("SepZMaxFollowRate",					separateZMaxFollowRate)
	IMPL_GETTER("SepLocalInterpRate",					localScalarRate)

	IMPL_GETTER("OffsetTransitionDuration",				offsetInterpDurationSecs)
	IMPL_GETTER("ZoomTransitionDuration",				zoomInterpDurationSecs)

	IMPL_GETTER("CameraDistanceClampXMin",				cameraDistanceClampXMin)
	IMPL_GETTER("CameraDistanceClampXMax",				cameraDistanceClampXMax)
	IMPL_GETTER("CameraDistanceClampYMin",				cameraDistanceClampYMin)
	IMPL_GETTER("CameraDistanceClampYMax",				cameraDistanceClampYMax)
	IMPL_GETTER("CameraDistanceClampZMin",				cameraDistanceClampZMin)
	IMPL_GETTER("CameraDistanceClampZMax",				cameraDistanceClampZMax)

	IMPL_GETTER("Standing:SideOffset", 					standing.sideOffset)
	IMPL_GETTER("Standing:UpOffset", 					standing.upOffset)
	IMPL_GETTER("Standing:ZoomOffset", 					standing.zoomOffset)
	IMPL_GETTER("StandingCombat:Ranged:SideOffset",		standing.combatRangedSideOffset)
	IMPL_GETTER("StandingCombat:Ranged:UpOffset", 		standing.combatRangedUpOffset)
	IMPL_GETTER("StandingCombat:Ranged:ZoomOffset",		standing.combatRangedZoomOffset)
	IMPL_GETTER("StandingCombat:Magic:SideOffset", 		standing.combatMagicSideOffset)
	IMPL_GETTER("StandingCombat:Magic:UpOffset", 		standing.combatMagicUpOffset)
	IMPL_GETTER("StandingCombat:Magic:ZoomOffset", 		standing.combatMagicZoomOffset)
	IMPL_GETTER("StandingCombat:Melee:SideOffset",		standing.combatMeleeSideOffset)
	IMPL_GETTER("StandingCombat:Melee:UpOffset", 		standing.combatMeleeUpOffset)
	IMPL_GETTER("StandingCombat:Melee:ZoomOffset", 		standing.combatMeleeZoomOffset)

	IMPL_GETTER("Walking:SideOffset", 					walking.sideOffset)
	IMPL_GETTER("Walking:UpOffset", 					walking.upOffset)
	IMPL_GETTER("Walking:ZoomOffset", 					walking.zoomOffset)
	IMPL_GETTER("WalkingCombat:Ranged:SideOffset",		walking.combatRangedSideOffset)
	IMPL_GETTER("WalkingCombat:Ranged:UpOffset",		walking.combatRangedUpOffset)
	IMPL_GETTER("WalkingCombat:Ranged:ZoomOffset",		walking.combatRangedZoomOffset)
	IMPL_GETTER("WalkingCombat:Magic:SideOffset",		walking.combatMagicSideOffset)
	IMPL_GETTER("WalkingCombat:Magic:UpOffset",			walking.combatMagicUpOffset)
	IMPL_GETTER("WalkingCombat:Magic:ZoomOffset",		walking.combatMagicZoomOffset)
	IMPL_GETTER("WalkingCombat:Melee:SideOffset",		walking.combatMeleeSideOffset)
	IMPL_GETTER("WalkingCombat:Melee:UpOffset",			walking.combatMeleeUpOffset)
	IMPL_GETTER("WalkingCombat:Melee:ZoomOffset",		walking.combatMeleeZoomOffset)

	IMPL_GETTER("Running:SideOffset",					running.sideOffset)
	IMPL_GETTER("Running:UpOffset",						running.upOffset)
	IMPL_GETTER("Running:ZoomOffset",					running.zoomOffset)
	IMPL_GETTER("RunningCombat:Ranged:SideOffset",		running.combatRangedSideOffset)
	IMPL_GETTER("RunningCombat:Ranged:UpOffset",		running.combatRangedUpOffset)
	IMPL_GETTER("RunningCombat:Ranged:ZoomOffset",		running.combatRangedZoomOffset)
	IMPL_GETTER("RunningCombat:Magic:SideOffset",		running.combatMagicSideOffset)
	IMPL_GETTER("RunningCombat:Magic:UpOffset",			running.combatMagicUpOffset)
	IMPL_GETTER("RunningCombat:Magic:ZoomOffset",		running.combatMagicZoomOffset)
	IMPL_GETTER("RunningCombat:Melee:SideOffset",		running.combatMeleeSideOffset)
	IMPL_GETTER("RunningCombat:Melee:UpOffset",			running.combatMeleeUpOffset)
	IMPL_GETTER("RunningCombat:Melee:ZoomOffset",		running.combatMeleeZoomOffset)

	IMPL_GETTER("Sprinting:SideOffset",					sprinting.sideOffset)
	IMPL_GETTER("Sprinting:UpOffset",					sprinting.upOffset)
	IMPL_GETTER("Sprinting:ZoomOffset",					sprinting.zoomOffset)
	IMPL_GETTER("SprintingCombat:Ranged:SideOffset",	sprinting.combatRangedSideOffset)
	IMPL_GETTER("SprintingCombat:Ranged:UpOffset",		sprinting.combatRangedUpOffset)
	IMPL_GETTER("SprintingCombat:Ranged:ZoomOffset",	sprinting.combatRangedZoomOffset)
	IMPL_GETTER("SprintingCombat:Magic:SideOffset",		sprinting.combatMagicSideOffset)
	IMPL_GETTER("SprintingCombat:Magic:UpOffset",		sprinting.combatMagicUpOffset)
	IMPL_GETTER("SprintingCombat:Magic:ZoomOffset",		sprinting.combatMagicZoomOffset)
	IMPL_GETTER("SprintingCombat:Melee:SideOffset",		sprinting.combatMeleeSideOffset)
	IMPL_GETTER("SprintingCombat:Melee:UpOffset",		sprinting.combatMeleeUpOffset)
	IMPL_GETTER("SprintingCombat:Melee:ZoomOffset",		sprinting.combatMeleeZoomOffset)

	IMPL_GETTER("Sneaking:SideOffset",					sneaking.sideOffset)
	IMPL_GETTER("Sneaking:UpOffset",					sneaking.upOffset)
	IMPL_GETTER("Sneaking:ZoomOffset",					sneaking.zoomOffset)
	IMPL_GETTER("SneakingCombat:Ranged:SideOffset",		sneaking.combatRangedSideOffset)
	IMPL_GETTER("SneakingCombat:Ranged:UpOffset",		sneaking.combatRangedUpOffset)
	IMPL_GETTER("SneakingCombat:Ranged:ZoomOffset",		sneaking.combatRangedZoomOffset)
	IMPL_GETTER("SneakingCombat:Magic:SideOffset",		sneaking.combatMagicSideOffset)
	IMPL_GETTER("SneakingCombat:Magic:UpOffset",		sneaking.combatMagicUpOffset)
	IMPL_GETTER("SneakingCombat:Magic:ZoomOffset",		sneaking.combatMagicZoomOffset)
	IMPL_GETTER("SneakingCombat:Melee:SideOffset",		sneaking.combatMeleeSideOffset)
	IMPL_GETTER("SneakingCombat:Melee:UpOffset",		sneaking.combatMeleeUpOffset)
	IMPL_GETTER("SneakingCombat:Melee:ZoomOffset",		sneaking.combatMeleeZoomOffset)

	IMPL_GETTER("Swimming:SideOffset",					swimming.sideOffset)
	IMPL_GETTER("Swimming:UpOffset",					swimming.upOffset)
	IMPL_GETTER("Swimming:ZoomOffset",					swimming.zoomOffset)

	IMPL_GETTER("Bowaim:SideOffset",					bowAim.sideOffset)
	IMPL_GETTER("Bowaim:UpOffset",						bowAim.upOffset)
	IMPL_GETTER("Bowaim:ZoomOffset",					bowAim.zoomOffset)
	IMPL_GETTER("BowaimHorse:SideOffset",				bowAim.horseSideOffset)
	IMPL_GETTER("BowaimHorse:UpOffset",					bowAim.horseUpOffset)
	IMPL_GETTER("BowaimHorse:ZoomOffset",				bowAim.horseZoomOffset)

	IMPL_GETTER("Sitting:SideOffset",					sitting.sideOffset)
	IMPL_GETTER("Sitting:UpOffset",						sitting.upOffset)
	IMPL_GETTER("Sitting:ZoomOffset",					sitting.zoomOffset)

	IMPL_GETTER("Horseback:SideOffset",					horseback.sideOffset)
	IMPL_GETTER("Horseback:UpOffset",					horseback.upOffset)
	IMPL_GETTER("Horseback:ZoomOffset",					horseback.zoomOffset)
	IMPL_GETTER("HorsebackCombat:Ranged:SideOffset",	horseback.combatRangedSideOffset)
	IMPL_GETTER("HorsebackCombat:Ranged:UpOffset",		horseback.combatRangedUpOffset)
	IMPL_GETTER("HorsebackCombat:Ranged:ZoomOffset",	horseback.combatRangedZoomOffset)
	IMPL_GETTER("HorsebackCombat:Magic:SideOffset",		horseback.combatMagicSideOffset)
	IMPL_GETTER("HorsebackCombat:Magic:UpOffset",		horseback.combatMagicUpOffset)
	IMPL_GETTER("HorsebackCombat:Magic:ZoomOffset",		horseback.combatMagicZoomOffset)
	IMPL_GETTER("HorsebackCombat:Melee:SideOffset",		horseback.combatMeleeSideOffset)
	IMPL_GETTER("HorsebackCombat:Melee:UpOffset",		horseback.combatMeleeUpOffset)
	IMPL_GETTER("HorsebackCombat:Melee:ZoomOffset",		horseback.combatMeleeZoomOffset)

	IMPL_GETTER("Dragon:SideOffset",					dragon.sideOffset)
	IMPL_GETTER("Dragon:UpOffset",						dragon.upOffset)
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
};

const std::unordered_map<std::string_view, std::function<void(bool)>> boolSetters = {
	IMPL_SETTER("FirstPersonHorse",					comaptIC_FirstPersonHorse, bool)
	IMPL_SETTER("FirstPersonDragon",				comaptIC_FirstPersonDragon, bool)
	IMPL_SETTER("FirstPersonSitting",				compatIC_FirstPersonSitting, bool)
	IMPL_SETTER("IFPVCompat",						compatIFPV, bool)	
	IMPL_SETTER("InterpolationEnabled",				enableInterp, bool)
	IMPL_SETTER("SeparateLocalInterpolation",		separateLocalInterp, bool)
	IMPL_SETTER("DisableDeltaTime",					disableDeltaTime, bool)
	IMPL_SETTER("DisableDuringDialog",				disableDuringDialog, bool)
	IMPL_SETTER("Enable3DBowCrosshair",				use3DBowAimCrosshair, bool)
	IMPL_SETTER("Enable3DMagicCrosshair",			use3DMagicCrosshair, bool)
	IMPL_SETTER("EnableCrosshairSizeManip",			enableCrosshairSizeManip, bool)
	IMPL_SETTER("HideCrosshairOutOfCombat",			hideNonCombatCrosshair, bool)
	IMPL_SETTER("HideCrosshairMeleeCombat",			hideCrosshairMeleeCombat, bool)
	IMPL_SETTER("SeparateZInterpEnabled",			separateZInterp, bool)
	IMPL_SETTER("OffsetTransitionEnabled",			enableOffsetInterpolation, bool)
	IMPL_SETTER("ZoomTransitionEnabled",			enableZoomInterpolation, bool)

	IMPL_SETTER("CameraDistanceClampXEnable",		cameraDistanceClampXEnable, bool)
	IMPL_SETTER("CameraDistanceClampYEnable",		cameraDistanceClampYEnable, bool)
	IMPL_SETTER("CameraDistanceClampZEnable",		cameraDistanceClampZEnable, bool)
	IMPL_SETTER("ShoulderSwapXClamping",			swapXClamping, bool)

	IMPL_SETTER("InterpStanding",					standing.interp, bool)
	IMPL_SETTER("InterpStandingRangedCombat",		standing.interpRangedCombat, bool)
	IMPL_SETTER("InterpStandingMagicCombat",		standing.interpMagicCombat, bool)
	IMPL_SETTER("InterpStandingMeleeCombat",		standing.interpMeleeCombat, bool)
	IMPL_SETTER("InterpWalking",					walking.interp, bool)
	IMPL_SETTER("InterpWalkingRangedCombat",		walking.interpRangedCombat, bool)
	IMPL_SETTER("InterpWalkingMagicCombat",			walking.interpMagicCombat, bool)
	IMPL_SETTER("InterpWalkingMeleeCombat",			walking.interpMeleeCombat, bool)
	IMPL_SETTER("InterpRunning",					running.interp, bool)
	IMPL_SETTER("InterpRunningRangedCombat",		running.interpRangedCombat, bool)
	IMPL_SETTER("InterpRunningMagicCombat",			running.interpMagicCombat, bool)
	IMPL_SETTER("InterpRunningMeleeCombat",			running.interpMeleeCombat, bool)
	IMPL_SETTER("InterpSprinting",					sprinting.interp, bool)
	IMPL_SETTER("InterpSprintingRangedCombat",		sprinting.interpRangedCombat, bool)
	IMPL_SETTER("InterpSprintingMagicCombat",		sprinting.interpMagicCombat, bool)
	IMPL_SETTER("InterpSprintingMeleeCombat",		sprinting.interpMeleeCombat, bool)
	IMPL_SETTER("InterpSneaking",					sneaking.interp, bool)
	IMPL_SETTER("InterpSneakingRangedCombat",		sneaking.interpRangedCombat, bool)
	IMPL_SETTER("InterpSneakingMagicCombat",		sneaking.interpMagicCombat, bool)
	IMPL_SETTER("InterpSneakingMeleeCombat",		sneaking.interpMeleeCombat, bool)
	IMPL_SETTER("InterpSwimming",					swimming.interp, bool)
	IMPL_SETTER("InterpBowAim",						bowAim.interpRangedCombat, bool)
	IMPL_SETTER("InterpBowAimHorseback",			bowAim.interpHorseback, bool)
	IMPL_SETTER("InterpSitting",					sitting.interp, bool)
	IMPL_SETTER("InterpHorseback",					horseback.interp, bool)
	IMPL_SETTER("InterpHorsebackRangedCombat",		horseback.interpRangedCombat, bool)
	IMPL_SETTER("InterpHorsebackMagicCombat",		horseback.interpMagicCombat, bool)
	IMPL_SETTER("InterpHorsebackMeleeCombat",		horseback.interpMeleeCombat, bool)
};

const std::unordered_map<std::string_view, std::function<void(float)>> floatSetters = {
	IMPL_SETTER("MinFollowDistance",					minCameraFollowDistance, float)
	IMPL_SETTER("MinCameraFollowRate",					minCameraFollowRate, float)
	IMPL_SETTER("MaxCameraFollowRate",					maxCameraFollowRate, float)
	IMPL_SETTER("MaxSmoothingInterpDistance",			zoomMaxSmoothingDistance, float)
	IMPL_SETTER("ZoomMul",								zoomMul, float)

	IMPL_SETTER("CrosshairNPCGrowSize",					crosshairNPCHitGrowSize, float)
	IMPL_SETTER("CrosshairMinDistSize",					crosshairMinDistSize, float)
	IMPL_SETTER("CrosshairMaxDistSize",					crosshairMaxDistSize, float)

	IMPL_SETTER("SepZMaxInterpDistance",				separateZMaxSmoothingDistance, float)
	IMPL_SETTER("SepZMinFollowRate",					separateZMinFollowRate, float)
	IMPL_SETTER("SepZMaxFollowRate",					separateZMaxFollowRate, float)
	IMPL_SETTER("SepLocalInterpRate",					localScalarRate, float)

	IMPL_SETTER("OffsetTransitionDuration",				offsetInterpDurationSecs, float)
	IMPL_SETTER("ZoomTransitionDuration",				zoomInterpDurationSecs, float)

	IMPL_SETTER("CameraDistanceClampXMin",				cameraDistanceClampXMin, float)
	IMPL_SETTER("CameraDistanceClampXMax",				cameraDistanceClampXMax, float)
	IMPL_SETTER("CameraDistanceClampYMin",				cameraDistanceClampYMin, float)
	IMPL_SETTER("CameraDistanceClampYMax",				cameraDistanceClampYMax, float)
	IMPL_SETTER("CameraDistanceClampZMin",				cameraDistanceClampZMin, float)
	IMPL_SETTER("CameraDistanceClampZMax",				cameraDistanceClampZMax, float)

	IMPL_SETTER("Standing:SideOffset", 					standing.sideOffset, float)
	IMPL_SETTER("Standing:UpOffset", 					standing.upOffset, float)
	IMPL_SETTER("Standing:ZoomOffset", 					standing.zoomOffset, float)
	IMPL_SETTER("StandingCombat:Ranged:SideOffset",		standing.combatRangedSideOffset, float)
	IMPL_SETTER("StandingCombat:Ranged:UpOffset", 		standing.combatRangedUpOffset, float)
	IMPL_SETTER("StandingCombat:Ranged:ZoomOffset",		standing.combatRangedZoomOffset, float)
	IMPL_SETTER("StandingCombat:Magic:SideOffset", 		standing.combatMagicSideOffset, float)
	IMPL_SETTER("StandingCombat:Magic:UpOffset", 		standing.combatMagicUpOffset, float)
	IMPL_SETTER("StandingCombat:Magic:ZoomOffset", 		standing.combatMagicZoomOffset, float)
	IMPL_SETTER("StandingCombat:Melee:SideOffset",		standing.combatMeleeSideOffset, float)
	IMPL_SETTER("StandingCombat:Melee:UpOffset", 		standing.combatMeleeUpOffset, float)
	IMPL_SETTER("StandingCombat:Melee:ZoomOffset", 		standing.combatMeleeZoomOffset, float)

	IMPL_SETTER("Walking:SideOffset", 					walking.sideOffset, float)
	IMPL_SETTER("Walking:UpOffset", 					walking.upOffset, float)
	IMPL_SETTER("Walking:ZoomOffset", 					walking.zoomOffset, float)
	IMPL_SETTER("WalkingCombat:Ranged:SideOffset",		walking.combatRangedSideOffset, float)
	IMPL_SETTER("WalkingCombat:Ranged:UpOffset",		walking.combatRangedUpOffset, float)
	IMPL_SETTER("WalkingCombat:Ranged:ZoomOffset",		walking.combatRangedZoomOffset, float)
	IMPL_SETTER("WalkingCombat:Magic:SideOffset",		walking.combatMagicSideOffset, float)
	IMPL_SETTER("WalkingCombat:Magic:UpOffset",			walking.combatMagicUpOffset, float)
	IMPL_SETTER("WalkingCombat:Magic:ZoomOffset",		walking.combatMagicZoomOffset, float)
	IMPL_SETTER("WalkingCombat:Melee:SideOffset",		walking.combatMeleeSideOffset, float)
	IMPL_SETTER("WalkingCombat:Melee:UpOffset",			walking.combatMeleeUpOffset, float)
	IMPL_SETTER("WalkingCombat:Melee:ZoomOffset",		walking.combatMeleeZoomOffset, float)

	IMPL_SETTER("Running:SideOffset",					running.sideOffset, float)
	IMPL_SETTER("Running:UpOffset",						running.upOffset, float)
	IMPL_SETTER("Running:ZoomOffset",					running.zoomOffset, float)
	IMPL_SETTER("RunningCombat:Ranged:SideOffset",		running.combatRangedSideOffset, float)
	IMPL_SETTER("RunningCombat:Ranged:UpOffset",		running.combatRangedUpOffset, float)
	IMPL_SETTER("RunningCombat:Ranged:ZoomOffset",		running.combatRangedZoomOffset, float)
	IMPL_SETTER("RunningCombat:Magic:SideOffset",		running.combatMagicSideOffset, float)
	IMPL_SETTER("RunningCombat:Magic:UpOffset",			running.combatMagicUpOffset, float)
	IMPL_SETTER("RunningCombat:Magic:ZoomOffset",		running.combatMagicZoomOffset, float)
	IMPL_SETTER("RunningCombat:Melee:SideOffset",		running.combatMeleeSideOffset, float)
	IMPL_SETTER("RunningCombat:Melee:UpOffset",			running.combatMeleeUpOffset, float)
	IMPL_SETTER("RunningCombat:Melee:ZoomOffset",		running.combatMeleeZoomOffset, float)

	IMPL_SETTER("Sprinting:SideOffset",					sprinting.sideOffset, float)
	IMPL_SETTER("Sprinting:UpOffset",					sprinting.upOffset, float)
	IMPL_SETTER("Sprinting:ZoomOffset",					sprinting.zoomOffset, float)
	IMPL_SETTER("SprintingCombat:Ranged:SideOffset",	sprinting.combatRangedSideOffset, float)
	IMPL_SETTER("SprintingCombat:Ranged:UpOffset",		sprinting.combatRangedUpOffset, float)
	IMPL_SETTER("SprintingCombat:Ranged:ZoomOffset",	sprinting.combatRangedZoomOffset, float)
	IMPL_SETTER("SprintingCombat:Magic:SideOffset",		sprinting.combatMagicSideOffset, float)
	IMPL_SETTER("SprintingCombat:Magic:UpOffset",		sprinting.combatMagicUpOffset, float)
	IMPL_SETTER("SprintingCombat:Magic:ZoomOffset",		sprinting.combatMagicZoomOffset, float)
	IMPL_SETTER("SprintingCombat:Melee:SideOffset",		sprinting.combatMeleeSideOffset, float)
	IMPL_SETTER("SprintingCombat:Melee:UpOffset",		sprinting.combatMeleeUpOffset, float)
	IMPL_SETTER("SprintingCombat:Melee:ZoomOffset",		sprinting.combatMeleeZoomOffset, float)

	IMPL_SETTER("Sneaking:SideOffset",					sneaking.sideOffset, float)
	IMPL_SETTER("Sneaking:UpOffset",					sneaking.upOffset, float)
	IMPL_SETTER("Sneaking:ZoomOffset",					sneaking.zoomOffset, float)
	IMPL_SETTER("SneakingCombat:Ranged:SideOffset",		sneaking.combatRangedSideOffset, float)
	IMPL_SETTER("SneakingCombat:Ranged:UpOffset",		sneaking.combatRangedUpOffset, float)
	IMPL_SETTER("SneakingCombat:Ranged:ZoomOffset",		sneaking.combatRangedZoomOffset, float)
	IMPL_SETTER("SneakingCombat:Magic:SideOffset",		sneaking.combatMagicSideOffset, float)
	IMPL_SETTER("SneakingCombat:Magic:UpOffset",		sneaking.combatMagicUpOffset, float)
	IMPL_SETTER("SneakingCombat:Magic:ZoomOffset",		sneaking.combatMagicZoomOffset, float)
	IMPL_SETTER("SneakingCombat:Melee:SideOffset",		sneaking.combatMeleeSideOffset, float)
	IMPL_SETTER("SneakingCombat:Melee:UpOffset",		sneaking.combatMeleeUpOffset, float)
	IMPL_SETTER("SneakingCombat:Melee:ZoomOffset",		sneaking.combatMeleeZoomOffset, float)

	IMPL_SETTER("Swimming:SideOffset",					swimming.sideOffset, float)
	IMPL_SETTER("Swimming:UpOffset",					swimming.upOffset, float)
	IMPL_SETTER("Swimming:ZoomOffset",					swimming.zoomOffset, float)

	IMPL_SETTER("Bowaim:SideOffset",					bowAim.sideOffset, float)
	IMPL_SETTER("Bowaim:UpOffset",						bowAim.upOffset, float)
	IMPL_SETTER("Bowaim:ZoomOffset",					bowAim.zoomOffset, float)
	IMPL_SETTER("BowaimHorse:SideOffset",				bowAim.horseSideOffset, float)
	IMPL_SETTER("BowaimHorse:UpOffset",					bowAim.horseUpOffset, float)
	IMPL_SETTER("BowaimHorse:ZoomOffset",				bowAim.horseZoomOffset, float)

	IMPL_SETTER("Sitting:SideOffset",					sitting.sideOffset, float)
	IMPL_SETTER("Sitting:UpOffset",						sitting.upOffset, float)
	IMPL_SETTER("Sitting:ZoomOffset",					sitting.zoomOffset, float)

	IMPL_SETTER("Horseback:SideOffset",					horseback.sideOffset, float)
	IMPL_SETTER("Horseback:UpOffset",					horseback.upOffset, float)
	IMPL_SETTER("Horseback:ZoomOffset",					horseback.zoomOffset, float)
	IMPL_SETTER("HorsebackCombat:Ranged:SideOffset",	horseback.combatRangedSideOffset, float)
	IMPL_SETTER("HorsebackCombat:Ranged:UpOffset",		horseback.combatRangedUpOffset, float)
	IMPL_SETTER("HorsebackCombat:Ranged:ZoomOffset",	horseback.combatRangedZoomOffset, float)
	IMPL_SETTER("HorsebackCombat:Magic:SideOffset",		horseback.combatMagicSideOffset, float)
	IMPL_SETTER("HorsebackCombat:Magic:UpOffset",		horseback.combatMagicUpOffset, float)
	IMPL_SETTER("HorsebackCombat:Magic:ZoomOffset",		horseback.combatMagicZoomOffset, float)
	IMPL_SETTER("HorsebackCombat:Melee:SideOffset",		horseback.combatMeleeSideOffset, float)
	IMPL_SETTER("HorsebackCombat:Melee:UpOffset",		horseback.combatMeleeUpOffset, float)
	IMPL_SETTER("HorsebackCombat:Melee:ZoomOffset",		horseback.combatMeleeZoomOffset, float)

	IMPL_SETTER("Dragon:SideOffset",					dragon.sideOffset, float)
	IMPL_SETTER("Dragon:UpOffset",						dragon.upOffset, float)

	// We can ignore getters for these as we just return 0.0f if not found, which is what we want in this case
	IMPL_GROUP_SETTER("Group:SideOffset",				sideOffset, float)
	IMPL_GROUP_SETTER("Group:UpOffset",					upOffset, float)
	IMPL_GROUP_SETTER("Group:ZoomOffset",				zoomOffset, float)
	IMPL_GROUP_SETTER("Group:Ranged:SideOffset",		combatRangedSideOffset, float)
	IMPL_GROUP_SETTER("Group:Ranged:UpOffset",			combatRangedUpOffset, float)
	IMPL_GROUP_SETTER("Group:Ranged:ZoomOffset",		combatRangedZoomOffset, float)
	IMPL_GROUP_SETTER("Group:Magic:SideOffset",			combatMagicSideOffset, float)
	IMPL_GROUP_SETTER("Group:Magic:UpOffset",			combatMagicUpOffset, float)
	IMPL_GROUP_SETTER("Group:Magic:ZoomOffset",			combatMagicZoomOffset, float)
	IMPL_GROUP_SETTER("Group:Melee:SideOffset",			combatMeleeSideOffset, float)
	IMPL_GROUP_SETTER("Group:Melee:UpOffset",			combatMeleeUpOffset, float)
	IMPL_GROUP_SETTER("Group:Melee:ZoomOffset",			combatMeleeZoomOffset, float)
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