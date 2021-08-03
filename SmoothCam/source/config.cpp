Config::UserConfig currentConfig;

#define CREATE_JSON_VALUE(obj, member) {#member, obj.member}
#define VALUE_FROM_JSON(obj, member)	\
{										\
	const auto def = obj.##member##;	\
	obj.member = j.value(#member, def);	\
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
		CREATE_JSON_VALUE(obj, interpHorseback)
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

		// Comapt
		CREATE_JSON_VALUE(obj, compatACC),
		CREATE_JSON_VALUE(obj, compatIC),
		CREATE_JSON_VALUE(obj, compatIFPV),
		CREATE_JSON_VALUE(obj, compatAGO),

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
		CREATE_JSON_VALUE(obj, localScalarRate),

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
		CREATE_JSON_VALUE(obj, werewolf)
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

	// Compat
	VALUE_FROM_JSON(obj, compatACC)
	VALUE_FROM_JSON(obj, compatIC)
	VALUE_FROM_JSON(obj, compatIFPV)
	VALUE_FROM_JSON(obj, compatAGO)

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
	VALUE_FROM_JSON(obj, localScalarRate)

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

void Config::ReadConfigFile() {
	Config::UserConfig cfg;

	std::ifstream is(L"Data/SKSE/Plugins/SmoothCam.json");
	if (is.good()) {
		try {
			Config::json j;
			is >> j;
			cfg = j.get<Config::UserConfig>();
		} catch (std::exception& e) {
			// Welp, something broke
			// Save the default config
			_WARNING("%s - %s", "Failed to load user config! Loading Defaults. Error message:", e.what());
			SaveCurrentConfig();
		}
	} else {
		// File not found? save our defaults
		SaveCurrentConfig();
	}

	// Load bone data
	LoadBonePriorities();
#ifdef DEVELOPER
	LoadEyeBonePriorities();
#endif
	currentConfig = cfg;
}

void Config::SaveCurrentConfig() {
	std::ofstream os(L"Data/SKSE/Plugins/SmoothCam.json");
	Config::json j = currentConfig;
	os << std::setw(4) << j << std::endl;
}

Config::UserConfig* Config::GetCurrentConfig() noexcept {
	return &currentConfig;
}

void Config::ResetConfig() {
	currentConfig = {};
	SaveCurrentConfig();
}

BSFixedString Config::SaveConfigAsPreset(int slot, const BSFixedString& name) {
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
		} catch (std::exception& e) {
			// Welp, something broke
			// Save the default config
			_WARNING("%s <%d> %s", "Failed to load preset config! Error message:", slot, e.what());
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
		} catch (std::exception& e) {
			// Welp, something broke
			// Save the default config
			_WARNING("%s <%d> %s", "Failed to load preset config! Error message:", slot, e.what());
			return LoadStatus::FAILED;
		}
	} else {
		return LoadStatus::MISSING;
	}

	name = p.name.c_str();
	return LoadStatus::OK;
}

BSFixedString Config::GetPresetSlotName(int slot) {
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

void trimString(std::string& outStr) {
	while(outStr.size() && isspace(outStr.front())) 
		outStr.erase(outStr.begin());

	while(outStr.size() && isspace(outStr.back())) 
		outStr.pop_back();
}

static Config::BoneList bonePriorities = {};
void Config::LoadBonePriorities() {
	// @Issue:35: std::filesystem::directory_iterator throws on unicode paths
	// @Note: Relative paths don't seem to work without MO2, prepend the full CWD
	const auto sz = GetCurrentDirectory(0, nullptr);
	WCHAR* buf = (WCHAR*)malloc(sizeof(WCHAR) * sz);
	GetCurrentDirectory(sz, buf);

	std::wstring path = L"\\\\?\\"; 
	path.append(buf);
	path.append(L"\\Data\\SKSE\\Plugins\\SmoothCam_FollowBones_*.txt");

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

					bonePriorities.emplace_back(line.c_str());
				}
			}

			if (!FindNextFile(hf, &data)) {
				FindClose(hf);
				break;
			}
		} while (hf != INVALID_HANDLE_VALUE);
	else
		_WARNING("FindFirstFileEx reported error %d", GetLastError());

	free(buf);

	if (bonePriorities.size() == 0) {
		WarningPopup(LR"(SmoothCam: Did not find any bone names to follow while loading! Is SmoothCam_FollowBones_Default.txt present in the SKSE plugins directory?
Will fall back to default third-person camera bone.
To prevent this warning ensure a bone list file is present with at least 1 bone defined within and that SmoothCam is able to load it.)");
		bonePriorities.emplace_back("Camera3rd [Cam3]");
	}
}

Config::BoneList& Config::GetBonePriorities() noexcept {
	return bonePriorities;
}

#ifdef DEVELOPER
static Config::BoneList eyeBonePriorities = {};
void Config::LoadEyeBonePriorities() {
	// @Issue:35: std::filesystem::directory_iterator throws on unicode paths
	// @Note: Relative paths don't seem to work without MO2, prepend the full CWD
	const auto sz = GetCurrentDirectory(0, nullptr);
	WCHAR* buf = (WCHAR*)malloc(sizeof(WCHAR) * sz);
	GetCurrentDirectory(sz, buf);

	std::wstring path = L"\\\\?\\"; 
	path.append(buf);
	path.append(L"\\Data\\SKSE\\Plugins\\SmoothCam_EyeBones_*.txt");

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

					eyeBonePriorities.emplace_back(line.c_str());
				}
			}

			if (!FindNextFile(hf, &data)) {
				FindClose(hf);
				break;
			}
		} while (hf != INVALID_HANDLE_VALUE);
	else
		_WARNING("FindFirstFileEx reported error %d", GetLastError());

	free(buf);

	if (eyeBonePriorities.size() == 0) {
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