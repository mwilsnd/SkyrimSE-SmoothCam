#pragma once
#include "camera_states/base_third.h"

namespace Camera {
	class Thirdperson;

	namespace State {
		class ThirdpersonHorseState : public BaseThird {
			public:
				ThirdpersonHorseState(Thirdperson* camera) noexcept;
				ThirdpersonHorseState(const ThirdpersonHorseState&) = delete;
				ThirdpersonHorseState(ThirdpersonHorseState&&) noexcept = delete;
				ThirdpersonHorseState& operator=(const ThirdpersonHorseState&) = delete;
				ThirdpersonHorseState& operator=(ThirdpersonHorseState&&) noexcept = delete;

			public:
				virtual void OnBegin(const PlayerCharacter* player, const Actor* cameraRef, const CorrectedPlayerCamera* camera,
					BaseThird* fromState) noexcept override;
				virtual void OnEnd(const PlayerCharacter* player, const Actor* cameraRef, const CorrectedPlayerCamera* camera,
					BaseThird* nextState) noexcept override;
				virtual void Update(PlayerCharacter* player, const Actor* cameraRef, const CorrectedPlayerCamera* camera)
					noexcept override;
		};
	}
}