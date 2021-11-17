#pragma once

namespace SkyrimSE {
	class FreeCameraState : public RE::TESCameraState {
		public:
			FreeCameraState();
			virtual ~FreeCameraState();

			RE::PlayerInputHandler*		inputHandler;	// 20
			RE::NiPoint3				translate;
			float						unk30[5];		// 30
			uint32_t					unk4C;			// 4C
	};
}