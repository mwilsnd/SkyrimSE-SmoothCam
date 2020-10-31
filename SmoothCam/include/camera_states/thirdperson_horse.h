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
				virtual void OnBegin(const PlayerCharacter* player, const TESObjectREFR* cameraRef, const CorrectedPlayerCamera* camera,
					BaseCameraState* fromState) override;
				virtual void OnEnd(const PlayerCharacter* player, const TESObjectREFR* cameraRef, const CorrectedPlayerCamera* camera,
					BaseCameraState* nextState) override;
				virtual void Update(PlayerCharacter* player, const TESObjectREFR* cameraRef, const CorrectedPlayerCamera* camera) override;
		};
	}
}