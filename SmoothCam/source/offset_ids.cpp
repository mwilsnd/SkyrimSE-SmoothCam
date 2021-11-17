#include "offset_ids.h"

Offsets::Offsets() noexcept {
#ifdef IS_SKYRIM_AE
		populateAE();
#else
		populateSE();
#endif
}

#ifdef IS_SKYRIM_AE
void Offsets::populateAE() noexcept {
	const auto base = REL::Module::get().base();

	RenderShutdown = base + AE.RenderShutdown;
	RenderStartup = base + AE.RenderStartup;
	FactorCameraOffset = base + AE.FactorCameraOffset;
	FUN_140c69b20 = base + AE.FUN_140c69b20;
	FUN_140c69bd0_MakeNiSomething = base + AE.FUN_140c69bd0_MakeNiSomething;
	GetObjectByName = base + AE.GetObjectByName;
	DoPOVSwitch = base + AE.DoPOVSwitch;
	DoPovSwitchFPV = base + AE.DoPovSwitchFPV;
	vtable_ThirdPersonState_1 = base + AE.vtable_ThirdPersonState_1;
	vtable_HorseCameraState_1 = base + AE.vtable_HorseCameraState_1;
	vtable_DragonCameraState_1 = base + AE.vtable_DragonCameraState_1;
	vtable_BleedoutCameraState_1 = base + AE.vtable_BleedoutCameraState_1;
	vtable_FirstPersonState_1 = base + AE.vtable_FirstPersonState_1;
	vtable_FreeCameraState_1 = base + AE.vtable_FreeCameraState_1;

	S2 = base + AE.S2;
	Power = base + AE.Power;
	RWLocker = base + AE.RWLocker;
	DAT_142eff7d8 = base + AE.DAT_142eff7d8;
	UnkData = base + AE.UnkData;
	UpdateFlightPath = base + AE.UpdateFlightPath;
	DebugTraceProjectile = base + AE.DebugTraceProjectile;
	DebugSpawnProjectile = base + AE.DebugSpawnProjectile;

	ComputeToScreenMatrix = base + AE.ComputeToScreenMatrix;

	GFxGotoAndStop = base + AE.GFxGotoAndStop;
	GFxInvoke = base + AE.GFxInvoke;
	DAT_141de0da8 = base + AE.DAT_141de0da8;
	DAT_141de0dc0 = base + AE.DAT_141de0dc0;
	DAT_142f01438 = base + AE.DAT_142f01438;
	DAT_141de0df0 = base + AE.DAT_141de0df0;
	DAT_141de0e08 = base + AE.DAT_141de0e08;

	SwitchSkeleton = base + AE.SwitchSkeleton;
	RenderStuff = base + AE.RenderStuff;

	IsOverEncumbered = base + AE.IsOverEncumbered;

	CameraCaster = base + AE.CameraCaster;
	GetNiAVObject = base + AE.GetNiAVObject;

	FOVOffset = base + AE.FOVOffset;
	SwitchToFPV = base + AE.SwitchToFPV;
	UpdateFreeCamTransform = base + AE.UpdateFreeCamTransform;

	FOV = base + AE.FOV;
	D3DObjects = base + AE.D3DObjects;
	CameraSwap = base + AE.CameraSwap;
	UpdateGPUCameraData = base + AE.UpdateGPUCameraData;
	CBuffer12 = base + AE.CBuffer12;
	GBuffer = base + AE.GBuffer;
}
#else
void Offsets::populateSE() noexcept {
	RenderShutdown = REL::ID(SE.RenderShutdown).address();
	RenderStartup = REL::ID(SE.RenderStartup).address();
	FactorCameraOffset = REL::ID(SE.FactorCameraOffset).address();
	FUN_140c69b20 = REL::ID(SE.FUN_140c69b20).address();
	FUN_140c69bd0_MakeNiSomething = REL::ID(SE.FUN_140c69bd0_MakeNiSomething).address();
	GetObjectByName = REL::ID(SE.GetObjectByName).address();
	DoPOVSwitch = REL::ID(SE.DoPOVSwitch).address();
	DoPovSwitchFPV = REL::ID(SE.DoPovSwitchFPV).address();
	vtable_ThirdPersonState_1 = REL::ID(SE.vtable_ThirdPersonState_1).address();
	vtable_HorseCameraState_1 = REL::ID(SE.vtable_HorseCameraState_1).address();
	vtable_DragonCameraState_1 = REL::ID(SE.vtable_DragonCameraState_1).address();
	vtable_BleedoutCameraState_1 = REL::ID(SE.vtable_BleedoutCameraState_1).address();
	vtable_FirstPersonState_1 = REL::ID(SE.vtable_FirstPersonState_1).address();
	vtable_FreeCameraState_1 = REL::ID(SE.vtable_FreeCameraState_1).address();

	S2 = REL::ID(SE.S2).address();
	Power = REL::ID(SE.Power).address();
	RWLocker = REL::ID(SE.RWLocker).address();
	DAT_142eff7d8 = REL::ID(SE.DAT_142eff7d8).address();
	UnkData = REL::ID(SE.UnkData).address();
	UpdateFlightPath = REL::ID(SE.UpdateFlightPath).address();
	DebugTraceProjectile = REL::ID(SE.DebugTraceProjectile).address();
	DebugSpawnProjectile = REL::ID(SE.DebugSpawnProjectile).address();

	ComputeToScreenMatrix = REL::ID(SE.ComputeToScreenMatrix).address();

	GFxGotoAndStop = REL::ID(SE.GFxGotoAndStop).address();
	GFxInvoke = REL::ID(SE.GFxInvoke).address();
	DAT_141de0da8 = REL::ID(SE.DAT_141de0da8).address();
	DAT_141de0dc0 = REL::ID(SE.DAT_141de0dc0).address();
	DAT_142f01438 = REL::ID(SE.DAT_142f01438).address();
	DAT_141de0df0 = REL::ID(SE.DAT_141de0df0).address();
	DAT_141de0e08 = REL::ID(SE.DAT_141de0e08).address();

	SwitchSkeleton = REL::ID(SE.SwitchSkeleton).address();
	RenderStuff = REL::ID(SE.RenderStuff).address();

	IsOverEncumbered = REL::ID(SE.IsOverEncumbered).address();

	CameraCaster = REL::ID(SE.CameraCaster).address();
	GetNiAVObject = REL::ID(SE.GetNiAVObject).address();

	FOVOffset = REL::ID(SE.FOVOffset).address();
	SwitchToFPV = REL::ID(SE.SwitchToFPV).address();
	UpdateFreeCamTransform = REL::ID(SE.UpdateFreeCamTransform).address();

	FOV = REL::ID(SE.FOV).address();
	D3DObjects = REL::ID(SE.D3DObjects).address();
	CameraSwap = REL::ID(SE.CameraSwap).address();
	UpdateGPUCameraData = REL::ID(SE.UpdateGPUCameraData).address();
	CBuffer12 = REL::ID(SE.CBuffer12).address();
	GBuffer = REL::ID(SE.GBuffer).address();
}
#endif