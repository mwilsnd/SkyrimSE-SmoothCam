#pragma once
#include "camera_states/base_third.h"

namespace Camera {
	class Thirdperson;

	namespace State {
		class ThirdpersonState : public BaseThird {
			public:
				ThirdpersonState(Thirdperson* camera) noexcept;
				ThirdpersonState(const ThirdpersonState&) = delete;
				ThirdpersonState(ThirdpersonState&&) noexcept = delete;
				ThirdpersonState& operator=(const ThirdpersonState&) = delete;
				ThirdpersonState& operator=(ThirdpersonState&&) noexcept = delete;

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