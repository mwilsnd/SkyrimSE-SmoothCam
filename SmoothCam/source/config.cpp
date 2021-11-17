#pragma warning(push)
#pragma warning(disable : 5103)

Config::UserConfig currentConfig;
#define CREATE_JSON_VALUE(obj, member) {#member, obj.member}
#define VALUE_FROM_JSON(obj, member)	\
{										\
	const auto def = obj.##member##;	\
	obj.member = j.value(#member, def);	\
}

void Config::to_json(json& j, const DialogueOblivion& obj) {
	j = json{
		CREATE_JSON_VALUE(obj, fovOffset),
		CREATE_JSON_VALUE(obj, zoomInDuration),
		CREATE_JSON_VALUE(obj, zoomOutDuration),
		CREATE_JSON_VALUE(obj, runInFirstPerson)
	};
}

void Config::from_json(const json& j, DialogueOblivion& obj) {
	VALUE_FROM_JSON(obj, fovOffset)
	VALUE_FROM_JSON(obj, zoomInDuration)
	VALUE_FROM_JSON(obj, zoomOutDuration)
	VALUE_FROM_JSON(obj, runInFirstPerson)
}

void Config::to_json(json& j, const DialogueFaceToFace& obj) {
	j = json{
		CREATE_JSON_VALUE(obj, sideOffset),
		CREATE_JSON_VALUE(obj, upOffset),
		CREATE_JSON_VALUE(obj, zoomOffset),
		CREATE_JSON_VALUE(obj, rotationDuration),
		CREATE_JSON_VALUE(obj, zoomInDuration),
		CREATE_JSON_VALUE(obj, zoomOutDuration),
		CREATE_JSON_VALUE(obj, faceToFaceNoSwitch),
		CREATE_JSON_VALUE(obj, forceThirdPerson)
	};
}

void Config::from_json(const json& j, DialogueFaceToFace& obj) {
	VALUE_FROM_JSON(obj, sideOffset)
	VALUE_FROM_JSON(obj, upOffset)
	VALUE_FROM_JSON(obj, zoomOffset)
	VALUE_FROM_JSON(obj, rotationDuration)
	VALUE_FROM_JSON(obj, zoomInDuration)
	VALUE_FROM_JSON(obj, zoomOutDuration)
	VALUE_FROM_JSON(obj, faceToFaceNoSwitch)
	VALUE_FROM_JSON(obj, forceThirdPerson)
}

void Config::to_json(json& j, const OffsetGroupScalar& obj) {
	j = json{
		CREATE_JSON_VALUE(obj, overrideInterp),
		CREATE_JSON_VALUE(obj, currentScalar),
		CREATE_JSON_VALUE(obj, minCameraFollowRate),
		CREATE_JSON_VALUE(obj, maxCameraFollowRate),
		CREATE_JSON_VALUE(obj, zoomMaxSmoothingDistance),
		CREATE_JSON_VALUE(obj, overrideLocalInterp),
		CREATE_JSON_VALUE(obj, separateLocalScalar),
		CREATE_JSON_VALUE(obj, localMinFollowRate),
		CREATE_JSON_VALUE(obj, localMaxFollowRate),
		CREATE_JSON_VALUE(obj, localMaxSmoothingDistance),
	};
}

void Config::from_json(const json& j, OffsetGroupScalar& obj) {
	VALUE_FROM_JSON(obj, overrideInterp)
	VALUE_FROM_JSON(obj, currentScalar)
	VALUE_FROM_JSON(obj, minCameraFollowRate)
	VALUE_FROM_JSON(obj, maxCameraFollowRate)
	VALUE_FROM_JSON(obj, zoomMaxSmoothingDistance)
	VALUE_FROM_JSON(obj, overrideLocalInterp)
	VALUE_FROM_JSON(obj, separateLocalScalar)
	VALUE_FROM_JSON(obj, localMinFollowRate)
	VALUE_FROM_JSON(obj, localMaxFollowRate)
	VALUE_FROM_JSON(obj, localMaxSmoothingDistance)
}

void Config::to_json(json& j, const OffsetGroup& obj) {
	j = json{
		CREATE_JSON_VALUE(obj, sideOffset),
		CREATE_JSON_VALUE(obj, upOffset),
		CREATE_JSON_VALUE(obj, zoomOffset),
		CREATE_JSON_VALUE(obj, fovOffset),

		CREATE_JSON_VALUE(obj, combatRangedSideOffset),
		CREATE_JSON_VALUE(obj, combatRangedUpOffset),
		CREATE_JSON_VALUE(obj, combatRangedZoomOffset),
		CREATE_JSON_VALUE(obj, combatRangedFOVOffset),

		CREATE_JSON_VALUE(obj, combatMagicSideOffset),
		CREATE_JSON_VALUE(obj, combatMagicUpOffset),
		CREATE_JSON_VALUE(obj, combatMagicZoomOffset),
		CREATE_JSON_VALUE(obj, combatMagicFOVOffset),

		CREATE_JSON_VALUE(obj, combatMeleeSideOffset),
		CREATE_JSON_VALUE(obj, combatMeleeUpOffset),
		CREATE_JSON_VALUE(obj, combatMeleeZoomOffset),
		CREATE_JSON_VALUE(obj, combatMeleeFOVOffset),

		CREATE_JSON_VALUE(obj, horseSideOffset),
		CREATE_JSON_VALUE(obj, horseUpOffset),
		CREATE_JSON_VALUE(obj, horseZoomOffset),
		CREATE_JSON_VALUE(obj, horseFOVOffset),

		CREATE_JSON_VALUE(obj, interp),
		CREATE_JSON_VALUE(obj, interpRangedCombat),
		CREATE_JSON_VALUE(obj, interpMagicCombat),
		CREATE_JSON_VALUE(obj, interpMeleeCombat),
		CREATE_JSON_VALUE(obj, interpHorseback),

		CREATE_JSON_VALUE(obj, interpConf),
		CREATE_JSON_VALUE(obj, interpRangedConf),
		CREATE_JSON_VALUE(obj, interpMagicConf),
		CREATE_JSON_VALUE(obj, interpMeleeConf),
		CREATE_JSON_VALUE(obj, interpHorsebackConf)
	};
}

void Config::from_json(const json& j, OffsetGroup& obj) {
	VALUE_FROM_JSON(obj, sideOffset)
	VALUE_FROM_JSON(obj, upOffset)
	VALUE_FROM_JSON(obj, zoomOffset)
	VALUE_FROM_JSON(obj, fovOffset)

	VALUE_FROM_JSON(obj, combatRangedSideOffset)
	VALUE_FROM_JSON(obj, combatRangedUpOffset)
	VALUE_FROM_JSON(obj, combatRangedZoomOffset)
	VALUE_FROM_JSON(obj, combatRangedFOVOffset)

	VALUE_FROM_JSON(obj, combatMagicSideOffset)
	VALUE_FROM_JSON(obj, combatMagicUpOffset)
	VALUE_FROM_JSON(obj, combatMagicZoomOffset)
	VALUE_FROM_JSON(obj, combatMagicFOVOffset)

	VALUE_FROM_JSON(obj, combatMeleeSideOffset)
	VALUE_FROM_JSON(obj, combatMeleeUpOffset)
	VALUE_FROM_JSON(obj, combatMeleeZoomOffset)
	VALUE_FROM_JSON(obj, combatMeleeFOVOffset)

	VALUE_FROM_JSON(obj, horseSideOffset)
	VALUE_FROM_JSON(obj, horseUpOffset)
	VALUE_FROM_JSON(obj, horseZoomOffset)
	VALUE_FROM_JSON(obj, horseFOVOffset)

	VALUE_FROM_JSON(obj, interp)
	VALUE_FROM_JSON(obj, interpRangedCombat)
	VALUE_FROM_JSON(obj, interpMagicCombat)
	VALUE_FROM_JSON(obj, interpMeleeCombat)
	VALUE_FROM_JSON(obj, interpHorseback)

	VALUE_FROM_JSON(obj, interpConf)
	VALUE_FROM_JSON(obj, interpRangedConf)
	VALUE_FROM_JSON(obj, interpMagicConf)
	VALUE_FROM_JSON(obj, interpMeleeConf)
	VALUE_FROM_JSON(obj, interpHorsebackConf)
}

void Config::to_json(json& j, const UserConfig& obj) {
	j = json{
		// Crosshair
		CREATE_JSON_VALUE(obj, use3DBowAimCrosshair),
		CREATE_JSON_VALUE(obj, use3DMagicCrosshair),
		CREATE_JSON_VALUE(obj, hideNonCombatCrosshair),
		CREATE_JSON_VALUE(obj, hideCrosshairMeleeCombat),
		CREATE_JSON_VALUE(obj, enableCrosshairSizeManip),
		CREATE_JSON_VALUE(obj, crosshairNPCHitGrowSize),
		CREATE_JSON_VALUE(obj, crosshairMinDistSize),
		CREATE_JSON_VALUE(obj, crosshairMaxDistSize),
		CREATE_JSON_VALUE(obj, useWorldCrosshair),
		CREATE_JSON_VALUE(obj, worldCrosshairDepthTest),
		CREATE_JSON_VALUE(obj, worldCrosshairType),
		CREATE_JSON_VALUE(obj, stealthMeterXOffset),
		CREATE_JSON_VALUE(obj, stealthMeterYOffset),
		CREATE_JSON_VALUE(obj, offsetStealthMeter),
		CREATE_JSON_VALUE(obj, alwaysOffsetStealthMeter),

		// Arrow prediction
		CREATE_JSON_VALUE(obj, useArrowPrediction),
		CREATE_JSON_VALUE(obj, drawArrowArc),
		CREATE_JSON_VALUE(obj, arrowArcColor),
		CREATE_JSON_VALUE(obj, maxArrowPredictionRange),

		// Misc
		CREATE_JSON_VALUE(obj, disableDeltaTime),
		CREATE_JSON_VALUE(obj, nextPresetKey),
		CREATE_JSON_VALUE(obj, shoulderSwapKey),
		CREATE_JSON_VALUE(obj, swapXClamping),
		CREATE_JSON_VALUE(obj, modDisabled),
		CREATE_JSON_VALUE(obj, modToggleKey),
		CREATE_JSON_VALUE(obj, customZOffset),
		CREATE_JSON_VALUE(obj, applyZOffsetKey),
		CREATE_JSON_VALUE(obj, enableCrashDumps),
		CREATE_JSON_VALUE(obj, toggleUserDefinedOffsetGroupKey),

		// Pitch Zoom
		CREATE_JSON_VALUE(obj, enablePitchZoom),
		CREATE_JSON_VALUE(obj, pitchZoomAfterInterp),
		CREATE_JSON_VALUE(obj, pitchZoomMaxAngle),
		CREATE_JSON_VALUE(obj, pitchZoomMax),
		CREATE_JSON_VALUE(obj, pitchZoomMethod),

		// Primary interpolation
		CREATE_JSON_VALUE(obj, enableInterp),
		CREATE_JSON_VALUE(obj, currentScalar),
		CREATE_JSON_VALUE(obj, minCameraFollowDistance),
		CREATE_JSON_VALUE(obj, minCameraFollowRate),
		CREATE_JSON_VALUE(obj, maxCameraFollowRate),
		CREATE_JSON_VALUE(obj, zoomMul),
		CREATE_JSON_VALUE(obj, zoomMaxSmoothingDistance),

		// Separate local space interpolation
		CREATE_JSON_VALUE(obj, separateLocalInterp),
		CREATE_JSON_VALUE(obj, separateLocalScalar),
		CREATE_JSON_VALUE(obj, localMinFollowRate),
		CREATE_JSON_VALUE(obj, localMaxFollowRate),
		CREATE_JSON_VALUE(obj, localMaxSmoothingDistance),

		// Separate Z
		CREATE_JSON_VALUE(obj, separateZInterp),
		CREATE_JSON_VALUE(obj, separateZScalar),
		CREATE_JSON_VALUE(obj, separateZMaxSmoothingDistance),
		CREATE_JSON_VALUE(obj, separateZMinFollowRate),
		CREATE_JSON_VALUE(obj, separateZMaxFollowRate),
		
		// Offset interpolation
		CREATE_JSON_VALUE(obj, enableOffsetInterpolation),
		CREATE_JSON_VALUE(obj, offsetScalar),
		CREATE_JSON_VALUE(obj, offsetInterpDurationSecs),

		// Zoom interpolation
		CREATE_JSON_VALUE(obj, enableZoomInterpolation),
		CREATE_JSON_VALUE(obj, zoomScalar),
		CREATE_JSON_VALUE(obj, zoomInterpDurationSecs),

		// FOV interpolation
		CREATE_JSON_VALUE(obj, enableFOVInterpolation),
		CREATE_JSON_VALUE(obj, fovScalar),
		CREATE_JSON_VALUE(obj, fovInterpDurationSecs),

		// Interp override smoother
		CREATE_JSON_VALUE(obj, globalInterpDisableSmoothing),
		CREATE_JSON_VALUE(obj, globalInterpDisableMehtod),
		CREATE_JSON_VALUE(obj, globalInterpOverrideSmoothing),
		CREATE_JSON_VALUE(obj, globalInterpOverrideMethod),
		CREATE_JSON_VALUE(obj, localInterpOverrideSmoothing),
		CREATE_JSON_VALUE(obj, localInterpOverrideMethod),

		// Dialogue
		CREATE_JSON_VALUE(obj, dialogueMode),
		CREATE_JSON_VALUE(obj, oblivionDialogue),
		CREATE_JSON_VALUE(obj, faceToFaceDialogue),

		// Distance clamping
		CREATE_JSON_VALUE(obj, cameraDistanceClampXEnable),
		CREATE_JSON_VALUE(obj, cameraDistanceClampXMin),
		CREATE_JSON_VALUE(obj, cameraDistanceClampXMax),
		CREATE_JSON_VALUE(obj, cameraDistanceClampYEnable),
		CREATE_JSON_VALUE(obj, cameraDistanceClampYMin),
		CREATE_JSON_VALUE(obj, cameraDistanceClampYMax),
		CREATE_JSON_VALUE(obj, cameraDistanceClampZEnable),
		CREATE_JSON_VALUE(obj, cameraDistanceClampZMin),
		CREATE_JSON_VALUE(obj, cameraDistanceClampZMax),

		// Per state positions
		CREATE_JSON_VALUE(obj, standing),
		CREATE_JSON_VALUE(obj, walking),
		CREATE_JSON_VALUE(obj, running),
		CREATE_JSON_VALUE(obj, sprinting),
		CREATE_JSON_VALUE(obj, sneaking),
		CREATE_JSON_VALUE(obj, swimming),
		CREATE_JSON_VALUE(obj, bowAim),
		CREATE_JSON_VALUE(obj, sitting),
		CREATE_JSON_VALUE(obj, horseback),
		CREATE_JSON_VALUE(obj, dragon),
		CREATE_JSON_VALUE(obj, vampireLord),
		CREATE_JSON_VALUE(obj, werewolf),
		CREATE_JSON_VALUE(obj, vanity),
		CREATE_JSON_VALUE(obj, userDefined)
	};
}

void Config::from_json(const json& j, UserConfig& obj) {
	// Crosshair
	VALUE_FROM_JSON(obj, use3DBowAimCrosshair)
	VALUE_FROM_JSON(obj, use3DMagicCrosshair)
	VALUE_FROM_JSON(obj, hideNonCombatCrosshair)
	VALUE_FROM_JSON(obj, hideCrosshairMeleeCombat)
	VALUE_FROM_JSON(obj, enableCrosshairSizeManip)
	VALUE_FROM_JSON(obj, crosshairNPCHitGrowSize)
	VALUE_FROM_JSON(obj, crosshairMinDistSize)
	VALUE_FROM_JSON(obj, crosshairMaxDistSize)
	VALUE_FROM_JSON(obj, useWorldCrosshair)
	VALUE_FROM_JSON(obj, worldCrosshairDepthTest)
	VALUE_FROM_JSON(obj, worldCrosshairType)
	VALUE_FROM_JSON(obj, stealthMeterXOffset)
	VALUE_FROM_JSON(obj, stealthMeterYOffset)
	VALUE_FROM_JSON(obj, offsetStealthMeter)
	VALUE_FROM_JSON(obj, alwaysOffsetStealthMeter)

	// Arrow prediction
	VALUE_FROM_JSON(obj, useArrowPrediction)
	VALUE_FROM_JSON(obj, drawArrowArc)
	VALUE_FROM_JSON(obj, arrowArcColor)
	VALUE_FROM_JSON(obj, maxArrowPredictionRange)

	// Misc
	VALUE_FROM_JSON(obj, disableDeltaTime)
	VALUE_FROM_JSON(obj, nextPresetKey)
	VALUE_FROM_JSON(obj, shoulderSwapKey)
	VALUE_FROM_JSON(obj, swapXClamping)
	VALUE_FROM_JSON(obj, modDisabled)
	VALUE_FROM_JSON(obj, modToggleKey)
	VALUE_FROM_JSON(obj, customZOffset)
	VALUE_FROM_JSON(obj, applyZOffsetKey)
	VALUE_FROM_JSON(obj, enableCrashDumps)
	VALUE_FROM_JSON(obj, toggleUserDefinedOffsetGroupKey)

	// Pitch Zoom
	VALUE_FROM_JSON(obj, enablePitchZoom)
	VALUE_FROM_JSON(obj, pitchZoomAfterInterp)
	VALUE_FROM_JSON(obj, pitchZoomMaxAngle)
	VALUE_FROM_JSON(obj, pitchZoomMax)
	VALUE_FROM_JSON(obj, pitchZoomMethod)

	// Primary interpolation
	VALUE_FROM_JSON(obj, enableInterp)
	VALUE_FROM_JSON(obj, currentScalar)
	VALUE_FROM_JSON(obj, minCameraFollowDistance)
	VALUE_FROM_JSON(obj, minCameraFollowRate)
	VALUE_FROM_JSON(obj, maxCameraFollowRate)
	VALUE_FROM_JSON(obj, zoomMul)
	VALUE_FROM_JSON(obj, zoomMaxSmoothingDistance)

	// Separate local space interpolation
	VALUE_FROM_JSON(obj, separateLocalInterp)
	VALUE_FROM_JSON(obj, separateLocalScalar)

	if (j.contains("localScalarRate")) {
		// Pre 1.5 porting
		obj.localMinFollowRate = obj.localMaxFollowRate =
			j.value("localScalarRate", 1.0f);

	} else {
		VALUE_FROM_JSON(obj, localMinFollowRate)
		VALUE_FROM_JSON(obj, localMaxFollowRate)
		VALUE_FROM_JSON(obj, localMaxSmoothingDistance)
	}

	// Separate Z
	VALUE_FROM_JSON(obj, separateZInterp)
	VALUE_FROM_JSON(obj, separateZScalar)
	VALUE_FROM_JSON(obj, separateZMaxSmoothingDistance)
	VALUE_FROM_JSON(obj, separateZMinFollowRate)
	VALUE_FROM_JSON(obj, separateZMaxFollowRate)

	// Offset interpolation
	VALUE_FROM_JSON(obj, enableOffsetInterpolation)
	VALUE_FROM_JSON(obj, offsetScalar)
	VALUE_FROM_JSON(obj, offsetInterpDurationSecs)

	// Zoom interpolation
	VALUE_FROM_JSON(obj, enableZoomInterpolation)
	VALUE_FROM_JSON(obj, zoomScalar)
	VALUE_FROM_JSON(obj, zoomInterpDurationSecs)

	// FOV interpolation
	VALUE_FROM_JSON(obj, enableFOVInterpolation)
	VALUE_FROM_JSON(obj, fovScalar)
	VALUE_FROM_JSON(obj, fovInterpDurationSecs)

	// Dialogue
	VALUE_FROM_JSON(obj, dialogueMode)
	VALUE_FROM_JSON(obj, oblivionDialogue)
	VALUE_FROM_JSON(obj, faceToFaceDialogue)

	// Distance clamping
	VALUE_FROM_JSON(obj, cameraDistanceClampXEnable)
	VALUE_FROM_JSON(obj, cameraDistanceClampXMin)
	VALUE_FROM_JSON(obj, cameraDistanceClampXMax)
	VALUE_FROM_JSON(obj, cameraDistanceClampYEnable)
	VALUE_FROM_JSON(obj, cameraDistanceClampYMin)
	VALUE_FROM_JSON(obj, cameraDistanceClampYMax)
	VALUE_FROM_JSON(obj, cameraDistanceClampZEnable)
	VALUE_FROM_JSON(obj, cameraDistanceClampZMin)
	VALUE_FROM_JSON(obj, cameraDistanceClampZMax)

	// Interp override smoother
	VALUE_FROM_JSON(obj, globalInterpDisableSmoothing)
	VALUE_FROM_JSON(obj, globalInterpDisableMehtod)
	VALUE_FROM_JSON(obj, globalInterpOverrideSmoothing)
	VALUE_FROM_JSON(obj, globalInterpOverrideMethod)
	VALUE_FROM_JSON(obj, localInterpOverrideSmoothing)
	VALUE_FROM_JSON(obj, localInterpOverrideMethod)

	// Per state positions
	VALUE_FROM_JSON(obj, standing)
	VALUE_FROM_JSON(obj, walking)
	VALUE_FROM_JSON(obj, running)
	VALUE_FROM_JSON(obj, sprinting)
	VALUE_FROM_JSON(obj, sneaking)
	VALUE_FROM_JSON(obj, swimming)
	VALUE_FROM_JSON(obj, bowAim)
	VALUE_FROM_JSON(obj, sitting)
	VALUE_FROM_JSON(obj, horseback)
	VALUE_FROM_JSON(obj, dragon)
	VALUE_FROM_JSON(obj, vampireLord)
	VALUE_FROM_JSON(obj, werewolf)
	VALUE_FROM_JSON(obj, vanity)
	VALUE_FROM_JSON(obj, userDefined)

	obj.standing.id = OffsetGroupID::Standing;
	obj.walking.id = OffsetGroupID::Walking;
	obj.running.id = OffsetGroupID::Running;
	obj.sprinting.id = OffsetGroupID::Sprinting;
	obj.sneaking.id = OffsetGroupID::Sneaking;
	obj.swimming.id = OffsetGroupID::Swimming;
	obj.bowAim.id = OffsetGroupID::BowAim;
	obj.sitting.id = OffsetGroupID::Sitting;
	obj.horseback.id = OffsetGroupID::Horseback;
	obj.dragon.id = OffsetGroupID::Dragon;
	obj.vampireLord.id = OffsetGroupID::VampireLord;
	obj.werewolf.id = OffsetGroupID::Werewolf;
	obj.vanity.id = OffsetGroupID::Vanity;
	obj.userDefined.id = OffsetGroupID::UserDefined;
}

void Config::to_json(json& j, const Preset& obj) {
	j = json{
		CREATE_JSON_VALUE(obj, name),
		CREATE_JSON_VALUE(obj, config)
	};
}

void Config::from_json(const json& j, Preset& obj) {
	VALUE_FROM_JSON(obj, name)
	VALUE_FROM_JSON(obj, config)
}

void Config::to_json(json& j, const Color& obj) {
	j = json{
		CREATE_JSON_VALUE(obj, r),
		CREATE_JSON_VALUE(obj, g),
		CREATE_JSON_VALUE(obj, b),
		CREATE_JSON_VALUE(obj, a)
	};
}

void Config::from_json(const json& j, Color& obj) {
	VALUE_FROM_JSON(obj, r);
	VALUE_FROM_JSON(obj, g);
	VALUE_FROM_JSON(obj, b);
	VALUE_FROM_JSON(obj, a);
}

const Config::UserConfig& Config::GetDefaultConfig() noexcept {
	static auto conf = UserConfig{};
	conf.standing.sideOffset = 25.0f;
	conf.standing.upOffset = 0.0f;
	conf.standing.zoomOffset = 0.0f;
	conf.standing.fovOffset = 0.0f;
	conf.standing.combatRangedSideOffset = 40.0f;
	conf.standing.combatRangedUpOffset = -10.0f;
	conf.standing.combatRangedZoomOffset = 5.0f;
	conf.standing.combatRangedFOVOffset = 0.0f;
	conf.standing.combatMagicSideOffset = 40.0f;
	conf.standing.combatMagicUpOffset = -10.0f;
	conf.standing.combatMagicZoomOffset = 0.0f;
	conf.standing.combatMagicFOVOffset = 0.0f;
	conf.standing.combatMeleeSideOffset = 30.0f;
	conf.standing.combatMeleeUpOffset = -5.0f;
	conf.standing.combatMeleeZoomOffset = 0.0f;
	conf.standing.combatMeleeFOVOffset = 0.0f;
	conf.standing.interp = true;
	conf.standing.interpRangedCombat = true;
	conf.standing.interpMagicCombat = true;
	conf.standing.interpMeleeCombat = true;
	conf.standing.interpHorseback = true;
	conf.standing.interpConf.overrideInterp = false;
	conf.standing.interpConf.currentScalar = ScalarMethods::SINE_IN;
	conf.standing.interpConf.minCameraFollowRate = 0.33f;
	conf.standing.interpConf.maxCameraFollowRate = 0.85f;
	conf.standing.interpConf.zoomMaxSmoothingDistance = 650.0f;
	conf.standing.interpConf.overrideLocalInterp = true;
	conf.standing.interpConf.separateLocalScalar = ScalarMethods::QUAD_IN;
	conf.standing.interpConf.localMinFollowRate = 0.2f;
	conf.standing.interpConf.localMaxFollowRate = 0.8f;
	conf.standing.interpConf.localMaxSmoothingDistance = 150.0f;
	conf.standing.interpRangedConf.overrideInterp = false;
	conf.standing.interpRangedConf.currentScalar = ScalarMethods::SINE_IN;
	conf.standing.interpRangedConf.minCameraFollowRate = 0.33f;
	conf.standing.interpRangedConf.maxCameraFollowRate = 0.85f;
	conf.standing.interpRangedConf.zoomMaxSmoothingDistance = 650.0f;
	conf.standing.interpRangedConf.overrideLocalInterp = true;
	conf.standing.interpRangedConf.separateLocalScalar = ScalarMethods::SINE_IN;
	conf.standing.interpRangedConf.localMinFollowRate = 0.5f;
	conf.standing.interpRangedConf.localMaxFollowRate = 0.85f;
	conf.standing.interpRangedConf.localMaxSmoothingDistance = 120.0f;
	conf.standing.interpMagicConf.overrideInterp = false;
	conf.standing.interpMagicConf.currentScalar = ScalarMethods::SINE_IN;
	conf.standing.interpMagicConf.minCameraFollowRate = 0.33f;
	conf.standing.interpMagicConf.maxCameraFollowRate = 0.85f;
	conf.standing.interpMagicConf.zoomMaxSmoothingDistance = 650.0f;
	conf.standing.interpMagicConf.overrideLocalInterp = true;
	conf.standing.interpMagicConf.separateLocalScalar = ScalarMethods::SINE_IN;
	conf.standing.interpMagicConf.localMinFollowRate = 0.5f;
	conf.standing.interpMagicConf.localMaxFollowRate = 0.85f;
	conf.standing.interpMagicConf.localMaxSmoothingDistance = 120.0f;
	conf.standing.interpMeleeConf.overrideInterp = false;
	conf.standing.interpMeleeConf.currentScalar = ScalarMethods::SINE_IN;
	conf.standing.interpMeleeConf.minCameraFollowRate = 0.33f;
	conf.standing.interpMeleeConf.maxCameraFollowRate = 0.85f;
	conf.standing.interpMeleeConf.zoomMaxSmoothingDistance = 650.0f;
	conf.standing.interpMeleeConf.overrideLocalInterp = true;
	conf.standing.interpMeleeConf.separateLocalScalar = ScalarMethods::SINE_IN;
	conf.standing.interpMeleeConf.localMinFollowRate = 0.5f;
	conf.standing.interpMeleeConf.localMaxFollowRate = 0.85f;
	conf.standing.interpMeleeConf.localMaxSmoothingDistance = 120.0f;
	conf.standing.id = OffsetGroupID::Standing;

	conf.walking.sideOffset = 25.0f;
	conf.walking.upOffset = 0.0f;
	conf.walking.zoomOffset = 0.0f;
	conf.walking.fovOffset = 0.0f;
	conf.walking.combatRangedSideOffset = 40.0f;
	conf.walking.combatRangedUpOffset = -10.0f;
	conf.walking.combatRangedZoomOffset = 5.0f;
	conf.walking.combatRangedFOVOffset = 0.0f;
	conf.walking.combatMagicSideOffset = 40.0f;
	conf.walking.combatMagicUpOffset = -10.0f;
	conf.walking.combatMagicZoomOffset = 0.0f;
	conf.walking.combatMagicFOVOffset = 0.0f;
	conf.walking.combatMeleeSideOffset = 30.0f;
	conf.walking.combatMeleeUpOffset = -5.0f;
	conf.walking.combatMeleeZoomOffset = 0.0f;
	conf.walking.combatMeleeFOVOffset = 0.0f;
	conf.walking.interp = true;
	conf.walking.interpRangedCombat = true;
	conf.walking.interpMagicCombat = true;
	conf.walking.interpMeleeCombat = true;
	conf.walking.interpHorseback = true;
	conf.walking.interpConf.overrideInterp = true;
	conf.walking.interpConf.currentScalar = ScalarMethods::CUBIC_IN;
	conf.walking.interpConf.minCameraFollowRate = 0.45f;
	conf.walking.interpConf.maxCameraFollowRate = 0.79f;
	conf.walking.interpConf.zoomMaxSmoothingDistance = 650.0f;
	conf.walking.interpConf.overrideLocalInterp = true;
	conf.walking.interpConf.separateLocalScalar = ScalarMethods::SINE_IN;
	conf.walking.interpConf.localMinFollowRate = 0.35f;
	conf.walking.interpConf.localMaxFollowRate = 0.98f;
	conf.walking.interpConf.localMaxSmoothingDistance = 60.0f;
	conf.walking.interpRangedConf.overrideInterp = false;
	conf.walking.interpRangedConf.currentScalar = ScalarMethods::SINE_IN;
	conf.walking.interpRangedConf.minCameraFollowRate = 0.33f;
	conf.walking.interpRangedConf.maxCameraFollowRate = 0.85f;
	conf.walking.interpRangedConf.zoomMaxSmoothingDistance = 650.0f;
	conf.walking.interpRangedConf.overrideLocalInterp = true;
	conf.walking.interpRangedConf.separateLocalScalar = ScalarMethods::SINE_IN;
	conf.walking.interpRangedConf.localMinFollowRate = 0.77f;
	conf.walking.interpRangedConf.localMaxFollowRate = 0.85f;
	conf.walking.interpRangedConf.localMaxSmoothingDistance = 120.0f;
	conf.walking.interpMagicConf.overrideInterp = false;
	conf.walking.interpMagicConf.currentScalar = ScalarMethods::SINE_IN;
	conf.walking.interpMagicConf.minCameraFollowRate = 0.33f;
	conf.walking.interpMagicConf.maxCameraFollowRate = 0.85f;
	conf.walking.interpMagicConf.zoomMaxSmoothingDistance = 650.0f;
	conf.walking.interpMagicConf.overrideLocalInterp = true;
	conf.walking.interpMagicConf.separateLocalScalar = ScalarMethods::SINE_IN;
	conf.walking.interpMagicConf.localMinFollowRate = 0.77f;
	conf.walking.interpMagicConf.localMaxFollowRate = 0.85f;
	conf.walking.interpMagicConf.localMaxSmoothingDistance = 120.0f;
	conf.walking.interpMeleeConf.overrideInterp = false;
	conf.walking.interpMeleeConf.currentScalar = ScalarMethods::SINE_IN;
	conf.walking.interpMeleeConf.minCameraFollowRate = 0.33f;
	conf.walking.interpMeleeConf.maxCameraFollowRate = 0.85f;
	conf.walking.interpMeleeConf.zoomMaxSmoothingDistance = 650.0f;
	conf.walking.interpMeleeConf.overrideLocalInterp = true;
	conf.walking.interpMeleeConf.separateLocalScalar = ScalarMethods::SINE_IN;
	conf.walking.interpMeleeConf.localMinFollowRate = 0.77f;
	conf.walking.interpMeleeConf.localMaxFollowRate = 0.85f;
	conf.walking.interpMeleeConf.localMaxSmoothingDistance = 120.0f;
	conf.walking.id = OffsetGroupID::Walking;

	conf.running.sideOffset = 25.0f;
	conf.running.upOffset = 0.0f;
	conf.running.zoomOffset = 0.0f;
	conf.running.fovOffset = 0.0f;
	conf.running.combatRangedSideOffset = 40.0f;
	conf.running.combatRangedUpOffset = -10.0f;
	conf.running.combatRangedZoomOffset = 5.0f;
	conf.running.combatRangedFOVOffset = 0.0f;
	conf.running.combatMagicSideOffset = 40.0f;
	conf.running.combatMagicUpOffset = -10.0f;
	conf.running.combatMagicZoomOffset = 0.0f;
	conf.running.combatMagicFOVOffset = 0.0f;
	conf.running.combatMeleeSideOffset = 30.0f;
	conf.running.combatMeleeUpOffset = -5.0f;
	conf.running.combatMeleeZoomOffset = 0.0f;
	conf.running.combatMeleeFOVOffset = 0.0f;
	conf.running.interp = true;
	conf.running.interpRangedCombat = true;
	conf.running.interpMagicCombat = true;
	conf.running.interpMeleeCombat = true;
	conf.running.interpHorseback = true;
	conf.running.interpConf.overrideInterp = true;
	conf.running.interpConf.currentScalar = ScalarMethods::SINE_IN;
	conf.running.interpConf.minCameraFollowRate = 0.27f;
	conf.running.interpConf.maxCameraFollowRate = 0.73f;
	conf.running.interpConf.zoomMaxSmoothingDistance = 650.0f;
	conf.running.interpConf.overrideLocalInterp = true;
	conf.running.interpConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.running.interpConf.localMinFollowRate = 1.0f;
	conf.running.interpConf.localMaxFollowRate = 1.0f;
	conf.running.interpConf.localMaxSmoothingDistance = 60.0f;
	conf.running.interpRangedConf.overrideInterp = true;
	conf.running.interpRangedConf.currentScalar = ScalarMethods::CUBIC_IN;
	conf.running.interpRangedConf.minCameraFollowRate = 0.4f;
	conf.running.interpRangedConf.maxCameraFollowRate = 0.75f;
	conf.running.interpRangedConf.zoomMaxSmoothingDistance = 650.0f;
	conf.running.interpRangedConf.overrideLocalInterp = true;
	conf.running.interpRangedConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.running.interpRangedConf.localMinFollowRate = 1.0f;
	conf.running.interpRangedConf.localMaxFollowRate = 1.0f;
	conf.running.interpRangedConf.localMaxSmoothingDistance = 60.0f;
	conf.running.interpMagicConf.overrideInterp = true;
	conf.running.interpMagicConf.currentScalar = ScalarMethods::CUBIC_IN;
	conf.running.interpMagicConf.minCameraFollowRate = 0.4f;
	conf.running.interpMagicConf.maxCameraFollowRate = 0.75f;
	conf.running.interpMagicConf.zoomMaxSmoothingDistance = 650.0f;
	conf.running.interpMagicConf.overrideLocalInterp = true;
	conf.running.interpMagicConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.running.interpMagicConf.localMinFollowRate = 1.0f;
	conf.running.interpMagicConf.localMaxFollowRate = 1.0f;
	conf.running.interpMagicConf.localMaxSmoothingDistance = 60.0f;
	conf.running.interpMeleeConf.overrideInterp = true;
	conf.running.interpMeleeConf.currentScalar = ScalarMethods::SINE_IN;
	conf.running.interpMeleeConf.minCameraFollowRate = 0.4f;
	conf.running.interpMeleeConf.maxCameraFollowRate = 0.75f;
	conf.running.interpMeleeConf.zoomMaxSmoothingDistance = 650.0f;
	conf.running.interpMeleeConf.overrideLocalInterp = true;
	conf.running.interpMeleeConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.running.interpMeleeConf.localMinFollowRate = 1.0f;
	conf.running.interpMeleeConf.localMaxFollowRate = 1.0f;
	conf.running.interpMeleeConf.localMaxSmoothingDistance = 60.0f;
	conf.running.id = OffsetGroupID::Running;

	conf.sprinting.sideOffset = 40.0f;
	conf.sprinting.upOffset = -10.0f;
	conf.sprinting.zoomOffset = 5.0f;
	conf.sprinting.fovOffset = -5.0f;
	conf.sprinting.combatRangedSideOffset = 40.0f;
	conf.sprinting.combatRangedUpOffset = -10.0f;
	conf.sprinting.combatRangedZoomOffset = 5.0f;
	conf.sprinting.combatRangedFOVOffset = -5.0;
	conf.sprinting.combatMagicSideOffset = 40.0f;
	conf.sprinting.combatMagicUpOffset = -10.0f;
	conf.sprinting.combatMagicZoomOffset = 5.0f;
	conf.sprinting.combatMagicFOVOffset = -5.0;
	conf.sprinting.combatMeleeSideOffset = 30.0f;
	conf.sprinting.combatMeleeUpOffset = -5.0f;
	conf.sprinting.combatMeleeZoomOffset = 5.0f;
	conf.sprinting.combatMeleeFOVOffset = -5.0;
	conf.sprinting.interp = true;
	conf.sprinting.interpRangedCombat = true;
	conf.sprinting.interpMagicCombat = true;
	conf.sprinting.interpMeleeCombat = true;
	conf.sprinting.interpHorseback = true;
	conf.sprinting.interpConf.overrideInterp = true;
	conf.sprinting.interpConf.currentScalar = ScalarMethods::SINE_IN;
	conf.sprinting.interpConf.minCameraFollowRate = 0.7f;
	conf.sprinting.interpConf.maxCameraFollowRate = 0.98f;
	conf.sprinting.interpConf.zoomMaxSmoothingDistance = 650.0f;
	conf.sprinting.interpConf.overrideLocalInterp = true;
	conf.sprinting.interpConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.sprinting.interpConf.localMinFollowRate = 1.0f;
	conf.sprinting.interpConf.localMaxFollowRate = 1.0f;
	conf.sprinting.interpConf.localMaxSmoothingDistance = 60.0f;
	conf.sprinting.interpRangedConf.overrideInterp = true;
	conf.sprinting.interpRangedConf.currentScalar = ScalarMethods::SINE_IN;
	conf.sprinting.interpRangedConf.minCameraFollowRate = 0.7f;
	conf.sprinting.interpRangedConf.maxCameraFollowRate = 0.98f;
	conf.sprinting.interpRangedConf.zoomMaxSmoothingDistance = 650.0f;
	conf.sprinting.interpRangedConf.overrideLocalInterp = true;
	conf.sprinting.interpRangedConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.sprinting.interpRangedConf.localMinFollowRate = 1.0f;
	conf.sprinting.interpRangedConf.localMaxFollowRate = 1.0f;
	conf.sprinting.interpRangedConf.localMaxSmoothingDistance = 60.0f;
	conf.sprinting.interpMagicConf.overrideInterp = true;
	conf.sprinting.interpMagicConf.currentScalar = ScalarMethods::SINE_IN;
	conf.sprinting.interpMagicConf.minCameraFollowRate = 0.7f;
	conf.sprinting.interpMagicConf.maxCameraFollowRate = 0.98f;
	conf.sprinting.interpMagicConf.zoomMaxSmoothingDistance = 650.0f;
	conf.sprinting.interpMagicConf.overrideLocalInterp = true;
	conf.sprinting.interpMagicConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.sprinting.interpMagicConf.localMinFollowRate = 1.0f;
	conf.sprinting.interpMagicConf.localMaxFollowRate = 1.0f;
	conf.sprinting.interpMagicConf.localMaxSmoothingDistance = 60.0f;
	conf.sprinting.interpMeleeConf.overrideInterp = true;
	conf.sprinting.interpMeleeConf.currentScalar = ScalarMethods::SINE_IN;
	conf.sprinting.interpMeleeConf.minCameraFollowRate = 0.7f;
	conf.sprinting.interpMeleeConf.maxCameraFollowRate = 0.98f;
	conf.sprinting.interpMeleeConf.zoomMaxSmoothingDistance = 650.0f;
	conf.sprinting.interpMeleeConf.overrideLocalInterp = true;
	conf.sprinting.interpMeleeConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.sprinting.interpMeleeConf.localMinFollowRate = 1.0f;
	conf.sprinting.interpMeleeConf.localMaxFollowRate = 1.0f;
	conf.sprinting.interpMeleeConf.localMaxSmoothingDistance = 60.0f;
	conf.sprinting.id = OffsetGroupID::Running;

	conf.sneaking.sideOffset = 40.0f;
	conf.sneaking.upOffset = -10.0f;
	conf.sneaking.zoomOffset = 5.0f;
	conf.sneaking.fovOffset = -5.0f;
	conf.sneaking.combatRangedSideOffset = 40.0f;
	conf.sneaking.combatRangedUpOffset = -10.0f;
	conf.sneaking.combatRangedZoomOffset = 5.0f;
	conf.sneaking.combatRangedFOVOffset = -5.0;
	conf.sneaking.combatMagicSideOffset = 40.0f;
	conf.sneaking.combatMagicUpOffset = -10.0f;
	conf.sneaking.combatMagicZoomOffset = 5.0f;
	conf.sneaking.combatMagicFOVOffset = -5.0;
	conf.sneaking.combatMeleeSideOffset = 35.0f;
	conf.sneaking.combatMeleeUpOffset = -10.0f;
	conf.sneaking.combatMeleeZoomOffset = 5.0f;
	conf.sneaking.combatMeleeFOVOffset = -5.0;
	conf.sneaking.interp = true;
	conf.sneaking.interpRangedCombat = true;
	conf.sneaking.interpMagicCombat = true;
	conf.sneaking.interpMeleeCombat = true;
	conf.sneaking.interpHorseback = true;
	conf.sneaking.interpConf.overrideInterp = true;
	conf.sneaking.interpConf.currentScalar = ScalarMethods::SINE_IN;
	conf.sneaking.interpConf.minCameraFollowRate = 0.7f;
	conf.sneaking.interpConf.maxCameraFollowRate = 0.98f;
	conf.sneaking.interpConf.zoomMaxSmoothingDistance = 650.0f;
	conf.sneaking.interpConf.overrideLocalInterp = true;
	conf.sneaking.interpConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.sneaking.interpConf.localMinFollowRate = 1.0f;
	conf.sneaking.interpConf.localMaxFollowRate = 1.0f;
	conf.sneaking.interpConf.localMaxSmoothingDistance = 60.0f;
	conf.sneaking.interpRangedConf.overrideInterp = true;
	conf.sneaking.interpRangedConf.currentScalar = ScalarMethods::SINE_IN;
	conf.sneaking.interpRangedConf.minCameraFollowRate = 0.7f;
	conf.sneaking.interpRangedConf.maxCameraFollowRate = 0.98f;
	conf.sneaking.interpRangedConf.zoomMaxSmoothingDistance = 650.0f;
	conf.sneaking.interpRangedConf.overrideLocalInterp = true;
	conf.sneaking.interpRangedConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.sneaking.interpRangedConf.localMinFollowRate = 1.0f;
	conf.sneaking.interpRangedConf.localMaxFollowRate = 1.0f;
	conf.sneaking.interpRangedConf.localMaxSmoothingDistance = 60.0f;
	conf.sneaking.interpMagicConf.overrideInterp = true;
	conf.sneaking.interpMagicConf.currentScalar = ScalarMethods::SINE_IN;
	conf.sneaking.interpMagicConf.minCameraFollowRate = 0.7f;
	conf.sneaking.interpMagicConf.maxCameraFollowRate = 0.98f;
	conf.sneaking.interpMagicConf.zoomMaxSmoothingDistance = 650.0f;
	conf.sneaking.interpMagicConf.overrideLocalInterp = true;
	conf.sneaking.interpMagicConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.sneaking.interpMagicConf.localMinFollowRate = 1.0f;
	conf.sneaking.interpMagicConf.localMaxFollowRate = 1.0f;
	conf.sneaking.interpMagicConf.localMaxSmoothingDistance = 60.0f;
	conf.sneaking.interpMeleeConf.overrideInterp = true;
	conf.sneaking.interpMeleeConf.currentScalar = ScalarMethods::SINE_IN;
	conf.sneaking.interpMeleeConf.minCameraFollowRate = 0.7f;
	conf.sneaking.interpMeleeConf.maxCameraFollowRate = 0.98f;
	conf.sneaking.interpMeleeConf.zoomMaxSmoothingDistance = 650.0f;
	conf.sneaking.interpMeleeConf.overrideLocalInterp = true;
	conf.sneaking.interpMeleeConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.sneaking.interpMeleeConf.localMinFollowRate = 1.0f;
	conf.sneaking.interpMeleeConf.localMaxFollowRate = 1.0f;
	conf.sneaking.interpMeleeConf.localMaxSmoothingDistance = 60.0f;
	conf.sneaking.id = OffsetGroupID::Sneaking;

	conf.swimming.sideOffset = 35.0f;
	conf.swimming.upOffset = 0.0f;
	conf.swimming.zoomOffset = 0.0f;
	conf.swimming.fovOffset = 0.0f;
	conf.swimming.interp = true;
	conf.swimming.interpConf.overrideInterp = false;
	conf.swimming.interpConf.currentScalar = ScalarMethods::SINE_IN;
	conf.swimming.interpConf.minCameraFollowRate = 0.33f;
	conf.swimming.interpConf.maxCameraFollowRate = 0.85f;
	conf.swimming.interpConf.zoomMaxSmoothingDistance = 650.0f;
	conf.swimming.interpConf.overrideLocalInterp = true;
	conf.swimming.interpConf.separateLocalScalar = ScalarMethods::SINE_IN;
	conf.swimming.interpConf.localMinFollowRate = 0.4f;
	conf.swimming.interpConf.localMaxFollowRate = 0.98f;
	conf.swimming.interpConf.localMaxSmoothingDistance = 120.0f;
	conf.swimming.id = OffsetGroupID::Swimming;

	conf.bowAim.sideOffset = 40.0f;
	conf.bowAim.upOffset = -10.0f;
	conf.bowAim.zoomOffset = 5.0f;
	conf.bowAim.fovOffset = -5.0f;
	conf.bowAim.combatMeleeSideOffset = 40.0f;
	conf.bowAim.combatMeleeUpOffset = -10.0f;
	conf.bowAim.combatMeleeZoomOffset = 5.0f;
	conf.bowAim.combatMeleeFOVOffset = -5.0f;
	conf.bowAim.horseSideOffset = 50.0f;
	conf.bowAim.horseUpOffset = 20.0f;
	conf.bowAim.horseZoomOffset = 5.0f;
	conf.bowAim.horseFOVOffset = -5.0f;
	conf.bowAim.interp = true;
	conf.bowAim.interpRangedCombat = true;
	conf.bowAim.interpMeleeCombat = true;
	conf.bowAim.interpHorseback = true;
	conf.bowAim.interpHorsebackConf.overrideInterp = true;
	conf.bowAim.interpHorsebackConf.currentScalar = ScalarMethods::LINEAR;
	conf.bowAim.interpHorsebackConf.minCameraFollowRate = 1.0f;
	conf.bowAim.interpHorsebackConf.maxCameraFollowRate = 1.0f;
	conf.bowAim.interpHorsebackConf.zoomMaxSmoothingDistance = 650.0f;
	conf.bowAim.interpHorsebackConf.overrideLocalInterp = true;
	conf.bowAim.interpHorsebackConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.bowAim.interpHorsebackConf.localMinFollowRate = 1.0f;
	conf.bowAim.interpHorsebackConf.localMaxFollowRate = 1.0f;
	conf.bowAim.interpHorsebackConf.localMaxSmoothingDistance = 60.0f;
	conf.bowAim.interpRangedConf.overrideInterp = true;
	conf.bowAim.interpRangedConf.currentScalar = ScalarMethods::LINEAR;
	conf.bowAim.interpRangedConf.minCameraFollowRate = 1.0f;
	conf.bowAim.interpRangedConf.maxCameraFollowRate = 1.0f;
	conf.bowAim.interpRangedConf.zoomMaxSmoothingDistance = 650.0f;
	conf.bowAim.interpRangedConf.overrideLocalInterp = true;
	conf.bowAim.interpRangedConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.bowAim.interpRangedConf.localMinFollowRate = 1.0f;
	conf.bowAim.interpRangedConf.localMaxFollowRate = 1.0f;
	conf.bowAim.interpRangedConf.localMaxSmoothingDistance = 60.0f;
	conf.bowAim.interpMeleeConf.overrideInterp = true;
	conf.bowAim.interpMeleeConf.currentScalar = ScalarMethods::LINEAR;
	conf.bowAim.interpMeleeConf.minCameraFollowRate = 1.0f;
	conf.bowAim.interpMeleeConf.maxCameraFollowRate = 1.0f;
	conf.bowAim.interpMeleeConf.zoomMaxSmoothingDistance = 650.0f;
	conf.bowAim.interpMeleeConf.overrideLocalInterp = true;
	conf.bowAim.interpMeleeConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.bowAim.interpMeleeConf.localMinFollowRate = 1.0f;
	conf.bowAim.interpMeleeConf.localMaxFollowRate = 1.0f;
	conf.bowAim.interpMeleeConf.localMaxSmoothingDistance = 60.0f;
	conf.bowAim.id = OffsetGroupID::BowAim;

	conf.sitting.sideOffset = 35.0f;
	conf.sitting.upOffset = -15.0f;
	conf.sitting.zoomOffset = 45.0f;
	conf.sitting.fovOffset = 0.0f;
	conf.sitting.interp = true;
	conf.sitting.interpConf.overrideInterp = false;
	conf.sitting.interpConf.currentScalar = ScalarMethods::SINE_IN;
	conf.sitting.interpConf.minCameraFollowRate = 0.25f;
	conf.sitting.interpConf.maxCameraFollowRate = 0.66f;
	conf.sitting.interpConf.zoomMaxSmoothingDistance = 650.0f;
	conf.sitting.interpConf.overrideLocalInterp = false;
	conf.sitting.interpConf.separateLocalScalar = ScalarMethods::EXP_IN;
	conf.sitting.interpConf.localMinFollowRate = 0.7f;
	conf.sitting.interpConf.localMaxFollowRate = 0.98f;
	conf.sitting.interpConf.localMaxSmoothingDistance = 60.0f;
	conf.sitting.id = OffsetGroupID::Sitting;

	conf.horseback.sideOffset = 35.0f;
	conf.horseback.upOffset = 20.0f;
	conf.horseback.zoomOffset = 0.0f;
	conf.horseback.fovOffset = 0.0f;
	conf.horseback.combatRangedSideOffset = 50.0f;
	conf.horseback.combatRangedUpOffset = 20.0f;
	conf.horseback.combatRangedZoomOffset = 0.0f;
	conf.horseback.combatRangedFOVOffset = 0.0;
	conf.horseback.combatMagicSideOffset = 50.0f;
	conf.horseback.combatMagicUpOffset = 20.0f;
	conf.horseback.combatMagicZoomOffset = 0.0f;
	conf.horseback.combatMagicFOVOffset = 0.0;
	conf.horseback.combatMeleeSideOffset = 50.0f;
	conf.horseback.combatMeleeUpOffset = 20.0f;
	conf.horseback.combatMeleeZoomOffset = 0.0f;
	conf.horseback.combatMeleeFOVOffset = 0.0;
	conf.horseback.interp = true;
	conf.horseback.interpRangedCombat = true;
	conf.horseback.interpMagicCombat = true;
	conf.horseback.interpMeleeCombat = true;
	conf.horseback.interpHorseback = true;
	conf.horseback.interpConf.overrideInterp = false;
	conf.horseback.interpConf.currentScalar = ScalarMethods::SINE_IN;
	conf.horseback.interpConf.minCameraFollowRate = 0.33f;
	conf.horseback.interpConf.maxCameraFollowRate = 0.98f;
	conf.horseback.interpConf.zoomMaxSmoothingDistance = 650.0f;
	conf.horseback.interpConf.overrideLocalInterp = true;
	conf.horseback.interpConf.separateLocalScalar = ScalarMethods::EXP_IN;
	conf.horseback.interpConf.localMinFollowRate = 0.7f;
	conf.horseback.interpConf.localMaxFollowRate = 0.98f;
	conf.horseback.interpConf.localMaxSmoothingDistance = 60.0f;
	conf.horseback.interpRangedConf.overrideInterp = true;
	conf.horseback.interpRangedConf.currentScalar = ScalarMethods::SINE_IN;
	conf.horseback.interpRangedConf.minCameraFollowRate = 0.33f;
	conf.horseback.interpRangedConf.maxCameraFollowRate = 0.98f;
	conf.horseback.interpRangedConf.zoomMaxSmoothingDistance = 650.0f;
	conf.horseback.interpRangedConf.overrideLocalInterp = true;
	conf.horseback.interpRangedConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.horseback.interpRangedConf.localMinFollowRate = 1.0f;
	conf.horseback.interpRangedConf.localMaxFollowRate = 1.0f;
	conf.horseback.interpRangedConf.localMaxSmoothingDistance = 60.0f;
	conf.horseback.interpMagicConf.overrideInterp = true;
	conf.horseback.interpMagicConf.currentScalar = ScalarMethods::SINE_IN;
	conf.horseback.interpMagicConf.minCameraFollowRate = 0.33f;
	conf.horseback.interpMagicConf.maxCameraFollowRate = 0.98f;
	conf.horseback.interpMagicConf.zoomMaxSmoothingDistance = 650.0f;
	conf.horseback.interpMagicConf.overrideLocalInterp = true;
	conf.horseback.interpMagicConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.horseback.interpMagicConf.localMinFollowRate = 1.0f;
	conf.horseback.interpMagicConf.localMaxFollowRate = 1.0f;
	conf.horseback.interpMagicConf.localMaxSmoothingDistance = 60.0f;
	conf.horseback.interpMeleeConf.overrideInterp = true;
	conf.horseback.interpMeleeConf.currentScalar = ScalarMethods::SINE_IN;
	conf.horseback.interpMeleeConf.minCameraFollowRate = 0.33f;
	conf.horseback.interpMeleeConf.maxCameraFollowRate = 0.98f;
	conf.horseback.interpMeleeConf.zoomMaxSmoothingDistance = 650.0f;
	conf.horseback.interpMeleeConf.overrideLocalInterp = true;
	conf.horseback.interpMeleeConf.separateLocalScalar = ScalarMethods::LINEAR;
	conf.horseback.interpMeleeConf.localMinFollowRate = 1.0f;
	conf.horseback.interpMeleeConf.localMaxFollowRate = 1.0f;
	conf.horseback.interpMeleeConf.localMaxSmoothingDistance = 60.0f;
	conf.horseback.id = OffsetGroupID::Horseback;

	conf.dragon.id = OffsetGroupID::Dragon;
	conf.vampireLord.id = OffsetGroupID::VampireLord;
	conf.werewolf.id = OffsetGroupID::Werewolf;
	conf.userDefined.id = OffsetGroupID::UserDefined;
	conf.vanity.id = OffsetGroupID::Vanity;

	return conf;
}

void Config::ReadConfigFile() {
	Config::UserConfig cfg;

	std::ifstream is(L"Data/SKSE/Plugins/SmoothCam.json");
	if (is.good()) {
		try {
			Config::json j;
			is >> j;
			cfg = j.get<Config::UserConfig>();
		} catch (const std::exception& e) {
			// Welp, something broke
			// Save the default config
			logger::warn(FMT_STRING("Failed to load user config! Loading Defaults. Error message: {}"), e.what());
			cfg = currentConfig = GetDefaultConfig();
			SaveCurrentConfig();
		}
	} else {
		// File not found? save our defaults
		cfg = currentConfig = GetDefaultConfig();
		SaveCurrentConfig();
	}

	// Load bone data
	LoadBonePriorities();
	LoadFocusBonePriorities();
#ifdef DEVELOPER
	LoadEyeBonePriorities();
#endif
	currentConfig = cfg;
}

void Config::SaveCurrentConfig() {
	std::ofstream os(L"Data/SKSE/Plugins/SmoothCam.json");
	const Config::json j = currentConfig;
	os << std::setw(4) << j << std::endl;
}

Config::UserConfig* Config::GetCurrentConfig() noexcept {
	return &currentConfig;
}

void Config::ResetConfig() {
	currentConfig = GetDefaultConfig();
	SaveCurrentConfig();
}

RE::BSFixedString Config::SaveConfigAsPreset(int slot, const RE::BSFixedString& name) {
	if (slot >= MaxPresetSlots) {
		return { "ERROR: Preset index out of range" };
	}

	Preset p;
	p.name = { name.c_str() };
	p.config = currentConfig;

	const auto path = GetPresetPath(slot);
	std::ofstream os(path.c_str());
	Config::json j = p;
	os << std::setw(4) << j << std::endl;

	return { "" };
}

bool Config::LoadPreset(int slot) {
	if (slot >= MaxPresetSlots) return false;
	
	Preset p;
	const auto path = GetPresetPath(slot);
	std::ifstream is(path.c_str());
	if (is.good()) {
		try {
			Config::json j;
			is >> j;
			p = j.get<Config::Preset>();
		} catch (const std::exception& e) {
			// Welp, something broke
			// Save the default config
			logger::warn(FMT_STRING("Failed to load preset config! Loading Defaults. Error message: {}"), e.what());
			return false;
		}
	} else {
		return false;
	}

	currentConfig = p.config;
	Config::SaveCurrentConfig();
	return true;
}

Config::LoadStatus Config::LoadPresetName(int slot, eastl::string& name) {
	if (slot >= MaxPresetSlots) return LoadStatus::FAILED;

	Preset p;
	const auto path = GetPresetPath(slot);
	std::ifstream is(path.c_str());
	if (is.good()) {
		try {
			Config::json j;
			is >> j;
			p = j.get<Config::Preset>();
		} catch (const std::exception& e) {
			logger::warn(FMT_STRING("Failed to load preset config! Error message: {}"), e.what());
			return LoadStatus::FAILED;
		}
	} else {
		return LoadStatus::MISSING;
	}

	name = p.name.c_str();
	return LoadStatus::OK;
}

RE::BSFixedString Config::GetPresetSlotName(int slot) {
	if (slot >= MaxPresetSlots)
		return { "ERROR: Preset index out of range" };

	eastl::string userName;
	const auto code = LoadPresetName(slot, userName);
	if (code == LoadStatus::OK)
		return { userName.c_str() };
	else if (code == LoadStatus::FAILED)
		return { "Error loading file" };
	else
		return { "Empty" };
}

eastl::wstring Config::GetPresetPath(int slot) {
	eastl::wstring slotName(L"Data/SKSE/Plugins/SmoothCamPreset");
	slotName.append(std::to_wstring(slot).c_str());
	slotName.append(L".json");
	return slotName;
}

static void trimString(std::string& outStr) {
	while(outStr.size() && isspace(outStr.front())) 
		outStr.erase(outStr.begin());

	while(outStr.size() && isspace(outStr.back())) 
		outStr.pop_back();
}

bool Config::LoadBoneList(const std::wstring_view&& searchName, BoneList& outBones) {
	// @Issue:35: std::filesystem::directory_iterator throws on unicode paths
	// @Note: Relative paths don't seem to work without MO2, prepend the full CWD
	const auto sz = GetCurrentDirectory(0, nullptr);
	WCHAR* buf = (WCHAR*)malloc(sizeof(WCHAR) * sz);
	GetCurrentDirectory(sz, buf);

	std::wstring path = L"\\\\?\\"; 
	path.append(buf);
	path.append(L"\\Data\\SKSE\\Plugins\\");
	path.append(searchName);

	WIN32_FIND_DATA data;
	auto hf = FindFirstFileEx(
		path.c_str(),
		FINDEX_INFO_LEVELS::FindExInfoStandard,
		&data,
		FINDEX_SEARCH_OPS::FindExSearchLimitToDirectories,
		nullptr,
		0
	);

	if (hf != INVALID_HANDLE_VALUE)
		do {
			std::wstring filePath = L"Data/SKSE/Plugins/";
			filePath.append(data.cFileName);
			std::ifstream ifs(filePath);
			if (ifs.good()) {
				std::string line;
				while (std::getline(ifs, line)) {
					trimString(line);
					if (line.length() == 0) continue;
					if (line.rfind("//", 0) == 0) continue;

					outBones.emplace_back(line.c_str());
				}
			}

			if (!FindNextFile(hf, &data)) {
				FindClose(hf);
				break;
			}
		} while (hf != INVALID_HANDLE_VALUE);
	else
		logger::warn(FMT_STRING("FindFirstFileEx reported error {}"), GetLastError());

	free(buf);

	return outBones.size() > 0;
}

static Config::BoneList bonePriorities = {};
void Config::LoadBonePriorities() {
	if (!LoadBoneList(L"SmoothCam_FollowBones_*.txt", bonePriorities)) {
		WarningPopup(LR"(SmoothCam: Did not find any bone names to follow while loading! Is SmoothCam_FollowBones_Default.txt present in the SKSE plugins directory?
Will fall back to default third-person camera bone.
To prevent this warning ensure a bone list file is present with at least 1 bone defined within and that SmoothCam is able to load it.)");
		bonePriorities.emplace_back("Camera3rd [Cam3]");
	}
}

Config::BoneList& Config::GetBonePriorities() noexcept {
	return bonePriorities;
}

static Config::BoneList focusBonePriorities = {};
void Config::LoadFocusBonePriorities() {
	if (!LoadBoneList(L"SmoothCam_FocusBones_*.txt", focusBonePriorities)) {
		WarningPopup(LR"(SmoothCam: Did not find any bone names to focus during dialogue while loading! Is SmoothCam_FocusBones_Default.txt present in the SKSE plugins directory?
Will fall back to default focal bone.
To prevent this warning ensure a bone list file is present with at least 1 bone defined within and that SmoothCam is able to load it.)");
		focusBonePriorities.emplace_back("NPC Head [Head]");
	}
}

Config::BoneList& Config::GetFocusBonePriorities() noexcept {
	return focusBonePriorities;
}

#ifdef DEVELOPER
static Config::BoneList eyeBonePriorities = {};
void Config::LoadEyeBonePriorities() {
	if (!LoadBoneList(L"SmoothCam_EyeBones_*.txt", eyeBonePriorities)) {
		WarningPopup(LR"(SmoothCam: Did not find any bone names to follow while loading! Is SmoothCam_EyeBones_Default.txt present in the SKSE plugins directory?
Will fall back to default first-person camera bone.
To prevent this warning ensure a bone list file is present with at least 1 bone defined within and that SmoothCam is able to load it.)");
		eyeBonePriorities.emplace_back("NPCEyeBone");
	}
}

Config::BoneList& Config::GetEyeBonePriorities() noexcept {
	return eyeBonePriorities;
}
#endif
#pragma warning(pop)