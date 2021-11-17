#pragma once
#include "camera_states/thirdperson/thirdperson_dialogue.h"

namespace Camera {
	namespace State {
		class SkyrimDialogue : public IThirdPersonDialogue {
			public:
				explicit SkyrimDialogue(ThirdpersonDialogueState* parentState) noexcept;
				virtual ~SkyrimDialogue() noexcept;

			public:
				virtual void OnStart(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept override;
				virtual void OnEnd(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept override;
				virtual void Update(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept override;
				virtual void ExitPoll(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept override;
				virtual bool CanExit(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept override;
		};
	}
}