#pragma once
#include "camera_states/base_third.h"

namespace Camera {
	class Thirdperson;

	namespace State {
		class ThirdpersonCombatState : public BaseThird {
			public:
				ThirdpersonCombatState(Thirdperson* camera) noexcept;
				ThirdpersonCombatState(const ThirdpersonCombatState&) = delete;
				ThirdpersonCombatState(ThirdpersonCombatState&&) noexcept = delete;
				ThirdpersonCombatState& operator=(const ThirdpersonCombatState&) = delete;
				ThirdpersonCombatState& operator=(ThirdpersonCombatState&&) noexcept = delete;

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