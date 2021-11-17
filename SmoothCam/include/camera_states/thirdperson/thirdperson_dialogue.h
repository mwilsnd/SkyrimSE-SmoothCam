#pragma once
#include "camera_states/base_third.h"

namespace Camera {
	class Thirdperson;

	namespace State {
		class ThirdpersonDialogueState;

		class IThirdPersonDialogue {
			public:
				explicit IThirdPersonDialogue(ThirdpersonDialogueState* parentState) noexcept;
				virtual ~IThirdPersonDialogue() noexcept;

			public:
				virtual void OnStart(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept = 0;
				virtual void OnEnd(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept = 0;
				virtual void Update(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept = 0;
				virtual void ExitPoll(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept = 0;
				virtual bool CanExit(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept = 0;

			protected:
				ThirdpersonDialogueState* parentState = nullptr;
		};

		class ThirdpersonDialogueState : public BaseThird {
			public:
				explicit ThirdpersonDialogueState(Thirdperson* camera) noexcept;
				ThirdpersonDialogueState(const ThirdpersonDialogueState&) = delete;
				ThirdpersonDialogueState(ThirdpersonDialogueState&&) noexcept = delete;
				ThirdpersonDialogueState& operator=(const ThirdpersonDialogueState&) = delete;
				ThirdpersonDialogueState& operator=(ThirdpersonDialogueState&&) noexcept = delete;

			public:
				virtual void OnBegin(RE::PlayerCharacter* player, RE::Actor* cameraRef, RE::PlayerCamera* playerCamera,
					BaseThird* fromState) noexcept override;
				virtual bool OnEnd(RE::PlayerCharacter* player, RE::Actor* cameraRef, RE::PlayerCamera* playerCamera,
					BaseThird* nextState, bool forced) noexcept override;
				virtual void Update(RE::PlayerCharacter* player, RE::Actor* cameraRef, RE::PlayerCamera* playerCamera)
					noexcept override;

				RE::NiAVObject* FindFocalBone(RE::TESObjectREFR* ref, const eastl::string_view& filterBone = "") const noexcept;
				Thirdperson* GetThirdpersonCamera() const noexcept;

			private:
				eastl::array<
					eastl::unique_ptr<IThirdPersonDialogue>,
					static_cast<size_t>(Config::DialogueMode::MAX)
				> modes;
				IThirdPersonDialogue* activeMode = nullptr;
				bool exitRequested = false;
		};
	}
}