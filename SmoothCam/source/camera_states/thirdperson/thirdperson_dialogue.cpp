#include "compat.h"
#include "thirdperson.h"
#include "camera_states/thirdperson/thirdperson_dialogue.h"
#include "camera_states/thirdperson/dialogue/skyrim.h"
#ifdef DEVELOPER
#include "camera_states/thirdperson/dialogue/oblivion.h"
#include "camera_states/thirdperson/dialogue/face_to_face.h"
#endif
Camera::State::IThirdPersonDialogue::IThirdPersonDialogue(ThirdpersonDialogueState* parentState) noexcept
	: parentState(parentState) {}

Camera::State::IThirdPersonDialogue::~IThirdPersonDialogue() noexcept {}

Camera::State::ThirdpersonDialogueState::ThirdpersonDialogueState(Thirdperson* camera) noexcept
	: BaseThird(camera)
{
	modes[static_cast<size_t>(Config::DialogueMode::Skyrim)] = eastl::move(
		eastl::make_unique<State::SkyrimDialogue>(this)
	);
#ifdef DEVELOPER
	modes[static_cast<size_t>(Config::DialogueMode::Oblivion)] = eastl::move(
		eastl::make_unique<State::OblivionDialogue>(this)
	);
	modes[static_cast<size_t>(Config::DialogueMode::FaceToFace)] = eastl::move(
		eastl::make_unique<State::FaceToFaceDialogue>(this)
	);
#endif
}

void Camera::State::ThirdpersonDialogueState::OnBegin(RE::PlayerCharacter* player, RE::Actor* cameraRef,
	RE::PlayerCamera* playerCamera, BaseThird*) noexcept
{
	if (Config::GetCurrentConfig()->dialogueMode == Config::DialogueMode::Disabled ||
		Config::GetCurrentConfig()->dialogueMode == Config::DialogueMode::MAX ||
		Compat::IsConsumerPresent("Alternate Conversation Camera"))
	{
		activeMode = nullptr;
		return;
	}

	auto& mode = modes[static_cast<size_t>(Config::GetCurrentConfig()->dialogueMode)];
	if (!mode) {
		activeMode = nullptr;
		return;
	}

	activeMode = mode.get();

	// @Note: From ACC, clear the NPC's pathfinding goal so they stop moving when we start talking
	auto focalActor = GameState::GetDialogueTarget();
	if (focalActor) {
		auto asActor = skyrim_cast<RE::Actor*>(focalActor.get());
		if (asActor && asActor->movementController) {
			RE::BSFixedString pathGoalInterface("IMovementSetGoal");
			
			auto movementController = reinterpret_cast<SkyrimSE::MovementControllerNPC*>(asActor->movementController.get());
			auto pathingGoal = movementController->QueryMovementInterface(pathGoalInterface);
			if (pathingGoal)
				reinterpret_cast<SkyrimSE::IMovementSetGoal*>(pathingGoal)->ClearPathingRequest();
		}
	}

	exitRequested = false;
	activeMode->OnStart(player, cameraRef, playerCamera);
}

bool Camera::State::ThirdpersonDialogueState::OnEnd(RE::PlayerCharacter* player, RE::Actor* cameraRef,
	RE::PlayerCamera* playerCamera, BaseThird* nextState, bool forced) noexcept
{
	if (forced || !activeMode) {
		if (activeMode) activeMode->OnEnd(player, cameraRef, playerCamera);
		StateHandOff(nextState);
		return false;
	}

	if (!activeMode->CanExit(player, cameraRef, playerCamera)) {
		exitRequested = true;
		return true;
	}

	if (activeMode) activeMode->OnEnd(player, cameraRef, playerCamera);
	StateHandOff(nextState); 
	return false;
}

void Camera::State::ThirdpersonDialogueState::Update(RE::PlayerCharacter* player, RE::Actor* cameraRef,
	RE::PlayerCamera* playerCamera) noexcept
{
	BaseThird::Update(player, cameraRef, playerCamera);

	if (!activeMode) return;
	if (exitRequested)
		activeMode->ExitPoll(player, cameraRef, playerCamera);
	else
		activeMode->Update(player, cameraRef, playerCamera);
}

RE::NiAVObject* Camera::State::ThirdpersonDialogueState::FindFocalBone(RE::TESObjectREFR* ref, const eastl::string_view& filterBone)
	const noexcept
{
	if (!ref->loadedData || !ref->loadedData->data3D) return nullptr;
	auto& boneList = Config::GetFocusBonePriorities();

	if (filterBone.length() > 0)
		for (auto it = boneList.begin(); it != boneList.end(); it++) {
			if (filterBone.compare(it->c_str()) == 0) continue;
			auto node = ref->loadedData->data3D->GetObjectByName(*it);
			if (node) return node;
		}
	else
		for (auto it = boneList.begin(); it != boneList.end(); it++) {
			auto node = ref->loadedData->data3D->GetObjectByName(*it);
			if (node) return node;
		}

	return nullptr;
}

Camera::Thirdperson* Camera::State::ThirdpersonDialogueState::GetThirdpersonCamera() const noexcept {
	return camera;
}