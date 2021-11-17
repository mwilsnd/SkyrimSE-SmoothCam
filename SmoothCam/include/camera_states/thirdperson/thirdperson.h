#pragma once
#include "camera_states/base_third.h"

namespace Camera {
	class Thirdperson;

	namespace State {
		class ThirdpersonState : public BaseThird {
			public:
				explicit ThirdpersonState(Thirdperson* camera) noexcept;
				ThirdpersonState(const ThirdpersonState&) = delete;
				ThirdpersonState(ThirdpersonState&&) noexcept = delete;
				ThirdpersonState& operator=(const ThirdpersonState&) = delete;
				ThirdpersonState& operator=(ThirdpersonState&&) noexcept = delete;

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