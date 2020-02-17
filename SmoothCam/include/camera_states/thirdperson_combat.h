#pragma once
#include "pch.h"
#include "camera_state.h"

namespace Camera {
	namespace State {
		class ThirdpersonCombatState : public BaseCameraState {
			public:
				ThirdpersonCombatState(Camera::SmoothCamera* camera) noexcept;
				ThirdpersonCombatState(const ThirdpersonCombatState&) = delete;
				ThirdpersonCombatState(ThirdpersonCombatState&&) noexcept = delete;
				ThirdpersonCombatState& operator=(const ThirdpersonCombatState&) = delete;
				ThirdpersonCombatState& operator=(ThirdpersonCombatState&&) noexcept = delete;

			public:
				virtual void OnBegin(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) override;
				virtual void OnEnd(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) override;
				virtual void Update(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) override;
		};
	}
}