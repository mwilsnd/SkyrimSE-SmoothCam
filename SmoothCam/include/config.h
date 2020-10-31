#pragma once
#include "code_analysis.h"
SILENCE_CODE_ANALYSIS;
#   include "nlohmann/json.hpp"
RESTORE_CODE_ANALYSIS;

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

	enum class CrosshairType : uint8_t {
		None,
		Skyrim,
		Dot
	};

	enum class LoadStatus {
		OK,
		MISSING,
		FAILED
	};

	enum class OffsetGroupID {
		Standing,
		Walking,
		Running,
		Sprinting,
		Sneaking,
		Swimming,
		BowAim,
		Sitting,
		Horseback,
		Dragon,
		VampireLord,
		Werewolf,
		Unknown,
		MAX_OFS
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

	constexpr auto crosshairTypeLookup = mapbox::eternal::hash_map<mapbox::eternal::string, CrosshairType>({
		{ "Skyrim", CrosshairType::Skyrim },
		{ "Dot", CrosshairType::Dot }
	});

	constexpr auto crosshairTypeRevLookup = mapbox::eternal::map<CrosshairType, mapbox::eternal::string>({
		{ CrosshairType::Skyrim, "Skyrim" },
		{ CrosshairType::Dot, "Dot" }
	});

	typedef struct gameConf {
		float f3PArrowTiltUpAngle = 2.5f;
		float f3PBoltTiltUpAngle = 2.5f;
		float fNearDistance = 15.0f;
		float fMinCurrentZoom = -0.200000003;
	} GameConfig;

	typedef struct offsetGroup {
		float sideOffset = 25.0;
		float upOffset = 0.0;
		float zoomOffset = 0.0f;
		float fovOffset = 0.0f;

		float combatRangedSideOffset = 25.0f;
		float combatRangedUpOffset = 0.0f;
		float combatRangedZoomOffset = 0.0f;
		float combatRangedFOVOffset = 0.0f;

		float combatMagicSideOffset = 25.0f;
		float combatMagicUpOffset = 0.0f;
		float combatMagicZoomOffset = 0.0f;
		float combatMagicFOVOffset = 0.0f;

		float combatMeleeSideOffset = 25.0f;
		float combatMeleeUpOffset = 0.0f;
		float combatMeleeZoomOffset = 0.0f;
		float combatMeleeFOVOffset = 0.0f;

		float horseSideOffset = 25.0f;
		float horseUpOffset = 0.0f;
		float horseZoomOffset = 0.0f;
		float horseFOVOffset = 0.0f;

		bool interp = true;
		bool interpRangedCombat = true;
		bool interpMagicCombat = true;
		bool interpMeleeCombat = true;
		bool interpHorseback = true;

		OffsetGroupID id = OffsetGroupID::Unknown;
	} OffsetGroup;
	void to_json(json& j, const OffsetGroup& obj);
	void from_json(const json& j, OffsetGroup& obj);

	typedef struct configColor {
		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;
		float a = 0.0f;

		configColor() {};
		configColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
	} Color;

	typedef struct parsedConfig {
		// Crosshair
		bool use3DBowAimCrosshair = true;
		bool use3DMagicCrosshair = true;
		bool hideNonCombatCrosshair = false;
		bool hideCrosshairMeleeCombat = false;
		bool enableCrosshairSizeManip = false;
		float crosshairNPCHitGrowSize = 16.0f;
		float crosshairMinDistSize = 16.0f;
		float crosshairMaxDistSize = 24.0f;
		bool useWorldCrosshair = false;
		bool worldCrosshairDepthTest = true;
		CrosshairType worldCrosshairType = CrosshairType::Skyrim;

		// Arrow prediction
		bool useArrowPrediction = false;
		bool drawArrowArc = false;
		Color arrowArcColor = Color(255.0f, 255.0f, 255.0f, 127.0f);
		float maxArrowPredictionRange = 10000.0f;

		// Misc
		bool disableDeltaTime = false;
		int shoulderSwapKey = -1;
		bool swapXClamping = true;
		
		// Compat
		bool compatIC = false;
		bool compatIFPV = false;
		bool compatAGO = false;
		bool compatACC = false;

		// Primary interpolation
		bool enableInterp = true;
		ScalarMethods currentScalar = ScalarMethods::SINE_IN;
		float minCameraFollowDistance = 64.0f;
		float minCameraFollowRate = 0.15f;
		float maxCameraFollowRate = 0.4f;
		float zoomMul = 500.0f;
		float zoomMaxSmoothingDistance = 650.0f;

		// Separate local space interpolation
		bool separateLocalInterp = true;
		ScalarMethods separateLocalScalar = ScalarMethods::EXP_IN;
		float localScalarRate = 1.0f;

		// Separate Z
		bool separateZInterp = true;
		ScalarMethods separateZScalar = ScalarMethods::SINE_IN;
		float separateZMaxSmoothingDistance = 60.0f;
		float separateZMinFollowRate = 0.2f;
		float separateZMaxFollowRate = 0.6f;

		// Offset interpolation
		bool enableOffsetInterpolation = true;
		ScalarMethods offsetScalar = ScalarMethods::SINE_INOUT;
		float offsetInterpDurationSecs = 0.25f;

		// Zoom interpolation
		bool enableZoomInterpolation = true;
		ScalarMethods zoomScalar = ScalarMethods::LINEAR;
		float zoomInterpDurationSecs = 0.1f;

		// FOV interpolation
		bool enableFOVInterpolation = true;
		ScalarMethods fovScalar = ScalarMethods::LINEAR;
		float fovInterpDurationSecs = 0.1f;

		// Distance clamping
		bool cameraDistanceClampXEnable = true;
		float cameraDistanceClampXMin = -75.0f;
		float cameraDistanceClampXMax = 35.0f;
		bool cameraDistanceClampYEnable = true;
		float cameraDistanceClampYMin = -100.0f;
		float cameraDistanceClampYMax = 10.0f;
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
		OffsetGroup vampireLord;
		OffsetGroup werewolf;
	} UserConfig;
	void to_json(json& j, const UserConfig& obj);
	void from_json(const json& j, UserConfig& obj);

	typedef struct {
		std::string name;
		UserConfig config;
	} Preset;
	void to_json(json& j, const Preset& obj);
	void from_json(const json& j, Preset& obj);

	void to_json(json& j, const Color& obj);
	void from_json(const json& j, Color& obj);

	void ReadConfigFile();
	void SaveCurrentConfig();
	UserConfig* GetCurrentConfig() noexcept;
	void ResetConfig();

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

	// Load the list of bones for the camera to follow
	void LoadBonePriorities();
	using BoneList = std::vector<BSFixedString>;
	// Get the follow bone list
	BoneList& GetBonePriorities() noexcept;

	const GameConfig* const GetGameConfig();
}