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
		USER_DEFINED,
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
		UserDefined,
		Vanity,
		Unknown,
		MAX_OFS
	};

	enum class DialogueMode : uint8_t {
		Disabled,
		Skyrim,
		Oblivion,
		FaceToFace,
		MAX
	};

	typedef struct dialogueOblivion {
		float fovOffset = -30.0f;
		float zoomInDuration = 1.0f;
		float zoomOutDuration = 1.0f;
		bool runInFirstPerson = true;
	} DialogueOblivion;
	void to_json(json& j, const DialogueOblivion& obj);
	void from_json(const json& j, DialogueOblivion& obj);

	typedef struct dialogueFaceToFace {
		float sideOffset = 30.0f;
		float upOffset = 0.0f;
		float zoomOffset = 0.0f;

		float rotationDuration = 0.25f;
		float zoomInDuration = 1.0f;
		float zoomOutDuration = 1.0f;

		bool faceToFaceNoSwitch = false;
		bool forceThirdPerson = false;
	} DialogueFaceToFace;
	void to_json(json& j, const DialogueFaceToFace& obj);
	void from_json(const json& j, DialogueFaceToFace& obj);

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

	constexpr auto dialogueTypeLookup = mapbox::eternal::hash_map<mapbox::eternal::string, DialogueMode>({
		{ "DISABLED", DialogueMode::Disabled },
		{ "SKYRIM", DialogueMode::Skyrim },
		{ "OBLIVION", DialogueMode::Oblivion },
		{ "FACE TO FACE", DialogueMode::FaceToFace }
	});

	constexpr auto dialogueTypeRevLookup = mapbox::eternal::map<DialogueMode, mapbox::eternal::string>({
		{ DialogueMode::Disabled, "DISABLED" },
		{ DialogueMode::Skyrim, "SKYRIM" },
		{ DialogueMode::Oblivion, "OBLIVION" },
		{ DialogueMode::FaceToFace, "FACE TO FACE" }
	});

	typedef struct OffsetGroupScalar {
		bool overrideInterp = false;
		ScalarMethods currentScalar = ScalarMethods::SINE_IN;
		float minCameraFollowRate = 0.25f;
		float maxCameraFollowRate = 0.66f;
		float zoomMaxSmoothingDistance = 650.0f;

		bool overrideLocalInterp = false;
		ScalarMethods separateLocalScalar = ScalarMethods::EXP_IN;
		float localMinFollowRate = 0.7f;
		float localMaxFollowRate = 0.98f;
		float localMaxSmoothingDistance = 60.0f;
	} OffsetGroupScalar;
	void to_json(json& j, const OffsetGroupScalar& obj);
	void from_json(const json& j, OffsetGroupScalar& obj);

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

		OffsetGroupScalar interpConf;
		OffsetGroupScalar interpRangedConf;
		OffsetGroupScalar interpMagicConf;
		OffsetGroupScalar interpMeleeConf;
		OffsetGroupScalar interpHorsebackConf;

		OffsetGroupID id = OffsetGroupID::Unknown;
	} OffsetGroup;
	void to_json(json& j, const OffsetGroup& obj);
	void from_json(const json& j, OffsetGroup& obj);

	typedef struct configColor {
		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;
		float a = 0.0f;

		configColor() noexcept {};
		configColor(float r, float g, float b, float a) noexcept : r(r), g(g), b(b), a(a) {}
	} Color;
	void to_json(json& j, const Color& obj);
	void from_json(const json& j, Color& obj);

	typedef struct parsedConfig {
		// Crosshair
		bool use3DBowAimCrosshair = true;
		bool use3DMagicCrosshair = true;
		bool hideNonCombatCrosshair = false;
		bool hideCrosshairMeleeCombat = false;
		bool enableCrosshairSizeManip = true;
		float crosshairNPCHitGrowSize = 16.0f;
		float crosshairMinDistSize = 16.0f;
		float crosshairMaxDistSize = 24.0f;
		bool useWorldCrosshair = true;
		bool worldCrosshairDepthTest = false;
		CrosshairType worldCrosshairType = CrosshairType::Skyrim;
		float stealthMeterXOffset = 0.0f;
		float stealthMeterYOffset = 250.0f;
		bool offsetStealthMeter = false;
		bool alwaysOffsetStealthMeter = false;

		// Arrow prediction
		bool useArrowPrediction = true;
		bool drawArrowArc = true;
		Color arrowArcColor = Color(255.0f, 255.0f, 255.0f, 200.0f);
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
		int toggleUserDefinedOffsetGroupKey = -1;
		bool userDefinedOffsetActive = false; // @NOSAVE

		// Pitch zoom
		bool enablePitchZoom = false;
		bool pitchZoomAfterInterp = false;
		float pitchZoomMaxAngle = 90.0f;
		float pitchZoomMax = 100.0f;
		ScalarMethods pitchZoomMethod = ScalarMethods::SINE_OUT;

		// Primary interpolation
		bool enableInterp = true;
		ScalarMethods currentScalar = ScalarMethods::SINE_IN;
		float minCameraFollowDistance = 80.0f;
		float minCameraFollowRate = 0.25f;
		float maxCameraFollowRate = 0.66f;
		float zoomMul = 500.0f;
		float zoomMaxSmoothingDistance = 650.0f;

		// Separate local space interpolation
		bool separateLocalInterp = true;
		ScalarMethods separateLocalScalar = ScalarMethods::EXP_IN;
		float localMinFollowRate = 0.7f;
		float localMaxFollowRate = 0.98f;
		float localMaxSmoothingDistance = 60.0f;

		// Separate Z
		bool separateZInterp = true;
		ScalarMethods separateZScalar = ScalarMethods::SINE_IN;
		float separateZMaxSmoothingDistance = 55.0f;
		float separateZMinFollowRate = 0.4f;
		float separateZMaxFollowRate = 1.0f;

		// Offset interpolation
		bool enableOffsetInterpolation = true;
		ScalarMethods offsetScalar = ScalarMethods::SINE_INOUT;
		float offsetInterpDurationSecs = 1.0f;

		// Zoom interpolation
		bool enableZoomInterpolation = true;
		ScalarMethods zoomScalar = ScalarMethods::LINEAR;
		float zoomInterpDurationSecs = 0.06f;

		// FOV interpolation
		bool enableFOVInterpolation = true;
		ScalarMethods fovScalar = ScalarMethods::QUAD_INOUT;
		float fovInterpDurationSecs = 1.0f;

		// Interp override smoother
		float globalInterpDisableSmoothing = 2.0f;
		ScalarMethods globalInterpDisableMehtod = ScalarMethods::LINEAR;
		float globalInterpOverrideSmoothing = 1.5f;
		ScalarMethods globalInterpOverrideMethod = ScalarMethods::LINEAR;
		float localInterpOverrideSmoothing = 2.0f;
		ScalarMethods localInterpOverrideMethod = ScalarMethods::LINEAR;

		// Distance clamping
		bool cameraDistanceClampXEnable = true;
		float cameraDistanceClampXMin = -75.0f;
		float cameraDistanceClampXMax = 35.0f;
		bool cameraDistanceClampYEnable = true;
		float cameraDistanceClampYMin = -150.0f;
		float cameraDistanceClampYMax = 0.0f;
		bool cameraDistanceClampZEnable = true;
		float cameraDistanceClampZMin = -60.0f;
		float cameraDistanceClampZMax = 60.0f;

		// Dialogue
		DialogueMode dialogueMode = DialogueMode::Skyrim;
		DialogueOblivion oblivionDialogue;
		DialogueFaceToFace faceToFaceDialogue;

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
		OffsetGroup vanity;
		OffsetGroup userDefined;
	} UserConfig;
	void to_json(json& j, const UserConfig& obj);
	void from_json(const json& j, UserConfig& obj);

	typedef struct {
		std::string name;
		UserConfig config;
	} Preset;
	void to_json(json& j, const Preset& obj);
	void from_json(const json& j, Preset& obj);

	const UserConfig& GetDefaultConfig() noexcept;

	void ReadConfigFile();
	void SaveCurrentConfig();
	UserConfig* GetCurrentConfig() noexcept;
	void ResetConfig();

	// Returns "" if ok, otherwise has an error message
	RE::BSFixedString SaveConfigAsPreset(int slot, const RE::BSFixedString& name);
	// Returns true if ok, otherwise does nothing
	bool LoadPreset(int slot);
	// Returns true if ok, otherwise does nothing
	LoadStatus LoadPresetName(int slot, eastl::string& name);
	// Returns the name of the saved preset or "Slot <N>" if no preset is found
	RE::BSFixedString GetPresetSlotName(int slot);
	// Get the file path for the given preset slot
	eastl::wstring GetPresetPath(int slot);

	// Load the list of bones for the camera to follow
	using BoneList = eastl::vector<RE::BSFixedString>;
	bool LoadBoneList(const std::wstring_view&& searchName, BoneList& outBones);
	
	void LoadBonePriorities();
	// Get the follow bone list
	BoneList& GetBonePriorities() noexcept;

	void LoadFocusBonePriorities();
	BoneList& GetFocusBonePriorities() noexcept;

#ifdef DEVELOPER
	void LoadEyeBonePriorities();
	// And the bone list for firstperson
	BoneList& GetEyeBonePriorities() noexcept;
#endif
}