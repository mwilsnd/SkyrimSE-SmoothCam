#pragma once
#include "camera_states/base_third.h"

namespace Camera {
	class Thirdperson;

	namespace State {
		class ThirdpersonVanityState : public BaseThird {
			public:
				explicit ThirdpersonVanityState(Thirdperson* camera) noexcept;
				ThirdpersonVanityState(const ThirdpersonVanityState&) = delete;
				ThirdpersonVanityState(ThirdpersonVanityState&&) noexcept = delete;
				ThirdpersonVanityState& operator=(const ThirdpersonVanityState&) = delete;
				ThirdpersonVanityState& operator=(ThirdpersonVanityState&&) noexcept = delete;

			public:
				virtual void OnBegin(RE::PlayerCharacter* player, RE::Actor* cameraRef, RE::PlayerCamera* playerCamera,
					BaseThird* fromState) noexcept override;
				virtual bool OnEnd(RE::PlayerCharacter* player, RE::Actor* cameraRef, RE::PlayerCamera* playerCamera,
					BaseThird* nextState, bool forced) noexcept override;
				virtual void Update(RE::PlayerCharacter* player, RE::Actor* cameraRef, RE::PlayerCamera* playerCamera)
					noexcept override;
		};
	}
}