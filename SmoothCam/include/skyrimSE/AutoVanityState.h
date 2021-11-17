#pragma once

namespace SkyrimSE {
	class AutoVanityState : public RE::TESCameraState {
		public:
			AutoVanityState();
			virtual ~AutoVanityState();

			float yaw; // 20
			uint32_t pad24; // 24
		};
}