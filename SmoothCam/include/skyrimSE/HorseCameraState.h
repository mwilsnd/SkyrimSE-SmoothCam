#pragma once
#include "skyrimSE/ThirdPersonState.h"

class CorrectedHorseCameraState : public CorrectedThirdPersonState {
	public:
		CorrectedHorseCameraState();
		virtual ~CorrectedHorseCameraState();

		uint32_t horseRefHandle;	// E8
		float horseYaw;				// EC
		uint8_t	unkF0;				// F0
		uint32_t unk1;
		uint16_t unk2;
		uint8_t unk33;
};