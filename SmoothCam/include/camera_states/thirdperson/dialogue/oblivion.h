#pragma once
#ifdef DEVELOPER
#include "camera_states/thirdperson/thirdperson_dialogue.h"

namespace Camera {
	namespace State {
		// Oblivion style dialogue camera, zoom in on the actor's face
		class OblivionDialogue : public IThirdPersonDialogue {
			public:
				explicit OblivionDialogue(ThirdpersonDialogueState* parentState) noexcept;
				virtual ~OblivionDialogue() noexcept;

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
			
			private:
				glm::vec3 GetActorGoalPos(RE::TESObjectREFR* actor) const noexcept;
				glm::vec3 GetActorFocalPos(RE::TESObjectREFR* actor) const noexcept;

			private:
				bool runningExit = false;
				bool exitDone = false;
				float zoomStartTime = 0.0f;
				float curFOV = 0.0f;
				glm::vec3 lastPosition{};
				glm::vec3 startPosition{};
				mmath::Rotation lookRotation;
		};
	}
}
#endif