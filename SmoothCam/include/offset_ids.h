#pragma once

class Offsets {
	public:
		static Offsets& Get() {
			static Offsets offsets;
			return offsets;
		}

	private:
		Offsets() noexcept;
#ifdef IS_SKYRIM_AE
		void populateAE() noexcept;
#else
		void populateSE() noexcept;
#endif

	public:
		struct {
			// Detours
			uintptr_t RenderShutdown = 75446;
			uintptr_t RenderStartup = 75827;
			uintptr_t FactorCameraOffset = 49866;
			uintptr_t FUN_140c69b20 = 69354;
			uintptr_t FUN_140c69bd0_MakeNiSomething = 69355;
			uintptr_t GetObjectByName = 68948;
			uintptr_t DoPOVSwitch = 49970;
			uintptr_t DoPovSwitchFPV = 49800;
			uintptr_t vtable_ThirdPersonState_1 = 256648;
			uintptr_t vtable_HorseCameraState_1 = 267750;
			uintptr_t vtable_DragonCameraState_1 = 256650;
			uintptr_t vtable_BleedoutCameraState_1 = 267820;
			uintptr_t vtable_FirstPersonState_1 = 267811;
			uintptr_t vtable_FreeCameraState_1 = 267815;

			// ArrowFixes
			uintptr_t S2 = 42537;
			uintptr_t Power = 42536;
			uintptr_t RWLocker = 12204;
			uintptr_t DAT_142eff7d8 = 514905;
			uintptr_t UnkData = 514725;
			uintptr_t UpdateFlightPath = 42998;
			uintptr_t DebugTraceProjectile = 43008;
			uintptr_t DebugSpawnProjectile = 42928;

			// Camera
			uintptr_t ComputeToScreenMatrix = 69271;

			// Crosshair
			uintptr_t GFxGotoAndStop = 80230;
			uintptr_t GFxInvoke = 80233;
			uintptr_t DAT_141de0da8 = 505064;
			uintptr_t DAT_141de0dc0 = 505066;
			uintptr_t DAT_142f01438 = 515530;
			uintptr_t DAT_141de0df0 = 505070;
			uintptr_t DAT_141de0e08 = 505072;

			// Firstperson
			uintptr_t SwitchSkeleton = 39401;
			uintptr_t RenderStuff = 100854;

			// GameState
			uintptr_t IsOverEncumbered = 36457;

			// Raycast
			uintptr_t CameraCaster = 32270;
			uintptr_t GetNiAVObject = 76160;

			// Thirdperson
			uintptr_t FOVOffset = 527997;
			uintptr_t SwitchToFPV = 49858;
			uintptr_t UpdateFreeCamTransform = 49816;

			// Render
			uintptr_t FOV = 513786;
			uintptr_t D3DObjects = 524728;
			uintptr_t CameraSwap = 75713;
			uintptr_t UpdateGPUCameraData = 75694;
			uintptr_t CBuffer12 = 524768;
			uintptr_t GBuffer = 524998;
		} SE;

		struct {
			// Detours
			uintptr_t RenderShutdown = 0xda4260;
			uintptr_t RenderStartup = 0xdc4f80;
			uintptr_t FactorCameraOffset = 0x8770b0;
			uintptr_t FUN_140c69b20 = 0xc91ff0;
			uintptr_t FUN_140c69bd0_MakeNiSomething = 0xc920a0;
			uintptr_t GetObjectByName = 0xc7f970;
			uintptr_t DoPOVSwitch = 0x87bd00;
			uintptr_t DoPovSwitchFPV = 0x873650;
			uintptr_t vtable_ThirdPersonState_1 = 0x17258e0;
			uintptr_t vtable_HorseCameraState_1 = 0x179c8b0;
			uintptr_t vtable_DragonCameraState_1 = 0x1725998;
			uintptr_t vtable_BleedoutCameraState_1 = 0x179d5a8;
			uintptr_t vtable_FirstPersonState_1 = 0x179d310;
			uintptr_t vtable_FreeCameraState_1 = 0x179d400;

			// ArrowFixes
			uintptr_t S2 = 0x75d1c0;
			uintptr_t Power = 0x75d1a0;
			uintptr_t RWLocker = 0x139160;
			uintptr_t DAT_142eff7d8 = 0x2f99f90;
			uintptr_t UnkData = 0x2f603b0;
			uintptr_t UpdateFlightPath = 0x77d760;
			uintptr_t DebugTraceProjectile = 0x77ecc0;
			uintptr_t DebugSpawnProjectile = 0x778480;

			// Camera
			uintptr_t ComputeToScreenMatrix = 0xc8e8e0;

			// Crosshair
			uintptr_t GFxGotoAndStop = 0xf0ccc0;
			uintptr_t GFxInvoke = 0xf0cfd0;
			uintptr_t DAT_141de0da8 = 0x1e74e88;
			uintptr_t DAT_141de0dc0 = 0x1e74ea0;
			uintptr_t DAT_142f01438 = 0x2f9bb58;
			uintptr_t DAT_141de0df0 = 0x1e74ed0;
			uintptr_t DAT_141de0e08 = 0x1e74ee8;

			// Firstperson
			uintptr_t SwitchSkeleton = 0x6c9f30;
			uintptr_t RenderStuff = 0x142f580;

			// GameState
			uintptr_t IsOverEncumbered = 0x607410;

			// Raycast
			uintptr_t CameraCaster = 0x50d640;
			uintptr_t GetNiAVObject = 0xdecf20;

			// Thirdperson
			uintptr_t FOVOffset = 0x32cd208;
			uintptr_t SwitchToFPV = 0x876e60;
			uintptr_t UpdateFreeCamTransform = 0x874200;

			// Render
			uintptr_t FOV = 0x1e9b420;
			uintptr_t D3DObjects = 0x30C0C08;
			uintptr_t CameraSwap = 0;
			uintptr_t UpdateGPUCameraData = 0;
			uintptr_t CBuffer12 = 0;
			uintptr_t GBuffer = 0x30c6d90;
		} AE;

		// Detours
		uintptr_t RenderShutdown = 0;
		uintptr_t RenderStartup = 0;
		uintptr_t FactorCameraOffset = 0;
		uintptr_t FUN_140c69b20 = 0;
		uintptr_t FUN_140c69bd0_MakeNiSomething = 0;
		uintptr_t GetObjectByName = 0;
		uintptr_t DoPOVSwitch = 0;
		uintptr_t DoPovSwitchFPV = 0;
		uintptr_t vtable_ThirdPersonState_1 = 0;
		uintptr_t vtable_HorseCameraState_1 = 0;
		uintptr_t vtable_DragonCameraState_1 = 0;
		uintptr_t vtable_BleedoutCameraState_1 = 0;
		uintptr_t vtable_FirstPersonState_1 = 0;
		uintptr_t vtable_FreeCameraState_1 = 0;

		// ArrowFixes
		uintptr_t S2 = 0;
		uintptr_t Power = 0;
		uintptr_t RWLocker = 0;
		uintptr_t DAT_142eff7d8 = 0;
		uintptr_t UnkData = 0;
		uintptr_t UpdateFlightPath = 0;
		uintptr_t DebugTraceProjectile = 0;
		uintptr_t DebugSpawnProjectile = 0;

		// Camera
		uintptr_t ComputeToScreenMatrix = 0;

		// Crosshair
		uintptr_t GFxGotoAndStop = 0;
		uintptr_t GFxInvoke = 0;
		uintptr_t DAT_141de0da8 = 0;
		uintptr_t DAT_141de0dc0 = 0;
		uintptr_t DAT_142f01438 = 0;
		uintptr_t DAT_141de0df0 = 0;
		uintptr_t DAT_141de0e08 = 0;

		// Firstperson
		uintptr_t SwitchSkeleton = 0;
		uintptr_t RenderStuff = 0;

		// GameState
		uintptr_t IsOverEncumbered = 0;

		// Raycast
		uintptr_t CameraCaster = 0;
		uintptr_t GetNiAVObject = 0;

		// Thirdperson
		uintptr_t FOVOffset = 0;
		uintptr_t SwitchToFPV = 0;
		uintptr_t UpdateFreeCamTransform = 0;

		// Render
		uintptr_t FOV = 0;
		uintptr_t D3DObjects = 0;
		uintptr_t CameraSwap = 0;
		uintptr_t UpdateGPUCameraData = 0;
		uintptr_t CBuffer12 = 0;
		uintptr_t GBuffer = 0;
};