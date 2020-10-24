Config::UserConfig currentConfig;
Config::GameConfig gameConfig;

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

		CREATE_JSON_VALUE(obj, combatRangedSideOffset),
		CREATE_JSON_VALUE(obj, combatRangedUpOffset),
		CREATE_JSON_VALUE(obj, combatRangedZoomOffset),

		CREATE_JSON_VALUE(obj, combatMagicSideOffset),
		CREATE_JSON_VALUE(obj, combatMagicUpOffset),
		CREATE_JSON_VALUE(obj, combatMagicZoomOffset),

		CREATE_JSON_VALUE(obj, combatMeleeSideOffset),
		CREATE_JSON_VALUE(obj, combatMeleeUpOffset),
		CREATE_JSON_VALUE(obj, combatMeleeZoomOffset),

		CREATE_JSON_VALUE(obj, horseSideOffset),
		CREATE_JSON_VALUE(obj, horseUpOffset),
		CREATE_JSON_VALUE(obj, horseZoomOffset),

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

	VALUE_FROM_JSON(obj, combatRangedSideOffset)
	VALUE_FROM_JSON(obj, combatRangedUpOffset)
	VALUE_FROM_JSON(obj, combatRangedZoomOffset)

	VALUE_FROM_JSON(obj, combatMagicSideOffset)
	VALUE_FROM_JSON(obj, combatMagicUpOffset)
	VALUE_FROM_JSON(obj, combatMagicZoomOffset)

	VALUE_FROM_JSON(obj, combatMeleeSideOffset)
	VALUE_FROM_JSON(obj, combatMeleeUpOffset)
	VALUE_FROM_JSON(obj, combatMeleeZoomOffset)

	VALUE_FROM_JSON(obj, horseSideOffset)
	VALUE_FROM_JSON(obj, horseUpOffset)
	VALUE_FROM_JSON(obj, horseZoomOffset)

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
		//CREATE_JSON_VALUE(obj, crosshairNPCHitGrowSize),
		CREATE_JSON_VALUE(obj, crosshairMinDistSize),
		CREATE_JSON_VALUE(obj, crosshairMaxDistSize),
		CREATE_JSON_VALUE(obj, useWorldCrosshair),
		CREATE_JSON_VALUE(obj, worldCrosshairDepthTest),
		CREATE_JSON_VALUE(obj, worldCrosshairType),

		// Arrow prediction
		CREATE_JSON_VALUE(obj, useArrowPrediction),
		CREATE_JSON_VALUE(obj, drawArrowArc),
		CREATE_JSON_VALUE(obj, arrowArcColor),
		CREATE_JSON_VALUE(obj, maxArrowPredictionRange),

		// Misc
		CREATE_JSON_VALUE(obj, disableDeltaTime),
		CREATE_JSON_VALUE(obj, shoulderSwapKey),
		CREATE_JSON_VALUE(obj, swapXClamping),

		// Comapt
		CREATE_JSON_VALUE(obj, disableDuringDialog),
		CREATE_JSON_VALUE(obj, comaptIC_FirstPersonHorse),
		CREATE_JSON_VALUE(obj, comaptIC_FirstPersonDragon),
		CREATE_JSON_VALUE(obj, compatIC_FirstPersonSitting),
		CREATE_JSON_VALUE(obj, compatIFPV),

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
		CREATE_JSON_VALUE(obj, dragon)
	};
}

void Config::from_json(const json& j, UserConfig& obj) {
	// Crosshair
	VALUE_FROM_JSON(obj, use3DBowAimCrosshair)
	VALUE_FROM_JSON(obj, use3DMagicCrosshair)
	VALUE_FROM_JSON(obj, hideNonCombatCrosshair)
	VALUE_FROM_JSON(obj, hideCrosshairMeleeCombat)
	VALUE_FROM_JSON(obj, enableCrosshairSizeManip)
	//VALUE_FROM_JSON(obj, crosshairNPCHitGrowSize)
	VALUE_FROM_JSON(obj, crosshairMinDistSize)
	VALUE_FROM_JSON(obj, crosshairMaxDistSize)
	VALUE_FROM_JSON(obj, useWorldCrosshair)
	VALUE_FROM_JSON(obj, worldCrosshairDepthTest)
	VALUE_FROM_JSON(obj, worldCrosshairType)

	// Arrow prediction
	VALUE_FROM_JSON(obj, useArrowPrediction)
	VALUE_FROM_JSON(obj, drawArrowArc)
	VALUE_FROM_JSON(obj, arrowArcColor)
	VALUE_FROM_JSON(obj, maxArrowPredictionRange)

	// Misc
	VALUE_FROM_JSON(obj, disableDeltaTime)
	VALUE_FROM_JSON(obj, shoulderSwapKey)
	VALUE_FROM_JSON(obj, swapXClamping)

	// Compat
	VALUE_FROM_JSON(obj, disableDuringDialog)
	VALUE_FROM_JSON(obj, comaptIC_FirstPersonHorse)
	VALUE_FROM_JSON(obj, comaptIC_FirstPersonDragon)
	VALUE_FROM_JSON(obj, compatIC_FirstPersonSitting)
	VALUE_FROM_JSON(obj, compatIFPV)

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

	wchar_t path[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path))) {
		_WARNING("Failed to locate My Documents folder, using defualt game config values.");
	} else {
		wchar_t buf[16];
		const auto inipath = std::wstring(path) + L"\\My Games\\Skyrim Special Edition\\Skyrim.ini";
		if (GetPrivateProfileString(L"Combat", L"f3PArrowTiltUpAngle", L"2.5", buf, 16, inipath.c_str()) != 0) {
			wchar_t* end;
			gameConfig.f3PArrowTiltUpAngle = std::wcstof(buf, &end);
		}

		if (GetPrivateProfileString(L"Combat", L"f3PBoltTiltUpAngle", L"2.5", buf, 16, inipath.c_str()) != 0) {
			wchar_t* end;
			gameConfig.f3PBoltTiltUpAngle = std::wcstof(buf, &end);
		}

		if (GetPrivateProfileString(L"Display", L"fNearDistance", L"15.0", buf, 16, inipath.c_str()) != 0) {
			wchar_t* end;
			gameConfig.fNearDistance = std::wcstof(buf, &end);
		}

		if (GetPrivateProfileString(L"Camera", L"fMinCurrentZoom", L"-0.200000003", buf, 16, inipath.c_str()) != 0) {
			wchar_t* end;
			gameConfig.fMinCurrentZoom = std::wcstof(buf, &end);
		}
	}

	// Load bone data
	LoadBonePriorities();

	currentConfig = cfg;
}

void Config::SaveCurrentConfig() {
	std::ofstream os(L"Data/SKSE/Plugins/SmoothCam.json");
	Config::json j = currentConfig;
	os << j << std::endl;
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

	std::ofstream os(GetPresetPath(slot));
	Config::json j = p;
	os << j << std::endl;

	return { "" };
}

bool Config::LoadPreset(int slot) {
	if (slot >= MaxPresetSlots) return false;
	
	Preset p;
	std::ifstream is(GetPresetPath(slot));
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

Config::LoadStatus Config::LoadPresetName(int slot, std::string& name) {
	if (slot >= MaxPresetSlots) return LoadStatus::FAILED;

	Preset p;
	std::ifstream is(GetPresetPath(slot));
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

	name = { p.name };
	return LoadStatus::OK;
}

BSFixedString Config::GetPresetSlotName(int slot) {
	if (slot >= MaxPresetSlots)
		return { "ERROR: Preset index out of range" };

	std::string userName;
	const auto code = LoadPresetName(slot, userName);
	if (code == LoadStatus::OK)
		return { userName.c_str() };
	else if (code == LoadStatus::FAILED)
		return { "Error loading file" };
	else
		return { "Empty" };
}

std::wstring Config::GetPresetPath(int slot) {
	std::wstring slotName(L"Data/SKSE/Plugins/SmoothCamPreset");
	slotName.append(std::to_wstring(slot));
	slotName.append(L".json");
	return slotName;
}

const Config::GameConfig* const Config::GetGameConfig() {
	return &gameConfig;
}

void trimString(std::string& outStr) {
	while(outStr.size() && isspace(outStr.front())) 
		outStr.erase(outStr.begin());

	while(outStr.size() && isspace(outStr.back())) 
		outStr.pop_back();
}

static Config::BoneList bonePriorities = {};
void Config::LoadBonePriorities() {
	const std::string search = "SmoothCam_FollowBones_";
	for (const auto& v : std::filesystem::directory_iterator("Data/SKSE/Plugins")) {
		if (!v.is_regular_file()) continue;
		const auto name = v.path().filename().string();

		if (name.length() < search.length()) continue;
		if (search.compare(0, search.length(), name.substr(0, search.length())) == 0) {
			std::string path = "Data/SKSE/Plugins/";
			path.append(name);

			std::ifstream ifs(path);
			if (!ifs.good()) continue;

			std::string line;
			while (std::getline(ifs, line)) {
				trimString(line);
				if (line.length() == 0) continue;
				if (line.rfind("//", 0) == 0) continue;

				bonePriorities.emplace_back(line.c_str());
			}
		}
	}
}

Config::BoneList& Config::GetBonePriorities() noexcept {
	return bonePriorities;
}