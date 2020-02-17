#pragma once
#include "pch.h"

namespace Config {
	enum class ScalarMethods {
		LINEAR,
		QUAD_IN, QUAD_OUT, QUAD_INOUT,
		CUBIC_IN, CUBIC_OUT, CUBIC_INOUT,
		QUART_IN, QUART_OUT, QUART_INOUT,
		QUINT_IN, QUINT_OUT, QUINT_INOUT,
		SINE_IN, SINE_OUT, SINE_INOUT,
		CIRC_IN, CIRC_OUT, CIRC_INOUT,
		EXP_IN, EXP_OUT, EXP_INOUT,
	};

	constexpr auto scalarMethods = mapbox::eternal::hash_map<mapbox::eternal::string, ScalarMethods>({
		{ "linear",					ScalarMethods::LINEAR },
		{ "quadraticEaseIn",		ScalarMethods::QUAD_IN },
		{ "quadraticEaseOut",		ScalarMethods::QUAD_OUT },
		{ "quadraticEaseInOut",		ScalarMethods::QUAD_INOUT },
		{ "cubicEaseIn",			ScalarMethods::CUBIC_IN },
		{ "cubicEaseOut",			ScalarMethods::CUBIC_OUT },
		{ "cubicEaseInOut",			ScalarMethods::CUBIC_INOUT },
		{ "quarticEaseIn",			ScalarMethods::QUART_IN },
		{ "quarticEaseOut",			ScalarMethods::QUART_OUT },
		{ "quarticEaseInOut",		ScalarMethods::QUART_INOUT },
		{ "quinticEaseIn",			ScalarMethods::QUINT_IN },
		{ "quinticEaseOut",			ScalarMethods::QUINT_OUT },
		{ "quinticEaseInOut",		ScalarMethods::QUINT_INOUT },
		{ "sineEaseIn",				ScalarMethods::SINE_IN },
		{ "sineEaseOut",			ScalarMethods::SINE_OUT },
		{ "sineEaseInOut",			ScalarMethods::SINE_INOUT },
		{ "circularEaseIn",			ScalarMethods::CIRC_IN },
		{ "circularEaseOut",		ScalarMethods::CIRC_OUT },
		{ "circularEaseInOut",		ScalarMethods::CIRC_INOUT },
		{ "exponentialEaseIn",		ScalarMethods::EXP_IN },
		{ "exponentialEaseOut",		ScalarMethods::EXP_OUT },
		{ "exponentialEaseInOut",	ScalarMethods::EXP_INOUT },
	});

	constexpr auto scalarMethodRevLookup = mapbox::eternal::map<ScalarMethods, mapbox::eternal::string>({
		{ ScalarMethods::LINEAR, 		"linear" },
		{ ScalarMethods::QUAD_IN, 		"quadraticEaseIn" },
		{ ScalarMethods::QUAD_OUT, 		"quadraticEaseOut" },
		{ ScalarMethods::QUAD_INOUT, 	"quadraticEaseInOut" },
		{ ScalarMethods::CUBIC_IN, 		"cubicEaseIn" },
		{ ScalarMethods::CUBIC_OUT, 	"cubicEaseOut" },
		{ ScalarMethods::CUBIC_INOUT, 	"cubicEaseInOut" },
		{ ScalarMethods::QUART_IN, 		"quarticEaseIn" },
		{ ScalarMethods::QUART_OUT, 	"quarticEaseOut" },
		{ ScalarMethods::QUART_INOUT, 	"quarticEaseInOut" },
		{ ScalarMethods::QUINT_IN, 		"quinticEaseIn" },
		{ ScalarMethods::QUINT_OUT, 	"quinticEaseOut" },
		{ ScalarMethods::QUINT_INOUT, 	"quinticEaseInOut" },
		{ ScalarMethods::SINE_IN, 		"sineEaseIn" },
		{ ScalarMethods::SINE_OUT, 		"sineEaseOut" },
		{ ScalarMethods::SINE_INOUT, 	"sineEaseInOut" },
		{ ScalarMethods::CIRC_IN, 		"circularEaseIn" },
		{ ScalarMethods::CIRC_OUT, 		"circularEaseOut" },
		{ ScalarMethods::CIRC_INOUT, 	"circularEaseInOut" },
		{ ScalarMethods::EXP_IN, 		"exponentialEaseIn" },
		{ ScalarMethods::EXP_OUT, 		"exponentialEaseOut" },
		{ ScalarMethods::EXP_INOUT, 	"exponentialEaseInOut" },
	});

	typedef struct parsedConfig {
		bool enableInterp = true;
		bool enable3DCrosshair = true;

		ScalarMethods currentScalar = ScalarMethods::CIRC_IN;

		// Comapt
		bool comaptIC_FirstPersonHorse = true;
		bool comaptIC_FirstPersonDragon = true;
		bool compatIC_FirstPersonSitting = true;

		// mins + maxs
		float minCameraFollowDistance = 64.0f;
		float minCameraFollowRate = 0.4f;
		float maxCameraFollowRate = 0.8f;

		float zoomMul = 500.0f;
		float zoomMaxSmoothingDistance = 500.0f;

		// Separate Z
		bool separateZInterp = false;
		float separateZMaxSmoothingDistance = 60.0f;
		float separateZMinFollowRate = 0.5f;
		float separateZMaxFollowRate = 1.0f;

		ScalarMethods separateZScalar = ScalarMethods::CIRC_IN;

		// Per state positions
		float standingSideOffset = 25.0f;
		float standingUpOffset = 0.0f;

		float standingCombatSideOffset = 25.0f;
		float standingCombatUpOffset = 0.0f;

		float walkingSideOffset = 25.0f;
		float walkingUpOffset = 0.0f;

		float walkingCombatSideOffset = 25.0f;
		float walkingCombatUpOffset = 0.0f;

		float runningSideOffset = 25.0f;
		float runningUpOffset = 0.0f;

		float runningCombatSideOffset = 25.0f;
		float runningCombatUpOffset = 0.0f;

		float sprintingSideOffset = 25.0f;
		float sprintingUpOffset = 0.0f;

		float sprintingCombatSideOffset = 25.0f;
		float sprintingCombatUpOffset = 0.0f;

		float sneakingSideOffset = 25.0f;
		float sneakingUpOffset = -5.0f;

		float sneakingCombatSideOffset = 25.0f;
		float sneakingCombatUpOffset = 0.0f;

		float swimmingSideOffset = 25.0f;
		float swimmingUpOffset = 0.0f;

		float bowAimSideOffset = 35.0f;
		float bowAimUpOffset = 0.0f;

		float bowAimHorseSideOffset = 40.0f;
		float bowAimHorseUpOffset = 0.0f;

		float sittingSideOffset = 18.0f;
		float sittingUpOffset = 0.0f;

		float horsebackSideOffset = 18.0f;
		float horsebackUpOffset = 0.0f;

		float dragonSideOffset = 18.0f;
		float dragonUpOffset = 0.0f;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(enableInterp),
				CEREAL_NVP(enable3DCrosshair),
				CEREAL_NVP(currentScalar),
				CEREAL_NVP(comaptIC_FirstPersonHorse),
				CEREAL_NVP(comaptIC_FirstPersonDragon),
				CEREAL_NVP(compatIC_FirstPersonSitting),
				CEREAL_NVP(minCameraFollowDistance),
				CEREAL_NVP(minCameraFollowRate),
				CEREAL_NVP(maxCameraFollowRate),
				CEREAL_NVP(zoomMul),
				CEREAL_NVP(zoomMaxSmoothingDistance),
				CEREAL_NVP(standingSideOffset),
				CEREAL_NVP(standingUpOffset),
				CEREAL_NVP(standingCombatSideOffset),
				CEREAL_NVP(standingCombatUpOffset),
				CEREAL_NVP(walkingSideOffset),
				CEREAL_NVP(walkingUpOffset),
				CEREAL_NVP(walkingCombatSideOffset),
				CEREAL_NVP(walkingCombatUpOffset),
				CEREAL_NVP(runningSideOffset),
				CEREAL_NVP(runningUpOffset),
				CEREAL_NVP(runningCombatSideOffset),
				CEREAL_NVP(runningCombatUpOffset),
				CEREAL_NVP(sprintingSideOffset),
				CEREAL_NVP(sprintingUpOffset),
				CEREAL_NVP(sprintingCombatSideOffset),
				CEREAL_NVP(sprintingCombatUpOffset),
				CEREAL_NVP(sneakingSideOffset),
				CEREAL_NVP(sneakingUpOffset),
				CEREAL_NVP(sneakingCombatSideOffset),
				CEREAL_NVP(sneakingCombatUpOffset),
				CEREAL_NVP(swimmingSideOffset),
				CEREAL_NVP(swimmingUpOffset),
				CEREAL_NVP(bowAimSideOffset),
				CEREAL_NVP(bowAimUpOffset),
				CEREAL_NVP(bowAimHorseSideOffset),
				CEREAL_NVP(bowAimHorseUpOffset),
				CEREAL_NVP(sittingSideOffset),
				CEREAL_NVP(sittingUpOffset),
				CEREAL_NVP(horsebackSideOffset),
				CEREAL_NVP(horsebackUpOffset),
				CEREAL_NVP(dragonSideOffset),
				CEREAL_NVP(dragonUpOffset)
			);
		}
	} UserConfig;

	void ReadConfigFile();
	void SaveCurrentConfig();
	UserConfig* GetCurrentConfig() noexcept;
}