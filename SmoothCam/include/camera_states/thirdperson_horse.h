#pragma once
#include "camera_state.h"

namespace Camera {
	namespace State {
		class ThirdpersonHorseState : public BaseCameraState {
			public:
				ThirdpersonHorseState(Camera::SmoothCamera* camera) noexcept;
				ThirdpersonHorseState(const ThirdpersonHorseState&) = delete;
				ThirdpersonHorseState(ThirdpersonHorseState&&) noexcept = delete;
				ThirdpersonHorseState& operator=(const ThirdpersonHorseState&) = delete;
				ThirdpersonHorseState& operator=(ThirdpersonHorseState&&) noexcept = delete;

			public:
				virtual void OnBegin(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) override;
				virtual void OnEnd(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) override;
				virtual void Update(PlayerCharacter* player, const CorrectedPlayerCamera* camera) override;
		};
	}
}