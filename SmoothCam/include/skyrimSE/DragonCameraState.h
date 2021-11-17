#pragma once

namespace SkyrimSE {
	class DragonCameraState : public RE::ThirdPersonState {
		public:
			DragonCameraState();
			virtual ~DragonCameraState();

			uint32_t unkE8;			// 0E8
			uint32_t unkEC;			// 0EC
			uint8_t unkF0;			// 0F0
			uint8_t padF1[3];		// 0F1
			float unkF4;			// 0F4 - init'd to 1
			uint8_t unkF8;			// 0F8
			uint8_t padF9[3];		// 0F9
			float unkFC[4];			// 0FC
			uint32_t unk10C;		// 10C
	};
}