#pragma once
#include "nlohmann/json.hpp"

#define CREATE_JSON_VALUE(obj, member) {#member, obj.member}
#define VALUE_FROM_JSON(obj, member)	\
{										\
	const auto def = obj.##member##;	\
	obj.member = j.value(#member, def);	\
}

namespace Config {
	using json = nlohmann::json;
	constexpr auto MaxPresetSlots = 6;

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

	enum class LoadStatus {
		OK,
		MISSING,
		FAILED
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

	typedef struct offsetGroup {
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
	} OffsetGroup;
	void to_json(json& j, const OffsetGroup& obj);
	void from_json(const json& j, OffsetGroup& obj);

	typedef struct parsedConfig {
		// Crosshair
		bool enable3DCrosshair = true;
		bool alwaysUse3DCrosshair = false;
		bool hideNonCombatCrosshair = false;
		bool hideCrosshairMeleeCombat = false;
		float crosshairNPCHitGrowSize = 16.0f;
		float crosshairMinDistSize = 16.0f;
		float crosshairMaxDistSize = 24.0f;

		// Misc
		bool disableDeltaTime = false;
		
		// Comapt
		bool disableDuringDialog = false;
		bool comaptIC_FirstPersonHorse = true;
		bool comaptIC_FirstPersonDragon = true;
		bool compatIC_FirstPersonSitting = true;

		// Primary interpolation
		bool enableInterp = true;
		ScalarMethods currentScalar = ScalarMethods::SINE_IN;
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
		ScalarMethods separateZScalar = ScalarMethods::SINE_IN;
		float separateZMaxSmoothingDistance = 60.0f;
		float separateZMinFollowRate = 0.25f;
		float separateZMaxFollowRate = 0.4f;

		// Offset interpolation
		bool enableOffsetInterpolation = true;
		ScalarMethods offsetScalar = ScalarMethods::SINE_INOUT;
		float offsetInterpDurationSecs = 0.25f;

		// Zoom interpolation
		bool enableZoomInterpolation = true;
		ScalarMethods zoomScalar = ScalarMethods::LINEAR;
		float zoomInterpDurationSecs = 0.1f;

		// Distance clamping
		bool cameraDistanceClampXEnable = true;
		float cameraDistanceClampXMin = -75.0f;
		float cameraDistanceClampXMax = 35.0f;
		bool cameraDistanceClampYEnable = true;
		float cameraDistanceClampYMin = -100.0f;
		float cameraDistanceClampYMax = 20.0f;
		bool cameraDistanceClampZEnable = true;
		float cameraDistanceClampZMin = -60.0f;
		float cameraDistanceClampZMax = 60.0f;

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
	} UserConfig;
	void to_json(json& j, const UserConfig& obj);
	void from_json(const json& j, UserConfig& obj);

	typedef struct {
		std::string name;
		UserConfig config;
	} Preset;
	void to_json(json& j, const Preset& obj);
	void from_json(const json& j, Preset& obj);

	void ReadConfigFile();
	void SaveCurrentConfig();
	UserConfig* GetCurrentConfig() noexcept;

	// Returns "" if ok, otherwise has an error message
	BSFixedString SaveConfigAsPreset(int slot, const BSFixedString& name);
	// Returns true if ok, otherwise does nothing
	bool LoadPreset(int slot);
	// Returns true if ok, otherwise does nothing
	LoadStatus LoadPresetName(int slot, std::string& name);
	// Returns the name of the saved preset or "Slot <N>" if no preset is found
	BSFixedString GetPresetSlotName(int slot);
	// Get the file path for the given preset slot
	std::wstring GetPresetPath(int slot);
}