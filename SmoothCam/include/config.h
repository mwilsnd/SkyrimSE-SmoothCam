#pragma once
#include "code_analysis.h"
SILENCE_CODE_ANALYSIS;
#   include "nlohmann/json.hpp"
RESTORE_CODE_ANALYSIS;

namespace Config {
	using json = nlohmann::json;
	constexpr auto MaxPresetSlots = 6;

	enum class ScalarMethods : uint8_t {
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

	enum class LoadStatus : uint8_t {
		OK,
		MISSING,
		FAILED
	};

	enum class OffsetGroupID : uint8_t {
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
		{ "LINEAR",					ScalarMethods::LINEAR },
		{ "QUADRATICEASEIN",		ScalarMethods::QUAD_IN },
		{ "QUADRATICEASEOUT",		ScalarMethods::QUAD_OUT },
		{ "QUADRATICEASEINOUT",		ScalarMethods::QUAD_INOUT },
		{ "CUBICEASEIN",			ScalarMethods::CUBIC_IN },
		{ "CUBICEASEOUT",			ScalarMethods::CUBIC_OUT },
		{ "CUBICEASEINOUT",			ScalarMethods::CUBIC_INOUT },
		{ "QUARTICEASEIN",			ScalarMethods::QUART_IN },
		{ "QUARTICEASEOUT",			ScalarMethods::QUART_OUT },
		{ "QUARTICEASEINOUT",		ScalarMethods::QUART_INOUT },
		{ "QUINTICEASEIN",			ScalarMethods::QUINT_IN },
		{ "QUINTICEASEOUT",			ScalarMethods::QUINT_OUT },
		{ "QUINTICEASEINOUT",		ScalarMethods::QUINT_INOUT },
		{ "SINEEASEIN",				ScalarMethods::SINE_IN },
		{ "SINEEASEOUT",			ScalarMethods::SINE_OUT },
		{ "SINEEASEINOUT",			ScalarMethods::SINE_INOUT },
		{ "CIRCULAREASEIN",			ScalarMethods::CIRC_IN },
		{ "CIRCULAREASEOUT",		ScalarMethods::CIRC_OUT },
		{ "CIRCULAREASEINOUT",		ScalarMethods::CIRC_INOUT },
		{ "EXPONENTIALEASEIN",		ScalarMethods::EXP_IN },
		{ "EXPONENTIALEASEOUT",		ScalarMethods::EXP_OUT },
		{ "EXPONENTIALEASEINOUT",	ScalarMethods::EXP_INOUT },
	});

	constexpr auto scalarMethodRevLookup = mapbox::eternal::map<ScalarMethods, mapbox::eternal::string>({
		{ ScalarMethods::LINEAR, 		"LINEAR" },
		{ ScalarMethods::QUAD_IN, 		"QUADRATICEASEIN" },
		{ ScalarMethods::QUAD_OUT, 		"QUADRATICEASEOUT" },
		{ ScalarMethods::QUAD_INOUT, 	"QUADRATICEASEINOUT" },
		{ ScalarMethods::CUBIC_IN, 		"CUBICEASEIN" },
		{ ScalarMethods::CUBIC_OUT, 	"CUBICEASEOUT" },
		{ ScalarMethods::CUBIC_INOUT, 	"CUBICEASEINOUT" },
		{ ScalarMethods::QUART_IN, 		"QUARTICEASEIN" },
		{ ScalarMethods::QUART_OUT, 	"QUARTICEASEOUT" },
		{ ScalarMethods::QUART_INOUT, 	"QUARTICEASEINOUT" },
		{ ScalarMethods::QUINT_IN, 		"QUINTICEASEIN" },
		{ ScalarMethods::QUINT_OUT, 	"QUINTICEASEOUT" },
		{ ScalarMethods::QUINT_INOUT, 	"QUINTICEASEINOUT" },
		{ ScalarMethods::SINE_IN, 		"SINEEASEIN" },
		{ ScalarMethods::SINE_OUT, 		"SINEEASEOUT" },
		{ ScalarMethods::SINE_INOUT, 	"SINEEASEINOUT" },
		{ ScalarMethods::CIRC_IN, 		"CIRCULAREASEIN" },
		{ ScalarMethods::CIRC_OUT, 		"CIRCULAREASEOUT" },
		{ ScalarMethods::CIRC_INOUT, 	"CIRCULAREASEINOUT" },
		{ ScalarMethods::EXP_IN, 		"EXPONENTIALEASEIN" },
		{ ScalarMethods::EXP_OUT, 		"EXPONENTIALEASEOUT" },
		{ ScalarMethods::EXP_INOUT, 	"EXPONENTIALEASEINOUT" },
	});

	constexpr auto crosshairTypeLookup = mapbox::eternal::hash_map<mapbox::eternal::string, CrosshairType>({
		{ "SKYRIM", CrosshairType::Skyrim },
		{ "DOT", CrosshairType::Dot }
	});

	constexpr auto crosshairTypeRevLookup = mapbox::eternal::map<CrosshairType, mapbox::eternal::string>({
		{ CrosshairType::Skyrim, "SKYRIM" },
		{ CrosshairType::Dot, "DOT" }
	});

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
		bool worldCrosshairDepthTest = false;
		CrosshairType worldCrosshairType = CrosshairType::Skyrim;
		float stealthMeterXOffset = 0.0f;
		float stealthMeterYOffset = 0.0f;
		bool offsetStealthMeter = false;
		bool alwaysOffsetStealthMeter = false;

		// Arrow prediction
		bool useArrowPrediction = false;
		bool drawArrowArc = false;
		Color arrowArcColor = Color(255.0f, 255.0f, 255.0f, 127.0f);
		float maxArrowPredictionRange = 10000.0f;

		// Misc
		bool disableDeltaTime = false;
		int nextPresetKey = -1;
		int shoulderSwapKey = -1;
		bool swapXClamping = true;
		bool modDisabled = false;
		int modToggleKey = -1;
		float customZOffset = 0.0f;
		int applyZOffsetKey = -1;
		bool zOffsetActive = false; // @NOSAVE
		bool enableCrashDumps = false;
		
		// Compat
		bool compatIC = false;
		bool compatIFPV = false;
		bool compatAGO = false;
		bool compatACC = false;

		// Primary interpolation
		bool enableInterp = true;
		ScalarMethods currentScalar = ScalarMethods::SINE_IN;
		float minCameraFollowDistance = 64.0f;
		float minCameraFollowRate = 0.25f;
		float maxCameraFollowRate = 0.66f;
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
	LoadStatus LoadPresetName(int slot, eastl::string& name);
	// Returns the name of the saved preset or "Slot <N>" if no preset is found
	BSFixedString GetPresetSlotName(int slot);
	// Get the file path for the given preset slot
	eastl::wstring GetPresetPath(int slot);

	// Load the list of bones for the camera to follow
	void LoadBonePriorities();
	using BoneList = eastl::vector<BSFixedString>;
	// Get the follow bone list
	BoneList& GetBonePriorities() noexcept;

#ifdef DEVELOPER
	void LoadEyeBonePriorities();
	// And the bone list for firstperson
	BoneList& GetEyeBonePriorities() noexcept;
#endif
}