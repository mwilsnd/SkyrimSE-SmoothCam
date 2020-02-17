#include "pch.h"
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

const std::unordered_map<ConfigStringMapping, std::function<BSFixedString(void)>> stringGetters = {
	{ ConfigStringMapping::InterpolationMethod, []() {
		const auto it = Config::scalarMethodRevLookup.find(Config::GetCurrentConfig()->currentScalar);
		if (it != Config::scalarMethodRevLookup.end())
			return BSFixedString(it->second.c_str());
		else
			return BSFixedString("linear");
	} },
	{ ConfigStringMapping::SepZInterpMethod, []() {
		const auto it = Config::scalarMethodRevLookup.find(Config::GetCurrentConfig()->separateZScalar);
		if (it != Config::scalarMethodRevLookup.end())
			return BSFixedString(it->second.c_str());
		else
			return BSFixedString("linear");
	} },
};

const std::unordered_map<ConfigStringMapping, std::function<bool(void)>> boolGetters = {
	IMPL_GETTER(ConfigStringMapping::FirstPersonHorse, comaptIC_FirstPersonHorse)
	IMPL_GETTER(ConfigStringMapping::FirstPersonDragon, comaptIC_FirstPersonDragon)
	IMPL_GETTER(ConfigStringMapping::FirstPersonSitting, compatIC_FirstPersonSitting)
	IMPL_GETTER(ConfigStringMapping::InterpolationEnabled, enableInterp)
	IMPL_GETTER(ConfigStringMapping::Crosshair3DEnabled, enable3DCrosshair)
	IMPL_GETTER(ConfigStringMapping::SepZInterpEnabled, separateZInterp)
};

const std::unordered_map<ConfigStringMapping, std::function<float(void)>> floatGetters = {
	IMPL_GETTER(ConfigStringMapping::MinFollowDistance, minCameraFollowDistance)
	IMPL_GETTER(ConfigStringMapping::MinCameraFollowRate, minCameraFollowRate)
	IMPL_GETTER(ConfigStringMapping::MaxCameraFollowRate, maxCameraFollowRate)
	IMPL_GETTER(ConfigStringMapping::MaxSmoothingInterpDistance, zoomMaxSmoothingDistance)
	IMPL_GETTER(ConfigStringMapping::ZoomMul, zoomMul)

	IMPL_GETTER(ConfigStringMapping::SepZMaxInterpDistance, separateZMaxSmoothingDistance)
	IMPL_GETTER(ConfigStringMapping::SepZMinFollowRate, separateZMinFollowRate)
	IMPL_GETTER(ConfigStringMapping::SepZMaxFollowRate, separateZMaxFollowRate)

	IMPL_GETTER(ConfigStringMapping::StandingSideOffset, standingSideOffset)
	IMPL_GETTER(ConfigStringMapping::StandingUpOffset, standingUpOffset)
	IMPL_GETTER(ConfigStringMapping::StandingCombatSideOffset, standingCombatSideOffset)
	IMPL_GETTER(ConfigStringMapping::StandingCombatUpOffset, standingCombatUpOffset)

	IMPL_GETTER(ConfigStringMapping::WalkingSideOffset, walkingSideOffset)
	IMPL_GETTER(ConfigStringMapping::WalkingUpOffset, walkingUpOffset)
	IMPL_GETTER(ConfigStringMapping::WalkingCombatSideOffset, walkingCombatSideOffset)
	IMPL_GETTER(ConfigStringMapping::WalkingCombatUpOffset, walkingCombatUpOffset)

	IMPL_GETTER(ConfigStringMapping::RunningSideOffset, runningSideOffset)
	IMPL_GETTER(ConfigStringMapping::RunningUpOffset, runningUpOffset)
	IMPL_GETTER(ConfigStringMapping::RunningCombatSideOffset, runningCombatSideOffset)
	IMPL_GETTER(ConfigStringMapping::RunningCombatUpOffset, runningCombatUpOffset)

	IMPL_GETTER(ConfigStringMapping::SprintingSideOffest, sprintingSideOffset)
	IMPL_GETTER(ConfigStringMapping::SprintingUpOffset, sprintingUpOffset)
	IMPL_GETTER(ConfigStringMapping::SprintingCombatSideOffset, sprintingCombatSideOffset)
	IMPL_GETTER(ConfigStringMapping::SprintingCombatUpOffset, sprintingCombatUpOffset)

	IMPL_GETTER(ConfigStringMapping::SneakingSideOffset, sneakingSideOffset)
	IMPL_GETTER(ConfigStringMapping::SneakingUpOffset, sneakingUpOffset)
	IMPL_GETTER(ConfigStringMapping::SneakingCombatSideOffset, sneakingCombatSideOffset)
	IMPL_GETTER(ConfigStringMapping::SneakingCombatUpOffset, sneakingCombatUpOffset)

	IMPL_GETTER(ConfigStringMapping::SwimmingSideOffset, swimmingSideOffset)
	IMPL_GETTER(ConfigStringMapping::SwimmingUpOffset, swimmingUpOffset)

	IMPL_GETTER(ConfigStringMapping::BowAimSideOffset, bowAimSideOffset)
	IMPL_GETTER(ConfigStringMapping::BowAimUpOffset, bowAimUpOffset)
	IMPL_GETTER(ConfigStringMapping::BowAimHorseSideOffset, bowAimHorseSideOffset)
	IMPL_GETTER(ConfigStringMapping::BowAimHorseUpOffset, bowAimHorseUpOffset)

	IMPL_GETTER(ConfigStringMapping::SittingSideOffset, sittingSideOffset)
	IMPL_GETTER(ConfigStringMapping::SittingUpOffset, sittingUpOffset)

	IMPL_GETTER(ConfigStringMapping::HorseSideOffset, horsebackSideOffset)
	IMPL_GETTER(ConfigStringMapping::HorseUpOffset, horsebackUpOffset)
	IMPL_GETTER(ConfigStringMapping::DragonSideOffset, dragonSideOffset)
	IMPL_GETTER(ConfigStringMapping::DragonUpOffset, dragonUpOffset)
};

const std::unordered_map<ConfigStringMapping, std::function<void(BSFixedString)>> stringSetters = {
	{ ConfigStringMapping::InterpolationMethod, [](BSFixedString str) {
		const auto it = Config::scalarMethods.find(str.c_str());
		if (it != Config::scalarMethods.end()) {
			Config::GetCurrentConfig()->currentScalar = it->second;
			Config::SaveCurrentConfig();
		}
	} },
	{ ConfigStringMapping::SepZInterpMethod, [](BSFixedString str) {
		const auto it = Config::scalarMethods.find(str.c_str());
		if (it != Config::scalarMethods.end()) {
			Config::GetCurrentConfig()->separateZScalar = it->second;
			Config::SaveCurrentConfig();
		}
	} },
};

const std::unordered_map<ConfigStringMapping, std::function<void(bool)>> boolSetters = {
	IMPL_SETTER(ConfigStringMapping::FirstPersonHorse, comaptIC_FirstPersonHorse, bool)
	IMPL_SETTER(ConfigStringMapping::FirstPersonDragon, comaptIC_FirstPersonDragon, bool)
	IMPL_SETTER(ConfigStringMapping::FirstPersonSitting, compatIC_FirstPersonSitting, bool)
	IMPL_SETTER(ConfigStringMapping::InterpolationEnabled, enableInterp, bool)
	IMPL_SETTER(ConfigStringMapping::Crosshair3DEnabled, enable3DCrosshair, bool)
	IMPL_SETTER(ConfigStringMapping::SepZInterpEnabled, separateZInterp, bool)
};

const std::unordered_map<ConfigStringMapping, std::function<void(float)>> floatSetters = {
	IMPL_SETTER(ConfigStringMapping::MinFollowDistance, minCameraFollowDistance, float)
	IMPL_SETTER(ConfigStringMapping::MinCameraFollowRate, minCameraFollowRate, float)
	IMPL_SETTER(ConfigStringMapping::MaxCameraFollowRate, maxCameraFollowRate, float)
	IMPL_SETTER(ConfigStringMapping::MaxSmoothingInterpDistance, zoomMaxSmoothingDistance, float)
	IMPL_SETTER(ConfigStringMapping::ZoomMul, zoomMul, float)

	IMPL_SETTER(ConfigStringMapping::SepZMaxInterpDistance, separateZMaxSmoothingDistance, float)
	IMPL_SETTER(ConfigStringMapping::SepZMinFollowRate, separateZMinFollowRate, float)
	IMPL_SETTER(ConfigStringMapping::SepZMaxFollowRate, separateZMaxFollowRate, float)

	IMPL_SETTER(ConfigStringMapping::StandingSideOffset, standingSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::StandingUpOffset, standingUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::StandingCombatSideOffset, standingCombatSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::StandingCombatUpOffset, standingCombatUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::WalkingSideOffset, walkingSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::WalkingUpOffset, walkingUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::WalkingCombatSideOffset, walkingCombatSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::WalkingCombatUpOffset, walkingCombatUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::RunningSideOffset, runningSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::RunningUpOffset, runningUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::RunningCombatSideOffset, runningCombatSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::RunningCombatUpOffset, runningCombatUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::SprintingSideOffest, sprintingSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SprintingUpOffset, sprintingUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::SprintingCombatSideOffset, sprintingCombatSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SprintingCombatUpOffset, sprintingCombatUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::SneakingSideOffset, sneakingSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SneakingUpOffset, sneakingUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::SneakingCombatSideOffset, sneakingCombatSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SneakingCombatUpOffset, sneakingCombatUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::SwimmingSideOffset, swimmingSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SwimmingUpOffset, swimmingUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::BowAimSideOffset, bowAimSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::BowAimUpOffset, bowAimUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::BowAimHorseSideOffset, bowAimHorseSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::BowAimHorseUpOffset, bowAimHorseUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::SittingSideOffset, sittingSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::SittingUpOffset, sittingUpOffset, float)

	IMPL_SETTER(ConfigStringMapping::HorseSideOffset, horsebackSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::HorseUpOffset, horsebackUpOffset, float)
	IMPL_SETTER(ConfigStringMapping::DragonSideOffset, dragonSideOffset, float)
	IMPL_SETTER(ConfigStringMapping::DragonUpOffset, dragonUpOffset, float)
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
}