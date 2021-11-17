#pragma once

namespace SkyrimSE {
	class HorseCameraState : public RE::ThirdPersonState {
		public:
			HorseCameraState();
			virtual ~HorseCameraState();

			uint32_t horseRefHandle;	// E8
			float horseYaw;				// EC
			uint8_t	unkF0;				// F0
			uint32_t unk1;
			uint16_t unk2;
			uint8_t unk33;
	};
}