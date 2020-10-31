#pragma once
#include "camera_state.h"

namespace Camera {
	namespace State {
		class ThirdpersonState : public BaseCameraState {
			public:
				ThirdpersonState(Camera::SmoothCamera* camera) noexcept;
				ThirdpersonState(const ThirdpersonState&) = delete;
				ThirdpersonState(ThirdpersonState&&) noexcept = delete;
				ThirdpersonState& operator=(const ThirdpersonState&) = delete;
				ThirdpersonState& operator=(ThirdpersonState&&) noexcept = delete;

			public:
				virtual void OnBegin(const PlayerCharacter* player, const TESObjectREFR* cameraRef, const CorrectedPlayerCamera* camera,
					BaseCameraState* fromState) override;
				virtual void OnEnd(const PlayerCharacter* player, const TESObjectREFR* cameraRef, const CorrectedPlayerCamera* camera,
					BaseCameraState* nextState) override;
				virtual void Update(PlayerCharacter* player, const TESObjectREFR* cameraRef, const CorrectedPlayerCamera* camera) override;
		};
	}
}