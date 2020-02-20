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
		DisableDeltaTime,
		Crosshair3DEnabled,
		HideCrosshairOutOfCombat, HideCrosshairMeleeCombat,
		DisableDuringDialog,
		MinFollowDistance, MinCameraFollowRate, MaxCameraFollowRate, MaxSmoothingInterpDistance,
		ZoomMul,
		SepZInterpEnabled, SepZMaxInterpDistance, SepZMinFollowRate, SepZMaxFollowRate, SepZInterpMethod,

		// Offsets
		StandingSideOffset, StandingUpOffset, StandingCombatRangedSideOffset, StandingCombatRangedUpOffset,
		StandingCombatMagicSideOffset, StandingCombatMagicUpOffset, StandingCombatMeleeSideOffset, StandingCombatMeleeUpOffset,

		WalkingSideOffset, WalkingUpOffset, WalkingCombatRangedSideOffset, WalkingCombatRangedUpOffset,
		WalkingCombatMagicSideOffset, WalkingCombatMagicUpOffset, WalkingCombatMeleeSideOffset, WalkingCombatMeleeUpOffset,

		RunningSideOffset, RunningUpOffset, RunningCombatRangedSideOffset, RunningCombatRangedUpOffset,
		RunningCombatMagicSideOffset, RunningCombatMagicUpOffset, RunningCombatMeleeSideOffset, RunningCombatMeleeUpOffset,

		SprintingSideOffest, SprintingUpOffset, SprintingCombatRangedSideOffset, SprintingCombatRangedUpOffset,
		SprintingCombatMagicSideOffset, SprintingCombatMagicUpOffset, SprintingCombatMeleeSideOffset, SprintingCombatMeleeUpOffset,

		SneakingSideOffset, SneakingUpOffset, SneakingCombatRangedSideOffset, SneakingCombatRangedUpOffset,
		SneakingCombatMagicSideOffset, SneakingCombatMagicUpOffset, SneakingCombatMeleeSideOffset, SneakingCombatMeleeUpOffset,

		SwimmingSideOffset, SwimmingUpOffset,
		BowAimSideOffset, BowAimUpOffset, BowAimHorseSideOffset, BowAimHorseUpOffset,
		SittingSideOffset, SittingUpOffset,

		HorseSideOffset, HorseUpOffset, HorseCombatRangedSideOffset, HorseCombatRangedUpOffset,
		HorseCombatMagicSideOffset, HorseCombatMagicUpOffset, HorseCombatMeleeSideOffset, HorseCombatMeleeUpOffset,
		
		DragonSideOffset, DragonUpOffset,
		
		// Per state interp switches
		InterpStanding, InterpStandingRangedCombat, InterpStandingMagicCombat, InterpStandingMeleeCombat,
		InterpWalking, InterpWalkingRangedCombat, InterpWalkingMagicCombat, InterpWalkingMeleeCombat,
		InterpRunning, InterpRunningRangedCombat, InterpRunningMagicCombat, InterpRunningMeleeCombat,
		InterpSprinting, InterpSprintingRangedCombat, InterpSprintingMagicCombat, InterpSprintingMeleeCombat,
		InterpSneaking, InterpSneakingRangedCombat, InterpSneakingMagicCombat, InterpSneakingMeleeCombat,
		InterpSwimming,
		InterpBowAim, InterpBowAimHorseback,
		InterpSitting,
		InterpHorseback, InterpHorsebackRangedCombat, InterpHorsebackMagicCombat, InterpHorsebackMeleeCombat,

		INVALID,
	};

	typedef struct stringMapKey {
		ConfigStringMapping mapping;
		ConfigType type;
	} ConfigVar;

	constexpr auto stringMap = mapbox::eternal::hash_map<mapbox::eternal::string, ConfigVar>({
		{ "FirstPersonHorse",					{ConfigStringMapping::FirstPersonHorse, ConfigType::BOOL} },
		{ "FirstPersonDragon",					{ConfigStringMapping::FirstPersonDragon, ConfigType::BOOL} },
		{ "FirstPersonSitting",					{ConfigStringMapping::FirstPersonSitting, ConfigType::BOOL} },

		{ "InterpolationEnabled",				{ConfigStringMapping::InterpolationEnabled, ConfigType::BOOL} },
		{ "InterpolationMethod",				{ConfigStringMapping::InterpolationMethod, ConfigType::STRING} },
		{ "DisableDeltaTime",					{ConfigStringMapping::DisableDeltaTime, ConfigType::BOOL} },
		{ "DisableDuringDialog",				{ConfigStringMapping::DisableDuringDialog, ConfigType::BOOL} },

		{ "SeparateZInterpEnabled",				{ConfigStringMapping::SepZInterpEnabled, ConfigType::BOOL} },
		{ "SepZMaxInterpDistance",				{ConfigStringMapping::SepZMaxInterpDistance, ConfigType::FLOAT} },
		{ "SepZMinFollowRate",					{ConfigStringMapping::SepZMinFollowRate, ConfigType::FLOAT} },
		{ "SepZMaxFollowRate",					{ConfigStringMapping::SepZMaxFollowRate, ConfigType::FLOAT} },
		{ "SeparateZInterpMethod",				{ConfigStringMapping::SepZInterpMethod, ConfigType::STRING} },

		{ "Enable3DCrosshair",					{ConfigStringMapping::Crosshair3DEnabled, ConfigType::BOOL} },
		{ "HideCrosshairOutOfCombat",			{ConfigStringMapping::HideCrosshairOutOfCombat, ConfigType::BOOL} },
		{ "HideCrosshairMeleeCombat",			{ConfigStringMapping::HideCrosshairMeleeCombat, ConfigType::BOOL} },

		{ "MinFollowDistance",					{ConfigStringMapping::MinFollowDistance, ConfigType::FLOAT} },
		{ "MinCameraFollowRate",				{ConfigStringMapping::MinCameraFollowRate, ConfigType::FLOAT} },
		{ "MaxCameraFollowRate",				{ConfigStringMapping::MaxCameraFollowRate, ConfigType::FLOAT} },
		{ "MaxSmoothingInterpDistance",			{ConfigStringMapping::MaxSmoothingInterpDistance, ConfigType::FLOAT} },
		{ "ZoomMul",							{ConfigStringMapping::ZoomMul, ConfigType::FLOAT} },

		{ "Standing:SideOffset",				{ConfigStringMapping::StandingSideOffset, ConfigType::FLOAT} },
		{ "Standing:UpOffset",					{ConfigStringMapping::StandingUpOffset, ConfigType::FLOAT} },
		{ "StandingCombat:Ranged:SideOffset",	{ConfigStringMapping::StandingCombatRangedSideOffset, ConfigType::FLOAT} },
		{ "StandingCombat:Ranged:UpOffset",		{ConfigStringMapping::StandingCombatRangedUpOffset, ConfigType::FLOAT} },
		{ "StandingCombat:Magic:SideOffset",	{ConfigStringMapping::StandingCombatMagicSideOffset, ConfigType::FLOAT} },
		{ "StandingCombat:Magic:UpOffset",		{ConfigStringMapping::StandingCombatMagicUpOffset, ConfigType::FLOAT} },
		{ "StandingCombat:Melee:SideOffset",	{ConfigStringMapping::StandingCombatMeleeSideOffset, ConfigType::FLOAT} },
		{ "StandingCombat:Melee:UpOffset",		{ConfigStringMapping::StandingCombatMeleeUpOffset, ConfigType::FLOAT} },

		{ "Walking:SideOffset",					{ConfigStringMapping::WalkingSideOffset, ConfigType::FLOAT} },
		{ "Walking:UpOffset",					{ConfigStringMapping::WalkingUpOffset, ConfigType::FLOAT} },
		{ "WalkingCombat:Ranged:SideOffset",	{ConfigStringMapping::WalkingCombatRangedSideOffset, ConfigType::FLOAT} },
		{ "WalkingCombat:Ranged:UpOffset",		{ConfigStringMapping::WalkingCombatRangedUpOffset, ConfigType::FLOAT} },
		{ "WalkingCombat:Magic:SideOffset",		{ConfigStringMapping::WalkingCombatMagicSideOffset, ConfigType::FLOAT} },
		{ "WalkingCombat:Magic:UpOffset",		{ConfigStringMapping::WalkingCombatMagicUpOffset, ConfigType::FLOAT} },
		{ "WalkingCombat:Melee:SideOffset",		{ConfigStringMapping::WalkingCombatMeleeSideOffset, ConfigType::FLOAT} },
		{ "WalkingCombat:Melee:UpOffset",		{ConfigStringMapping::WalkingCombatMeleeUpOffset, ConfigType::FLOAT} },

		{ "Running:SideOffset",					{ConfigStringMapping::RunningSideOffset, ConfigType::FLOAT} },
		{ "Running:UpOffset",					{ConfigStringMapping::RunningUpOffset, ConfigType::FLOAT} },
		{ "RunningCombat:Ranged:SideOffset",	{ConfigStringMapping::RunningCombatRangedSideOffset, ConfigType::FLOAT} },
		{ "RunningCombat:Ranged:UpOffset",		{ConfigStringMapping::RunningCombatRangedUpOffset, ConfigType::FLOAT} },
		{ "RunningCombat:Magic:SideOffset",		{ConfigStringMapping::RunningCombatMagicSideOffset, ConfigType::FLOAT} },
		{ "RunningCombat:Magic:UpOffset",		{ConfigStringMapping::RunningCombatMagicUpOffset, ConfigType::FLOAT} },
		{ "RunningCombat:Melee:SideOffset",		{ConfigStringMapping::RunningCombatMeleeSideOffset, ConfigType::FLOAT} },
		{ "RunningCombat:Melee:UpOffset",		{ConfigStringMapping::RunningCombatMeleeUpOffset, ConfigType::FLOAT} },

		{ "Sprinting:SideOffset",				{ConfigStringMapping::SprintingSideOffest, ConfigType::FLOAT} },
		{ "Sprinting:UpOffset",					{ConfigStringMapping::SprintingUpOffset, ConfigType::FLOAT} },
		{ "SprintingCombat:Ranged:SideOffset",	{ConfigStringMapping::SprintingCombatRangedSideOffset, ConfigType::FLOAT} },
		{ "SprintingCombat:Ranged:UpOffset",	{ConfigStringMapping::SprintingCombatRangedUpOffset, ConfigType::FLOAT} },
		{ "SprintingCombat:Magic:SideOffset",	{ConfigStringMapping::SprintingCombatMagicSideOffset, ConfigType::FLOAT} },
		{ "SprintingCombat:Magic:UpOffset",		{ConfigStringMapping::SprintingCombatMagicUpOffset, ConfigType::FLOAT} },
		{ "SprintingCombat:Melee:SideOffset",	{ConfigStringMapping::SprintingCombatMeleeSideOffset, ConfigType::FLOAT} },
		{ "SprintingCombat:Melee:UpOffset",		{ConfigStringMapping::SprintingCombatMeleeUpOffset, ConfigType::FLOAT} },

		{ "Sneaking:SideOffset",				{ConfigStringMapping::SneakingSideOffset, ConfigType::FLOAT} },
		{ "Sneaking:UpOffset",					{ConfigStringMapping::SneakingUpOffset, ConfigType::FLOAT} },
		{ "SneakingCombat:Ranged:SideOffset",	{ConfigStringMapping::SneakingCombatRangedSideOffset, ConfigType::FLOAT} },
		{ "SneakingCombat:Ranged:UpOffset",		{ConfigStringMapping::SneakingCombatRangedUpOffset, ConfigType::FLOAT} },
		{ "SneakingCombat:Magic:SideOffset",	{ConfigStringMapping::SneakingCombatMagicSideOffset, ConfigType::FLOAT} },
		{ "SneakingCombat:Magic:UpOffset",		{ConfigStringMapping::SneakingCombatMagicUpOffset, ConfigType::FLOAT} },
		{ "SneakingCombat:Melee:SideOffset",	{ConfigStringMapping::SneakingCombatMeleeSideOffset, ConfigType::FLOAT} },
		{ "SneakingCombat:Melee:UpOffset",		{ConfigStringMapping::SneakingCombatMeleeUpOffset, ConfigType::FLOAT} },

		{ "Swimming:SideOffset",				{ConfigStringMapping::SwimmingSideOffset, ConfigType::FLOAT} },
		{ "Swimming:UpOffset",					{ConfigStringMapping::SwimmingUpOffset, ConfigType::FLOAT} },

		{ "Bowaim:SideOffset",					{ConfigStringMapping::BowAimSideOffset, ConfigType::FLOAT} },
		{ "Bowaim:UpOffset",					{ConfigStringMapping::BowAimUpOffset, ConfigType::FLOAT} },
		{ "BowaimHorse:SideOffset",				{ConfigStringMapping::BowAimHorseSideOffset, ConfigType::FLOAT} },
		{ "BowaimHorse:UpOffset",				{ConfigStringMapping::BowAimHorseUpOffset, ConfigType::FLOAT} },

		{ "Sitting:SideOffset",					{ConfigStringMapping::SittingSideOffset, ConfigType::FLOAT} },
		{ "Sitting:UpOffset",					{ConfigStringMapping::SittingUpOffset, ConfigType::FLOAT} },

		{ "Horseback:SideOffset",				{ConfigStringMapping::HorseSideOffset, ConfigType::FLOAT} },
		{ "Horseback:UpOffset",					{ConfigStringMapping::HorseUpOffset, ConfigType::FLOAT} },
		{ "HorsebackCombat:Ranged:SideOffset",	{ConfigStringMapping::HorseCombatRangedSideOffset, ConfigType::FLOAT} },
		{ "HorsebackCombat:Ranged:UpOffset",	{ConfigStringMapping::HorseCombatRangedUpOffset, ConfigType::FLOAT} },
		{ "HorsebackCombat:Magic:SideOffset",	{ConfigStringMapping::HorseCombatMagicSideOffset, ConfigType::FLOAT} },
		{ "HorsebackCombat:Magic:UpOffset",		{ConfigStringMapping::HorseCombatMagicUpOffset, ConfigType::FLOAT} },
		{ "HorsebackCombat:Melee:SideOffset",	{ConfigStringMapping::HorseCombatMeleeSideOffset, ConfigType::FLOAT} },
		{ "HorsebackCombat:Melee:UpOffset",		{ConfigStringMapping::HorseCombatMeleeUpOffset, ConfigType::FLOAT} },

		{ "Dragon:SideOffset",					{ConfigStringMapping::DragonSideOffset, ConfigType::FLOAT} },
		{ "Dragon:UpOffset",					{ConfigStringMapping::DragonUpOffset, ConfigType::FLOAT} },

		{ "InterpStanding",						{ConfigStringMapping::InterpStanding, ConfigType::BOOL} },
		{ "InterpStandingRangedCombat",			{ConfigStringMapping::InterpStandingRangedCombat, ConfigType::BOOL} },
		{ "InterpStandingMagicCombat",			{ConfigStringMapping::InterpStandingMagicCombat, ConfigType::BOOL} },
		{ "InterpStandingMeleeCombat",			{ConfigStringMapping::InterpStandingMeleeCombat, ConfigType::BOOL} },
		{ "InterpWalking",						{ConfigStringMapping::InterpWalking, ConfigType::BOOL} },
		{ "InterpWalkingRangedCombat",			{ConfigStringMapping::InterpWalkingRangedCombat, ConfigType::BOOL} },
		{ "InterpWalkingMagicCombat",			{ConfigStringMapping::InterpWalkingMagicCombat, ConfigType::BOOL} },
		{ "InterpWalkingMeleeCombat",			{ConfigStringMapping::InterpWalkingMeleeCombat, ConfigType::BOOL} },
		{ "InterpRunning",						{ConfigStringMapping::InterpRunning, ConfigType::BOOL} },
		{ "InterpRunningRangedCombat",			{ConfigStringMapping::InterpRunningRangedCombat, ConfigType::BOOL} },
		{ "InterpRunningMagicCombat",			{ConfigStringMapping::InterpRunningMagicCombat, ConfigType::BOOL} },
		{ "InterpRunningMeleeCombat",			{ConfigStringMapping::InterpRunningMeleeCombat, ConfigType::BOOL} },
		{ "InterpSprinting",					{ConfigStringMapping::InterpSprinting, ConfigType::BOOL} },
		{ "InterpSprintingRangedCombat",		{ConfigStringMapping::InterpSprintingRangedCombat, ConfigType::BOOL} },
		{ "InterpSprintingMagicCombat",			{ConfigStringMapping::InterpSprintingMagicCombat, ConfigType::BOOL} },
		{ "InterpSprintingMeleeCombat",			{ConfigStringMapping::InterpSprintingMeleeCombat, ConfigType::BOOL} },
		{ "InterpSneaking",						{ConfigStringMapping::InterpSneaking, ConfigType::BOOL} },
		{ "InterpSneakingRangedCombat",			{ConfigStringMapping::InterpSneakingRangedCombat, ConfigType::BOOL} },
		{ "InterpSneakingMagicCombat",			{ConfigStringMapping::InterpSneakingMagicCombat, ConfigType::BOOL} },
		{ "InterpSneakingMeleeCombat",			{ConfigStringMapping::InterpSneakingMeleeCombat, ConfigType::BOOL} },
		{ "InterpSwimming",						{ConfigStringMapping::InterpSwimming, ConfigType::BOOL} },
		{ "InterpBowAim",						{ConfigStringMapping::InterpBowAim, ConfigType::BOOL} },
		{ "InterpBowAimHorseback",				{ConfigStringMapping::InterpBowAimHorseback, ConfigType::BOOL} },
		{ "InterpSitting",						{ConfigStringMapping::InterpSitting, ConfigType::BOOL} },
		{ "InterpHorseback",					{ConfigStringMapping::InterpHorseback, ConfigType::BOOL} },
		{ "InterpHorsebackRangedCombat",		{ConfigStringMapping::InterpHorsebackRangedCombat, ConfigType::BOOL} },
		{ "InterpHorsebackMagicCombat",			{ConfigStringMapping::InterpHorsebackMagicCombat, ConfigType::BOOL} },
		{ "InterpHorsebackMeleeCombat",			{ConfigStringMapping::InterpHorsebackMeleeCombat, ConfigType::BOOL} },
	});

	ConfigVar GetInfo(const BSFixedString& member);
	void Bind(VMClassRegistry* registry);
}