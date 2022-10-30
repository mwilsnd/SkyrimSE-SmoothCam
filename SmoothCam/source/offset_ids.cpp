#include "offset_ids.h"

Offsets::Offsets() noexcept {
#ifdef SKYRIM_SUPPORT_AE
		populateAE();
#else
		populateSE();
#endif

#if defined(SKYRIM_SUPPORT_AE) && !defined(SKYRIM_IS_PRE629)
	menuHookOffset = 0x2B8;
#else
	menuHookOffset = 0x2B0;
#endif
}

#ifdef SKYRIM_SUPPORT_AE
void Offsets::populateAE() noexcept {
	RenderShutdown = REL::ID(AE.RenderShutdown).address();
	RenderStartup = REL::ID(AE.RenderStartup).address();
	FactorCameraOffset = REL::ID(AE.FactorCameraOffset).address();
	FUN_140c69b20 = REL::ID(AE.FUN_140c69b20).address();
	FUN_140c69bd0_MakeNiSomething = REL::ID(AE.FUN_140c69bd0_MakeNiSomething).address();
	GetObjectByName = REL::ID(AE.GetObjectByName).address();
	DoPOVSwitch = REL::ID(AE.DoPOVSwitch).address();
	DoPovSwitchFPV = REL::ID(AE.DoPovSwitchFPV).address();
	vtable_ThirdPersonState_1 = REL::ID(AE.vtable_ThirdPersonState_1).address();
	vtable_HorseCameraState_1 = REL::ID(AE.vtable_HorseCameraState_1).address();
	vtable_DragonCameraState_1 = REL::ID(AE.vtable_DragonCameraState_1).address();
	vtable_BleedoutCameraState_1 = REL::ID(AE.vtable_BleedoutCameraState_1).address();
	vtable_FirstPersonState_1 = REL::ID(AE.vtable_FirstPersonState_1).address();
	vtable_FreeCameraState_1 = REL::ID(AE.vtable_FreeCameraState_1).address();

	S2 = REL::ID(AE.S2).address();
	Power = REL::ID(AE.Power).address();
	RWLocker = REL::ID(AE.RWLocker).address();
	DAT_142eff7d8 = REL::ID(AE.DAT_142eff7d8).address();
	UnkData = REL::ID(AE.UnkData).address();
	UpdateFlightPath = REL::ID(AE.UpdateFlightPath).address();
	DebugTraceProjectile = REL::ID(AE.DebugTraceProjectile).address();
	DebugSpawnProjectile = REL::ID(AE.DebugSpawnProjectile).address();

	ComputeToScreenMatrix = REL::ID(AE.ComputeToScreenMatrix).address();

	GFxGotoAndStop = REL::ID(AE.GFxGotoAndStop).address();
	GFxInvoke = REL::ID(AE.GFxInvoke).address();
	DAT_141de0da8 = REL::ID(AE.DAT_141de0da8).address();
	DAT_141de0dc0 = REL::ID(AE.DAT_141de0dc0).address();
	DAT_142f01438 = REL::ID(AE.DAT_142f01438).address();
	DAT_141de0df0 = REL::ID(AE.DAT_141de0df0).address();
	DAT_141de0e08 = REL::ID(AE.DAT_141de0e08).address();
	CrosshairPickerData = REL::ID(AE.CrosshairPickerData).address();
	UpdateCrosshairReference = REL::ID(AE.UpdateCrosshairReference).address();
	CrosshairData_ctor = REL::ID(AE.CrosshairData_ctor).address();
	CrosshairData_dtor = REL::ID(AE.CrosshairData_dtor).address();
	CrosshairData_pick = REL::ID(AE.CrosshairData_pick).address();

	SwitchSkeleton = REL::ID(AE.SwitchSkeleton).address();
	RenderStuff = REL::ID(AE.RenderStuff).address();

	IsOverEncumbered = REL::ID(AE.IsOverEncumbered).address();

	CameraCaster = REL::ID(AE.CameraCaster).address();
	GetNiAVObject = REL::ID(AE.GetNiAVObject).address();

	FOVOffset = REL::ID(AE.FOVOffset).address();
	SwitchToFPV = REL::ID(AE.SwitchToFPV).address();
	UpdateFreeCamTransform = REL::ID(AE.UpdateFreeCamTransform).address();

	FOV = REL::ID(AE.FOV).address();
	D3DObjects = REL::ID(AE.D3DObjects).address();
	CameraSwap = REL::ID(AE.CameraSwap).address();
	UpdateGPUCameraData = REL::ID(AE.UpdateGPUCameraData).address();
	CBuffer12 = REL::ID(AE.CBuffer12).address();
	GBuffer = REL::ID(AE.GBuffer).address();
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
	CrosshairPickerData = REL::ID(SE.CrosshairPickerData).address();
	UpdateCrosshairReference = REL::ID(SE.UpdateCrosshairReference).address();
	CrosshairData_ctor = REL::ID(SE.CrosshairData_ctor).address();
	CrosshairData_dtor = REL::ID(SE.CrosshairData_dtor).address();
	CrosshairData_pick = REL::ID(SE.CrosshairData_pick).address();

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