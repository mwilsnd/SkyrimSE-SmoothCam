#pragma once
#include "pch.h"

namespace PapyrusBindings {
	constexpr auto ScriptClassName = "SmoothCamMCM";

	enum class ConfigType {
		STRING,
		BOOL,
		FLOAT,
		INVALID,
	};

	enum class ConfigStringMapping {
		FirstPersonHorse, FirstPersonDragon, FirstPersonSitting,
		InterpolationEnabled, InterpolationMethod,
		Crosshair3DEnabled,
		MinFollowDistance, MinCameraFollowRate, MaxCameraFollowRate, MaxSmoothingInterpDistance,
		ZoomMul,
		StandingSideOffset, StandingUpOffset, StandingCombatSideOffset, StandingCombatUpOffset,
		WalkingSideOffset, WalkingUpOffset, WalkingCombatSideOffset, WalkingCombatUpOffset,
		RunningSideOffset, RunningUpOffset, RunningCombatSideOffset, RunningCombatUpOffset,
		SprintingSideOffest, SprintingUpOffset, SprintingCombatSideOffset, SprintingCombatUpOffset,
		SneakingSideOffset, SneakingUpOffset, SneakingCombatSideOffset, SneakingCombatUpOffset,
		SwimmingSideOffset, SwimmingUpOffset,
		BowAimSideOffset, BowAimUpOffset, BowAimHorseSideOffset, BowAimHorseUpOffset,
		SittingSideOffset, SittingUpOffset,
		HorseSideOffset, HorseUpOffset, DragonSideOffset, DragonUpOffset,
		SepZInterpEnabled, SepZMaxInterpDistance, SepZMinFollowRate, SepZMaxFollowRate, SepZInterpMethod,

		INVALID,
	};

	typedef struct stringMapKey {
		ConfigStringMapping mapping;
		ConfigType type;
	} ConfigVar;

	constexpr auto stringMap = mapbox::eternal::hash_map<mapbox::eternal::string, ConfigVar>({
		{ "FirstPersonHorse",			{ConfigStringMapping::FirstPersonHorse, ConfigType::BOOL} },
		{ "FirstPersonDragon",			{ConfigStringMapping::FirstPersonDragon, ConfigType::BOOL} },
		{ "FirstPersonSitting",			{ConfigStringMapping::FirstPersonSitting, ConfigType::BOOL} },

		{ "InterpolationEnabled",		{ConfigStringMapping::InterpolationEnabled, ConfigType::BOOL} },
		{ "InterpolationMethod",		{ConfigStringMapping::InterpolationMethod, ConfigType::STRING} },

		{ "SeparateZInterpEnabled",		{ConfigStringMapping::SepZInterpEnabled, ConfigType::BOOL} },
		{ "SepZMaxInterpDistance",		{ConfigStringMapping::SepZMaxInterpDistance, ConfigType::FLOAT} },
		{ "SepZMinFollowRate",			{ConfigStringMapping::SepZMinFollowRate, ConfigType::FLOAT} },
		{ "SepZMaxFollowRate",			{ConfigStringMapping::SepZMaxFollowRate, ConfigType::FLOAT} },
		{ "SeparateZInterpMethod",		{ConfigStringMapping::SepZInterpMethod, ConfigType::STRING} },

		{ "Enable3DCrosshair",			{ConfigStringMapping::Crosshair3DEnabled, ConfigType::BOOL} },

		{ "MinFollowDistance",			{ConfigStringMapping::MinFollowDistance, ConfigType::FLOAT} },
		{ "MinCameraFollowRate",		{ConfigStringMapping::MinCameraFollowRate, ConfigType::FLOAT} },
		{ "MaxCameraFollowRate",		{ConfigStringMapping::MaxCameraFollowRate, ConfigType::FLOAT} },
		{ "MaxSmoothingInterpDistance",	{ConfigStringMapping::MaxSmoothingInterpDistance, ConfigType::FLOAT} },
		{ "ZoomMul",					{ConfigStringMapping::ZoomMul, ConfigType::FLOAT} },

		{ "Standing:SideOffset",		{ConfigStringMapping::StandingSideOffset, ConfigType::FLOAT} },
		{ "Standing:UpOffset",			{ConfigStringMapping::StandingUpOffset, ConfigType::FLOAT} },
		{ "StandingCombat:SideOffset",	{ConfigStringMapping::StandingCombatSideOffset, ConfigType::FLOAT} },
		{ "StandingCombat:UpOffset",	{ConfigStringMapping::StandingCombatUpOffset, ConfigType::FLOAT} },

		{ "Walking:SideOffset",			{ConfigStringMapping::WalkingSideOffset, ConfigType::FLOAT} },
		{ "Walking:UpOffset",			{ConfigStringMapping::WalkingUpOffset, ConfigType::FLOAT} },
		{ "WalkingCombat:SideOffset",	{ConfigStringMapping::WalkingCombatSideOffset, ConfigType::FLOAT} },
		{ "WalkingCombat:UpOffset",		{ConfigStringMapping::WalkingCombatUpOffset, ConfigType::FLOAT} },

		{ "Running:SideOffset",			{ConfigStringMapping::RunningSideOffset, ConfigType::FLOAT} },
		{ "Running:UpOffset",			{ConfigStringMapping::RunningUpOffset, ConfigType::FLOAT} },
		{ "RunningCombat:SideOffset",	{ConfigStringMapping::RunningCombatSideOffset, ConfigType::FLOAT} },
		{ "RunningCombat:UpOffset",		{ConfigStringMapping::RunningCombatUpOffset, ConfigType::FLOAT} },

		{ "Sprinting:SideOffset",		{ConfigStringMapping::SprintingSideOffest, ConfigType::FLOAT} },
		{ "Sprinting:UpOffset",			{ConfigStringMapping::SprintingUpOffset, ConfigType::FLOAT} },
		{ "SprintingCombat:SideOffset",	{ConfigStringMapping::SprintingCombatSideOffset, ConfigType::FLOAT} },
		{ "SprintingCombat:UpOffset",	{ConfigStringMapping::SprintingCombatUpOffset, ConfigType::FLOAT} },

		{ "Sneaking:SideOffset",		{ConfigStringMapping::SneakingSideOffset, ConfigType::FLOAT} },
		{ "Sneaking:UpOffset",			{ConfigStringMapping::SneakingUpOffset, ConfigType::FLOAT} },
		{ "SneakingCombat:SideOffset",	{ConfigStringMapping::SneakingCombatSideOffset, ConfigType::FLOAT} },
		{ "SneakingCombat:UpOffset",	{ConfigStringMapping::SprintingCombatUpOffset, ConfigType::FLOAT} },

		{ "Swimming:SideOffset",		{ConfigStringMapping::SwimmingSideOffset, ConfigType::FLOAT} },
		{ "Swimming:UpOffset",			{ConfigStringMapping::SwimmingUpOffset, ConfigType::FLOAT} },

		{ "Bowaim:SideOffset",			{ConfigStringMapping::BowAimSideOffset, ConfigType::FLOAT} },
		{ "Bowaim:UpOffset",			{ConfigStringMapping::BowAimUpOffset, ConfigType::FLOAT} },
		{ "BowaimHorse:SideOffset",		{ConfigStringMapping::BowAimHorseSideOffset, ConfigType::FLOAT} },
		{ "BowaimHorse:UpOffset",		{ConfigStringMapping::BowAimHorseUpOffset, ConfigType::FLOAT} },

		{ "Sitting:SideOffset",			{ConfigStringMapping::SittingSideOffset, ConfigType::FLOAT} },
		{ "Sitting:UpOffset",			{ConfigStringMapping::SittingUpOffset, ConfigType::FLOAT} },

		{ "Horseback:SideOffset",		{ConfigStringMapping::HorseSideOffset, ConfigType::FLOAT} },
		{ "Horseback:UpOffset",			{ConfigStringMapping::HorseUpOffset, ConfigType::FLOAT} },
		{ "Dragon:SideOffset",			{ConfigStringMapping::DragonSideOffset, ConfigType::FLOAT} },
		{ "Dragon:UpOffset",			{ConfigStringMapping::DragonUpOffset, ConfigType::FLOAT} },
	});

	ConfigVar GetInfo(const BSFixedString& member);
	void Bind(VMClassRegistry* registry);
}