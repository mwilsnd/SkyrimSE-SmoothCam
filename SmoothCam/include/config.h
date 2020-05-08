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

	typedef struct {
		float sideOffset = 25.0;
		float upOffset = 0.0;

		float combatRangedSideOffset = 25.0f;
		float combatRangedUpOffset = 0.0f;

		float combatMagicSideOffset = 25.0f;
		float combatMagicUpOffset = 0.0f;

		float combatMeleeSideOffset = 25.0f;
		float combatMeleeUpOffset = 0.0f;

		float horseSideOffset = 25.0f;
		float horseUpOffset = 0.0f;

		bool interp = true;
		bool interpRangedCombat = true;
		bool interpMagicCombat = true;
		bool interpMeleeCombat = true;
		bool interpHorseback = true;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(sideOffset),
				CEREAL_NVP(upOffset),
				CEREAL_NVP(combatRangedSideOffset),
				CEREAL_NVP(combatRangedUpOffset),
				CEREAL_NVP(combatMagicSideOffset),
				CEREAL_NVP(combatMagicUpOffset),
				CEREAL_NVP(combatMeleeSideOffset),
				CEREAL_NVP(combatMeleeUpOffset),
				CEREAL_NVP(horseSideOffset),
				CEREAL_NVP(horseUpOffset),
				CEREAL_NVP(interp),
				CEREAL_NVP(interpRangedCombat),
				CEREAL_NVP(interpMagicCombat),
				CEREAL_NVP(interpMeleeCombat),
				CEREAL_NVP(interpHorseback)
			);
		}
	} OffsetGroup;

	typedef struct parsedConfig {
		// Crosshair
		bool enable3DCrosshair = true;
		bool hideNonCombatCrosshair = true;
		bool hideCrosshairMeleeCombat = true;

		// Misc
		bool disableDeltaTime = false;
		
		// Comapt
		bool disableDuringDialog = false;
		bool comaptIC_FirstPersonHorse = true;
		bool comaptIC_FirstPersonDragon = true;
		bool compatIC_FirstPersonSitting = true;
		bool patchWorldToScreenMatrix = false;

		// Primary interpolation
		bool enableInterp = true;
		ScalarMethods currentScalar = ScalarMethods::CIRC_IN;
		float minCameraFollowDistance = 64.0f;
		float minCameraFollowRate = 0.2f;
		float maxCameraFollowRate = 0.5f;
		float zoomMul = 500.0f;
		float zoomMaxSmoothingDistance = 650.0f;

		// Separate local space interpolation
		bool separateLocalInterp = false;
		ScalarMethods separateLocalScalar = ScalarMethods::CIRC_IN;
		float localScalarRate = 1.0f;

		// Separate Z
		bool separateZInterp = false;
		ScalarMethods separateZScalar = ScalarMethods::CIRC_IN;
		float separateZMaxSmoothingDistance = 80.0f;
		float separateZMinFollowRate = 0.15f;
		float separateZMaxFollowRate = 0.3f;

		// Distance clamping
		bool cameraDistanceClampXEnable = true;
		float cameraDistanceClampXMin = -75.0f;
		float cameraDistanceClampXMax = 75.0f;
		bool cameraDistanceClampYEnable = true;
		float cameraDistanceClampYMin = -100.0f;
		float cameraDistanceClampYMax = 100.0f;
		bool cameraDistanceClampZEnable = true;
		float cameraDistanceClampZMin = -80.0f;
		float cameraDistanceClampZMax = 80.0f;

		// Per state positions
		OffsetGroup standing;
		OffsetGroup walking;
		OffsetGroup running;
		OffsetGroup sprinting;
		OffsetGroup sneaking;
		OffsetGroup swimming;
		OffsetGroup bowAim;
		OffsetGroup sitting;
		OffsetGroup horseback;
		OffsetGroup dragon; // @TODO

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(enableInterp),
				CEREAL_NVP(enable3DCrosshair),
				CEREAL_NVP(hideNonCombatCrosshair),
				CEREAL_NVP(hideCrosshairMeleeCombat),
				CEREAL_NVP(disableDeltaTime),
				CEREAL_NVP(disableDuringDialog),
				CEREAL_NVP(currentScalar),
				CEREAL_NVP(comaptIC_FirstPersonHorse),
				CEREAL_NVP(comaptIC_FirstPersonDragon),
				CEREAL_NVP(compatIC_FirstPersonSitting),
				CEREAL_NVP(patchWorldToScreenMatrix),
				CEREAL_NVP(minCameraFollowDistance),
				CEREAL_NVP(minCameraFollowRate),
				CEREAL_NVP(maxCameraFollowRate),
				CEREAL_NVP(zoomMul),
				CEREAL_NVP(zoomMaxSmoothingDistance),
				CEREAL_NVP(separateLocalInterp),
				CEREAL_NVP(separateLocalScalar),
				CEREAL_NVP(localScalarRate),
				CEREAL_NVP(cameraDistanceClampXEnable),
				CEREAL_NVP(cameraDistanceClampXMin),
				CEREAL_NVP(cameraDistanceClampXMax),
				CEREAL_NVP(cameraDistanceClampYEnable),
				CEREAL_NVP(cameraDistanceClampYMin),
				CEREAL_NVP(cameraDistanceClampYMax),
				CEREAL_NVP(cameraDistanceClampZEnable),
				CEREAL_NVP(cameraDistanceClampZMin),
				CEREAL_NVP(cameraDistanceClampZMax),
				CEREAL_NVP(separateZInterp),
				CEREAL_NVP(separateZMaxSmoothingDistance),
				CEREAL_NVP(separateZMinFollowRate),
				CEREAL_NVP(separateZMaxFollowRate),
				CEREAL_NVP(separateZScalar),
				CEREAL_NVP(standing),
				CEREAL_NVP(walking),
				CEREAL_NVP(running),
				CEREAL_NVP(sprinting),
				CEREAL_NVP(sneaking),
				CEREAL_NVP(swimming),
				CEREAL_NVP(bowAim),
				CEREAL_NVP(sitting),
				CEREAL_NVP(horseback),
				CEREAL_NVP(dragon)
			);
		}
	} UserConfig;

	void ReadConfigFile();
	void SaveCurrentConfig();
	UserConfig* GetCurrentConfig() noexcept;
}