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
			uintptr_t RenderShutdown = 77227;
			uintptr_t RenderStartup = 77649;
			uintptr_t FactorCameraOffset = 50799;
			uintptr_t FUN_140c69b20 = 70734;
			uintptr_t FUN_140c69bd0_MakeNiSomething = 70735;
			uintptr_t GetObjectByName = 70299;
			uintptr_t DoPOVSwitch = 50906;
			uintptr_t DoPovSwitchFPV = 50730;
			uintptr_t vtable_ThirdPersonState_1 = 205238;
			uintptr_t vtable_HorseCameraState_1 = 214839;
			uintptr_t vtable_DragonCameraState_1 = 205242;
			uintptr_t vtable_BleedoutCameraState_1 = 214875;
			uintptr_t vtable_FirstPersonState_1 = 214857;
			uintptr_t vtable_FreeCameraState_1 = 214865;

			// ArrowFixes
			uintptr_t S2 = 43700;
			uintptr_t Power = 43699;
			uintptr_t RWLocker = 12332;
			uintptr_t DAT_142eff7d8 = 401069;
			uintptr_t UnkData = 400883;
			uintptr_t UpdateFlightPath = 44184;
			uintptr_t DebugTraceProjectile = 44199;
			uintptr_t DebugSpawnProjectile = 44108;

			// Camera
			uintptr_t ComputeToScreenMatrix = 70641;

			// Crosshair
			uintptr_t GFxGotoAndStop = 82253;
			uintptr_t GFxInvoke = 82256;
			uintptr_t DAT_141de0da8 = 374223;
			uintptr_t DAT_141de0dc0 = 374226;
			uintptr_t DAT_142f01438 = 401668;
			uintptr_t DAT_141de0df0 = 374232;
			uintptr_t DAT_141de0e08 = 374235;

			// Firstperson
			uintptr_t SwitchSkeleton = 40476;
			uintptr_t RenderStuff = 107644;

			// GameState
			uintptr_t IsOverEncumbered = 37453;

			// Raycast
			uintptr_t CameraCaster = 33007;
			uintptr_t GetNiAVObject = 77988;

			// Thirdperson
			uintptr_t FOVOffset = 414942;
			uintptr_t SwitchToFPV = 50790;
			uintptr_t UpdateFreeCamTransform = 50746;

			// Render
			uintptr_t FOV = 388785;
			uintptr_t D3DObjects = 411347;
			uintptr_t CameraSwap = 0;
			uintptr_t UpdateGPUCameraData = 0;
			uintptr_t CBuffer12 = 0;
			uintptr_t GBuffer = 411479;
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