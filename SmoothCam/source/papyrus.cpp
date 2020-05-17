#include "papyrus.h"

/* I kind of turned my brain off for all of this */

using namespace PapyrusBindings;
#define IMPL_GETTER(Mapping, Var)                   \
	{ Mapping, []() noexcept {                      \
		return Config::GetCurrentConfig()->Var;     \
	} },

#define IMPL_SETTER(Mapping, Var, Type)             \
	{ Mapping, [](Type arg) {                       \
		Config::GetCurrentConfig()->Var = arg;      \
		Config::SaveCurrentConfig();                \
	} },

#define IMPL_SCALAR_METHOD_GETTER(Mapping, Var)													\
	{ Mapping, []() {																			\
		const auto it = Config::scalarMethodRevLookup.find(Config::GetCurrentConfig()->Var);	\
		if (it != Config::scalarMethodRevLookup.end())											\
			return BSFixedString(it->second.c_str());											\
		else																					\
			return BSFixedString("linear");														\
	} },

#define IMPL_SCALAR_METHOD_SETTER(Mapping, Var)						\
	{ Mapping, [](BSFixedString str) {								\
		const auto it = Config::scalarMethods.find(str.c_str());	\
		if (it != Config::scalarMethods.end()) {					\
			Config::GetCurrentConfig()->Var = it->second;			\
			Config::SaveCurrentConfig();							\
		}															\
	} },

const std::unordered_map<ConfigStringMapping, std::function<BSFixedString(void)>> stringGetters = {
	IMPL_SCALAR_METHOD_GETTER(ConfigStringMapping::InterpolationMethod, currentScalar)
	IMPL_SCALAR_METHOD_GETTER(ConfigStringMapping::SepZInterpMethod, separateZScalar)
	IMPL_SCALAR_METHOD_GETTER(ConfigStringMapping::SepLocalInterpMethod, separateLocalScalar)
	IMPL_SCALAR_METHOD_GETTER(ConfigStringMapping::OffsetTransitionMethod, offsetScalar)
	IMPL_SCALAR_METHOD_GETTER(ConfigStringMapping::ZoomTransitionMethod, zoomScalar)
};

const std::unordered_map<ConfigStringMapping, std::function<bool(void)>> boolGetters = {
	IMPL_GETTER(ConfigStringMapping::FirstPersonHorse,					comaptIC_FirstPersonHorse)
	IMPL_GETTER(ConfigStringMapping::FirstPersonDragon,					comaptIC_FirstPersonDragon)
	IMPL_GETTER(ConfigStringMapping::FirstPersonSitting,				compatIC_FirstPersonSitting)
	IMPL_GETTER(ConfigStringMapping::InterpolationEnabled,				enableInterp)
	IMPL_GETTER(ConfigStringMapping::SeparateLocalInterpolation,		separateLocalInterp)
	IMPL_GETTER(ConfigStringMapping::DisableDeltaTime,					disableDeltaTime)
	IMPL_GETTER(ConfigStringMapping::DisableDuringDialog,				disableDuringDialog)
	IMPL_GETTER(ConfigStringMapping::Crosshair3DEnabled,				enable3DCrosshair)
	IMPL_GETTER(ConfigStringMapping::AlwaysUse3DCrosshair,              alwaysUse3DCrosshair)
	IMPL_GETTER(ConfigStringMapping::HideCrosshairOutOfCombat,			hideNonCombatCrosshair)
	IMPL_GETTER(ConfigStringMapping::HideCrosshairMeleeCombat,			hideCrosshairMeleeCombat)
	IMPL_GETTER(ConfigStringMapping::SepZInterpEnabled,					separateZInterp)
	IMPL_GETTER(ConfigStringMapping::OffsetTransitionEnabled,			enableOffsetInterpolation)
	IMPL_GETTER(ConfigStringMapping::ZoomTransitionEnabled,				enableZoomInterpolation)

	IMPL_GETTER(ConfigStringMapping::CameraDistanceClampXEnable,		cameraDistanceClampXEnable)
	IMPL_GETTER(ConfigStringMapping::CameraDistanceClampYEnable,		cameraDistanceClampYEnable)
	IMPL_GETTER(ConfigStringMapping::CameraDistanceClampZEnable,		cameraDistanceClampZEnable)

	IMPL_GETTER(ConfigStringMapping::InterpStanding,					standing.interp)
	IMPL_GETTER(ConfigStringMapping::InterpStandingRangedCombat,		standing.interpRangedCombat)
	IMPL_GETTER(ConfigStringMapping::InterpStandingMagicCombat,			standing.interpMagicCombat)
	IMPL_GETTER(ConfigStringMapping::InterpStandingMeleeCombat,			standing.interpMeleeCombat)
	IMPL_GETTER(ConfigStringMapping::InterpWalking,						walking.interp)
	IMPL_GETTER(ConfigStringMapping::InterpWalkingRangedCombat,			walking.interpRangedCombat)
	IMPL_GETTER(ConfigStringMapping::InterpWalkingMagicCombat,			walking.interpMagicCombat)
	IMPL_GETTER(ConfigStringMapping::InterpWalkingMeleeCombat,			walking.interpMeleeCombat)
	IMPL_GETTER(ConfigStringMapping::InterpRunning,						running.interp)
	IMPL_GETTER(ConfigStringMapping::InterpRunningRangedCombat,			running.interpRangedCombat)
	IMPL_GETTER(ConfigStringMapping::InterpRunningMagicCombat,			running.interpMagicCombat)
	IMPL_GETTER(ConfigStringMapping::InterpRunningMeleeCombat,			running.interpMeleeCombat)
	IMPL_GETTER(ConfigStringMapping::InterpSprinting,					sprinting.interp)
	IMPL_GETTER(ConfigStringMapping::InterpSprintingRangedCombat,		sprinting.interpRangedCombat)
	IMPL_GETTER(ConfigStringMapping::InterpSprintingMagicCombat,		sprinting.interpMagicCombat)
	IMPL_GETTER(ConfigStringMapping::InterpSprintingMeleeCombat,		sprinting.interpMeleeCombat)
	IMPL_GETTER(ConfigStringMapping::InterpSneaking,					sneaking.interp)
	IMPL_GETTER(ConfigStringMapping::InterpSneakingRangedCombat,		sneaking.interpRangedCombat)
	IMPL_GETTER(ConfigStringMapping::InterpSneakingMagicCombat,			sneaking.interpMagicCombat)
	IMPL_GETTER(ConfigStringMapping::InterpSneakingMeleeCombat,			sneaking.interpMeleeCombat)
	IMPL_GETTER(ConfigStringMapping::InterpSwimming,					swimming.interp)
	IMPL_GETTER(ConfigStringMapping::InterpBowAim,						bowAim.interpRangedCombat)
	IMPL_GETTER(ConfigStringMapping::InterpBowAimHorseback,				bowAim.interpHorseback)
	IMPL_GETTER(ConfigStringMapping::InterpSitting,						sitting.interp)
	IMPL_GETTER(ConfigStringMapping::InterpHorseback,					horseback.interp)
	IMPL_GETTER(ConfigStringMapping::InterpHorsebackRangedCombat,		horseback.interpRangedCombat)
	IMPL_GETTER(ConfigStringMapping::InterpHorsebackMagicCombat,		horseback.interpMagicCombat)
	IMPL_GETTER(ConfigStringMapping::InterpHorsebackMeleeCombat,		horseback.interpMeleeCombat)
};

const std::unordered_map<ConfigStringMapping, std::function<float(void)>> floatGetters = {
	IMPL_GETTER(ConfigStringMapping::MinFollowDistance,					minCameraFollowDistance)
	IMPL_GETTER(ConfigStringMapping::MinCameraFollowRate,				minCameraFollowRate)
	IMPL_GETTER(ConfigStringMapping::MaxCameraFollowRate,				maxCameraFollowRate)
	IMPL_GETTER(ConfigStringMapping::MaxSmoothingInterpDistance,		zoomMaxSmoothingDistance)
	IMPL_GETTER(ConfigStringMapping::ZoomMul,							zoomMul)

	IMPL_GETTER(ConfigStringMapping::CrosshairNPCGrowSize,              crosshairNPCHitGrowSize)
	IMPL_GETTER(ConfigStringMapping::CrosshairMinDistSize,              crosshairMinDistSize)
	IMPL_GETTER(ConfigStringMapping::CrosshairMaxDistSize,              crosshairMaxDistSize)

	IMPL_GETTER(ConfigStringMapping::SepZMaxInterpDistance,				separateZMaxSmoothingDistance)
	IMPL_GETTER(ConfigStringMapping::SepZMinFollowRate,					separateZMinFollowRate)
	IMPL_GETTER(ConfigStringMapping::SepZMaxFollowRate,					separateZMaxFollowRate)
	IMPL_GETTER(ConfigStringMapping::SepLocalInterpRate,                localScalarRate)

	IMPL_GETTER(ConfigStringMapping::OffsetTransitionDuration,			offsetInterpDurationSecs)
	IMPL_GETTER(ConfigStringMapping::ZoomTransitionDuration,			zoomInterpDurationSecs)

	IMPL_GETTER(ConfigStringMapping::CameraDistanceClampXMin,			cameraDistanceClampXMin)
	IMPL_GETTER(ConfigStringMapping::CameraDistanceClampXMax,			cameraDistanceClampXMax)
	IMPL_GETTER(ConfigStringMapping::CameraDistanceClampYMin,			cameraDistanceClampYMin)
	IMPL_GETTER(ConfigStringMapping::CameraDistanceClampYMax,			cameraDistanceClampYMax)
	IMPL_GETTER(ConfigStringMapping::CameraDistanceClampZMin,			cameraDistanceClampZMin)
	IMPL_GETTER(ConfigStringMapping::CameraDistanceClampZMax,			cameraDistanceClampZMax)

	IMPL_GETTER(ConfigStringMapping::StandingSideOffset, 				standing.sideOffset)
	IMPL_GETTER(ConfigStringMapping::StandingUpOffset, 					standing.upOffset)
	IMPL_GETTER(ConfigStringMapping::StandingCombatRangedSideOffset, 	standing.combatRangedSideOffset)
	IMPL_GETTER(ConfigStringMapping::StandingCombatRangedUpOffset, 		standing.combatRangedUpOffset)
	IMPL_GETTER(ConfigStringMapping::StandingCombatMagicSideOffset, 	standing.combatMagicSideOffset)
	IMPL_GETTER(ConfigStringMapping::StandingCombatMagicUpOffset, 		standing.combatMagicUpOffset)
	IMPL_GETTER(ConfigStringMapping::StandingCombatMeleeSideOffset,		standing.combatMeleeSideOffset)
	IMPL_GETTER(ConfigStringMapping::StandingCombatMeleeUpOffset, 		standing.combatMeleeUpOffset)

	IMPL_GETTER(ConfigStringMapping::WalkingSideOffset, 				walking.sideOffset)
	IMPL_GETTER(ConfigStringMapping::WalkingUpOffset, 					walking.upOffset)
	IMPL_GETTER(ConfigStringMapping::WalkingCombatRangedSideOffset, 	walking.combatRangedSideOffset)
	IMPL_GETTER(ConfigStringMapping::WalkingCombatRangedUpOffset, 		walking.combatRangedUpOffset)
	IMPL_GETTER(ConfigStringMapping::WalkingCombatMagicSideOffset, 		walking.combatMagicSideOffset)
	IMPL_GETTER(ConfigStringMapping::WalkingCombatMagicUpOffset, 		walking.combatMagicUpOffset)
	IMPL_GETTER(ConfigStringMapping::WalkingCombatMeleeSideOffset, 		walking.combatMeleeSideOffset)
	IMPL_GETTER(ConfigStringMapping::WalkingCombatMeleeUpOffset, 		walking.combatMeleeUpOffset)

	IMPL_GETTER(ConfigStringMapping::RunningSideOffset, 				running.sideOffset)
	IMPL_GETTER(ConfigStringMapping::RunningUpOffset, 					running.upOffset)
	IMPL_GETTER(ConfigStringMapping::RunningCombatRangedSideOffset, 	running.combatRangedSideOffset)
	IMPL_GETTER(ConfigStringMapping::RunningCombatRangedUpOffset, 		running.combatRangedUpOffset)
	IMPL_GETTER(ConfigStringMapping::RunningCombatMagicSideOffset, 		running.combatMagicSideOffset)
	IMPL_GETTER(ConfigStringMapping::RunningCombatMagicUpOffset, 		running.combatMagicUpOffset)
	IMPL_GETTER(ConfigStringMapping::RunningCombatMeleeSideOffset, 		running.combatMeleeSideOffset)
	IMPL_GETTER(ConfigStringMapping::RunningCombatMeleeUpOffset, 		running.combatMeleeUpOffset)

	IMPL_GETTER(ConfigStringMapping::SprintingSideOffest, 				sprinting.sideOffset)
	IMPL_GETTER(ConfigStringMapping::SprintingUpOffset, 				sprinting.upOffset)
	IMPL_GETTER(ConfigStringMapping::SprintingCombatRangedSideOffset, 	sprinting.combatRangedSideOffset)
	IMPL_GETTER(ConfigStringMapping::SprintingCombatRangedUpOffset, 	sprinting.combatRangedUpOffset)
	IMPL_GETTER(ConfigStringMapping::SprintingCombatMagicSideOffset, 	sprinting.combatMagicSideOffset)
	IMPL_GETTER(ConfigStringMapping::SprintingCombatMagicUpOffset, 		sprinting.combatMagicUpOffset)
	IMPL_GETTER(ConfigStringMapping::SprintingCombatMeleeSideOffset, 	sprinting.combatMeleeSideOffset)
	IMPL_GETTER(ConfigStringMapping::SprintingCombatMeleeUpOffset, 		sprinting.combatMeleeUpOffset)

	IMPL_GETTER(ConfigStringMapping::SneakingSideOffset, 				sneaking.sideOffset)
	IMPL_GETTER(ConfigStringMapping::SneakingUpOffset, 					sneaking.upOffset)
	IMPL_GETTER(ConfigStringMapping::SneakingCombatRangedSideOffset, 	sneaking.combatRangedSideOffset)
	IMPL_GETTER(ConfigStringMapping::SneakingCombatRangedUpOffset, 		sneaking.combatRangedUpOffset)
	IMPL_GETTER(ConfigStringMapping::SneakingCombatMagicSideOffset, 	sneaking.combatMagicSideOffset)
	IMPL_GETTER(ConfigStringMapping::SneakingCombatMagicUpOffset, 		sneaking.combatMagicUpOffset)
	IMPL_GETTER(ConfigStringMapping::SneakingCombatMeleeSideOffset, 	sneaking.combatMeleeSideOffset)
	IMPL_GETTER(ConfigStringMapping::SneakingCombatMeleeUpOffset, 		sneaking.combatMeleeUpOffset)

	IMPL_GETTER(ConfigStringMapping::SwimmingSideOffset,				swimming.sideOffset)
	IMPL_GETTER(ConfigStringMapping::SwimmingUpOffset,					swimming.upOffset)

	IMPL_GETTER(ConfigStringMapping::BowAimSideOffset,					bowAim.sideOffset)
	IMPL_GETTER(ConfigStringMapping::BowAimUpOffset,					bowAim.upOffset)
	IMPL_GETTER(ConfigStringMapping::BowAimHorseSideOffset,				bowAim.horseSideOffset)
	IMPL_GETTER(ConfigStringMapping::BowAimHorseUpOffset,				bowAim.horseUpOffset)

	IMPL_GETTER(ConfigStringMapping::SittingSideOffset,					sitting.sideOffset)
	IMPL_GETTER(ConfigStringMapping::SittingUpOffset,					sitting.upOffset)

	IMPL_GETTER(ConfigStringMapping::HorseSideOffset,					horseback.sideOffset)
	IMPL_GETTER(ConfigStringMapping::HorseUpOffset,						horseback.upOffset)
	IMPL_GETTER(ConfigStringMapping::HorseCombatRangedSideOffset,		horseback.combatRangedSideOffset)
	IMPL_GETTER(ConfigStringMapping::HorseCombatRangedUpOffset,			horseback.combatRangedUpOffset)
	IMPL_GETTER(ConfigStringMapping::HorseCombatMagicSideOffset,		horseback.combatMagicSideOffset)
	IMPL_GETTER(ConfigStringMapping::HorseCombatMagicUpOffset,			horseback.combatMagicUpOffset)
	IMPL_GETTER(ConfigStringMapping::HorseCombatMeleeSideOffset,		horseback.combatMeleeSideOffset)
	IMPL_GETTER(ConfigStringMapping::HorseCombatMeleeUpOffset,			horseback.combatMeleeUpOffset)

	IMPL_GETTER(ConfigStringMapping::DragonSideOffset,					dragon.sideOffset)
	IMPL_GETTER(ConfigStringMapping::DragonUpOffset,					dragon.upOffset)
};

const std::unordered_map<ConfigStringMapping, std::function<void(BSFixedString)>> stringSetters = {
	IMPL_SCALAR_METHOD_SETTER(ConfigStringMapping::InterpolationMethod, currentScalar)
	IMPL_SCALAR_METHOD_SETTER(ConfigStringMapping::SepZInterpMethod, separateZScalar)
	IMPL_SCALAR_METHOD_SETTER(ConfigStringMapping::SepLocalInterpMethod, separateLocalScalar)
	IMPL_SCALAR_METHOD_SETTER(ConfigStringMapping::OffsetTransitionMethod, offsetScalar)
	IMPL_SCALAR_METHOD_SETTER(ConfigStringMapping::ZoomTransitionMethod, zoomScalar)
};

const std::unordered_map<ConfigStringMapping, std::function<void(bool)>> boolSetters = {
	IMPL_SETTER(ConfigStringMapping::FirstPersonHorse,					comaptIC_FirstPersonHorse, bool)
	IMPL_SETTER(ConfigStringMapping::FirstPersonDragon,					comaptIC_FirstPersonDragon, bool)
	IMPL_SETTER(ConfigStringMapping::FirstPersonSitting,				compatIC_FirstPersonSitting, bool)
	IMPL_SETTER(ConfigStringMapping::InterpolationEnabled,				enableInterp, bool)
	IMPL_SETTER(ConfigStringMapping::SeparateLocalInterpolation,		separateLocalInterp, bool)
	IMPL_SETTER(ConfigStringMapping::DisableDeltaTime,					disableDeltaTime, bool)
	IMPL_SETTER(ConfigStringMapping::DisableDuringDialog,				disableDuringDialog, bool)
	IMPL_SETTER(ConfigStringMapping::Crosshair3DEnabled,				enable3DCrosshair, bool)
	IMPL_SETTER(ConfigStringMapping::AlwaysUse3DCrosshair,              alwaysUse3DCrosshair, bool)
	IMPL_SETTER(ConfigStringMapping::HideCrosshairOutOfCombat,			hideNonCombatCrosshair, bool)
	IMPL_SETTER(ConfigStringMapping::HideCrosshairMeleeCombat,			hideCrosshairMeleeCombat, bool)
	IMPL_SETTER(ConfigStringMapping::SepZInterpEnabled,					separateZInterp, bool)
	IMPL_SETTER(ConfigStringMapping::OffsetTransitionEnabled,			enableOffsetInterpolation, bool)
	IMPL_SETTER(ConfigStringMapping::ZoomTransitionEnabled,				enableZoomInterpolation, bool)

	IMPL_SETTER(ConfigStringMapping::CameraDistanceClampXEnable,		cameraDistanceClampXEnable, bool)
	IMPL_SETTER(ConfigStringMapping::CameraDistanceClampYEnable,		cameraDistanceClampYEnable, bool)
	IMPL_SETTER(ConfigStringMapping::CameraDistanceClampZEnable,		cameraDistanceClampZEnable, bool)

	IMPL_SETTER(ConfigStringMapping::InterpStanding,					standing.interp, bool)
	IMPL_SETTER(ConfigStringMapping::InterpStandingRangedCombat,		standing.interpRangedCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpStandingMagicCombat,			standing.interpMagicCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpStandingMeleeCombat,			standing.interpMeleeCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpWalking,						walking.interp, bool)
	IMPL_SETTER(ConfigStringMapping::InterpWalkingRangedCombat,			walking.interpRangedCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpWalkingMagicCombat,			walking.interpMagicCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpWalkingMeleeCombat,			walking.interpMeleeCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpRunning,						running.interp, bool)
	IMPL_SETTER(ConfigStringMapping::InterpRunningRangedCombat,			running.interpRangedCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpRunningMagicCombat,			running.interpMagicCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpRunningMeleeCombat,			running.interpMeleeCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpSprinting,					sprinting.interp, bool)
	IMPL_SETTER(ConfigStringMapping::InterpSprintingRangedCombat,		sprinting.interpRangedCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpSprintingMagicCombat,		sprinting.interpMagicCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpSprintingMeleeCombat,		sprinting.interpMeleeCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpSneaking,					sneaking.interp, bool)
	IMPL_SETTER(ConfigStringMapping::InterpSneakingRangedCombat,		sneaking.interpRangedCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpSneakingMagicCombat,			sneaking.interpMagicCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpSneakingMeleeCombat,			sneaking.interpMeleeCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpSwimming,					swimming.interp, bool)
	IMPL_SETTER(ConfigStringMapping::InterpBowAim,						bowAim.interpRangedCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpBowAimHorseback,				bowAim.interpHorseback, bool)
	IMPL_SETTER(ConfigStringMapping::InterpSitting,						sitting.interp, bool)
	IMPL_SETTER(ConfigStringMapping::InterpHorseback,					horseback.interp, bool)
	IMPL_SETTER(ConfigStringMapping::InterpHorsebackRangedCombat,		horseback.interpRangedCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpHorsebackMagicCombat,		horseback.interpMagicCombat, bool)
	IMPL_SETTER(ConfigStringMapping::InterpHorsebackMeleeCombat,		horseback.interpMeleeCombat, bool)
};

const std::unordered_map<ConfigStringMapping, std::function<void(float)>> floatSetters = {
	IMPL_SETTER(ConfigStringMapping::MinFollowDistance,					minCameraFollowDistance, float)
	IMPL_SETTER(ConfigStringMapping::MinCameraFollowRate,				minCameraFollowRate, float)
	IMPL_SETTER(ConfigStringMapping::MaxCameraFollowRate,				maxCameraFollowRate, float)
	IMPL_SETTER(ConfigStringMapping::MaxSmoothingInterpDistance,		zoomMaxSmoothingDistance, float)
	IMPL_SETTER(ConfigStringMapping::ZoomMul,							zoomMul, float)

	IMPL_SETTER(ConfigStringMapping::CrosshairNPCGrowSize,              crosshairNPCHitGrowSize, float)
	IMPL_SETTER(ConfigStringMapping::CrosshairMinDistSize,              crosshairMinDistSize, float)
	IMPL_SETTER(ConfigStringMapping::CrosshairMaxDistSize,              crosshairMaxDistSize, float)

	IMPL_SETTER(ConfigStringMapping::SepZMaxInterpDistance,				separateZMaxSmoothingDistance, float)
	IMPL_SETTER(ConfigStringMapping::SepZMinFollowRate,					separateZMinFollowRate, float)
	IMPL_SETTER(ConfigStringMapping::SepZMaxFollowRate,					separateZMaxFollowRate, float)
	IMPL_SETTER(ConfigStringMapping::SepLocalInterpRate,                localScalarRate, float)

	IMPL_SETTER(ConfigStringMapping::OffsetTransitionDuration,			offsetInterpDurationSecs, float)
	IMPL_SETTER(ConfigStringMapping::ZoomTransitionDuration,			zoomInterpDurationSecs, float)

	IMPL_SETTER(ConfigStringMapping::CameraDistanceClampXMin,			cameraDistanceClampXMin, float)
	IMPL_SETTER(ConfigStringMapping::CameraDistanceClampXMax,			cameraDistanceClampXMax, float)
	IMPL_SETTER(ConfigStringMapping::CameraDistanceClampYMin,			cameraDistanceClampYMin, float)
	IMPL_SETTER(ConfigStringMapping::CameraDistanceClampYMax,			cameraDistanceClampYMax, float)
	IMPL_SETTER(ConfigStringMapping::CameraDistanceClampZMin,			cameraDistanceClampZMin, float)
	IMPL_SETTER(ConfigStringMapping::CameraDistanceClampZMax,			cameraDistanceClampZMax, float)

	IMPL_SETTER(ConfigStringMapping::StandingSideOffset, 				standing.sideOffset, float)
	IMPL_SETTER(ConfigStringMapping::StandingUpOffset, 					standing.upOffset, float)
	IMPL_SETTER(ConfigStringMapping::StandingCombatRangedSideOffset, 	standing.combatRangedSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::StandingCombatRangedUpOffset, 		standing.combatRangedUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::StandingCombatMagicSideOffset, 	standing.combatMagicSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::StandingCombatMagicUpOffset, 		standing.combatMagicUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::StandingCombatMeleeSideOffset,		standing.combatMeleeSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::StandingCombatMeleeUpOffset, 		standing.combatMeleeUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::WalkingSideOffset, 				walking.sideOffset, float)
	IMPL_SETTER(ConfigStringMapping::WalkingUpOffset, 					walking.upOffset, float)
	IMPL_SETTER(ConfigStringMapping::WalkingCombatRangedSideOffset, 	walking.combatRangedSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::WalkingCombatRangedUpOffset, 		walking.combatRangedUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::WalkingCombatMagicSideOffset, 		walking.combatMagicSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::WalkingCombatMagicUpOffset, 		walking.combatMagicUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::WalkingCombatMeleeSideOffset, 		walking.combatMeleeSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::WalkingCombatMeleeUpOffset, 		walking.combatMeleeUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::RunningSideOffset, 				running.sideOffset, float)
	IMPL_SETTER(ConfigStringMapping::RunningUpOffset, 					running.upOffset, float)
	IMPL_SETTER(ConfigStringMapping::RunningCombatRangedSideOffset, 	running.combatRangedSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::RunningCombatRangedUpOffset, 		running.combatRangedUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::RunningCombatMagicSideOffset, 		running.combatMagicSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::RunningCombatMagicUpOffset, 		running.combatMagicUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::RunningCombatMeleeSideOffset, 		running.combatMeleeSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::RunningCombatMeleeUpOffset, 		running.combatMeleeUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::SprintingSideOffest, 				sprinting.sideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SprintingUpOffset, 				sprinting.upOffset, float)
	IMPL_SETTER(ConfigStringMapping::SprintingCombatRangedSideOffset, 	sprinting.combatRangedSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SprintingCombatRangedUpOffset, 	sprinting.combatRangedUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::SprintingCombatMagicSideOffset, 	sprinting.combatMagicSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SprintingCombatMagicUpOffset, 		sprinting.combatMagicUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::SprintingCombatMeleeSideOffset, 	sprinting.combatMeleeSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SprintingCombatMeleeUpOffset, 		sprinting.combatMeleeUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::SneakingSideOffset, 				sneaking.sideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SneakingUpOffset, 					sneaking.upOffset, float)
	IMPL_SETTER(ConfigStringMapping::SneakingCombatRangedSideOffset, 	sneaking.combatRangedSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SneakingCombatRangedUpOffset, 		sneaking.combatRangedUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::SneakingCombatMagicSideOffset, 	sneaking.combatMagicSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SneakingCombatMagicUpOffset, 		sneaking.combatMagicUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::SneakingCombatMeleeSideOffset, 	sneaking.combatMeleeSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SneakingCombatMeleeUpOffset, 		sneaking.combatMeleeUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::SwimmingSideOffset,				swimming.sideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SwimmingUpOffset,					swimming.upOffset, float)

	IMPL_SETTER(ConfigStringMapping::BowAimSideOffset,					bowAim.sideOffset, float)
	IMPL_SETTER(ConfigStringMapping::BowAimUpOffset,					bowAim.upOffset, float)
	IMPL_SETTER(ConfigStringMapping::BowAimHorseSideOffset,				bowAim.horseSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::BowAimHorseUpOffset,				bowAim.horseUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::SittingSideOffset,					sitting.sideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SittingUpOffset,					sitting.upOffset, float)

	IMPL_SETTER(ConfigStringMapping::HorseSideOffset,					horseback.sideOffset, float)
	IMPL_SETTER(ConfigStringMapping::HorseUpOffset,						horseback.upOffset, float)
	IMPL_SETTER(ConfigStringMapping::HorseCombatRangedSideOffset,		horseback.combatRangedSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::HorseCombatRangedUpOffset,			horseback.combatRangedUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::HorseCombatMagicSideOffset,		horseback.combatMagicSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::HorseCombatMagicUpOffset,			horseback.combatMagicUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::HorseCombatMeleeSideOffset,		horseback.combatMeleeSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::HorseCombatMeleeUpOffset,			horseback.combatMeleeUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::DragonSideOffset,					dragon.sideOffset, float)
	IMPL_SETTER(ConfigStringMapping::DragonUpOffset,					dragon.upOffset, float)
};


ConfigVar PapyrusBindings::GetInfo(const BSFixedString& member) {
	const auto it = stringMap.find(member.c_str());
	if (it != stringMap.end()) {
		return it->second;
	}
	return { ConfigStringMapping::INVALID, ConfigType::INVALID };
}

void PapyrusBindings::Bind(VMClassRegistry* registry) {
	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, void, BSFixedString, BSFixedString>(
			"SmoothCam_SetStringConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, BSFixedString var, BSFixedString value) {
				const auto info = GetInfo(var);
				if (info.type != ConfigType::STRING) return;
				const auto it = stringSetters.find(info.mapping);
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
				const auto info = GetInfo(var);
				if (info.type != ConfigType::BOOL) return;
				const auto it = boolSetters.find(info.mapping);
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
				const auto info = GetInfo(var);
				if (info.type != ConfigType::FLOAT) return;
				const auto it = floatSetters.find(info.mapping);
				if (it != floatSetters.end())
					it->second(value);
			},
			registry
		)
	);

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, BSFixedString, BSFixedString>(
			"SmoothCam_GetStringConfig",
			ScriptClassName,
			[](StaticFunctionTag* thisInput, BSFixedString var) {
				const auto info = GetInfo(var);
				if (info.type != ConfigType::STRING) return BSFixedString("");
				const auto it = stringGetters.find(info.mapping);
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
				const auto info = GetInfo(var);
				if (info.type != ConfigType::BOOL) return false;
				const auto it = boolGetters.find(info.mapping);
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
				const auto info = GetInfo(var);
				if (info.type != ConfigType::FLOAT) return 0.0f;
				const auto it = floatGetters.find(info.mapping);
				if (it != floatGetters.end())
					return it->second();
				else
					return 0.0f;
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
}