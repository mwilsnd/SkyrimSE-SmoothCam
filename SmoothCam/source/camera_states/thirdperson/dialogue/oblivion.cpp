#ifdef DEVELOPER
#include "thirdperson.h"
#include "camera_states/thirdperson/dialogue/oblivion.h"

using namespace Camera;

State::OblivionDialogue::OblivionDialogue(ThirdpersonDialogueState* parentState) noexcept
	: IThirdPersonDialogue(parentState)
{}

State::OblivionDialogue::~OblivionDialogue() noexcept {}

void State::OblivionDialogue::OnStart(RE::PlayerCharacter*, RE::Actor*,
	RE::PlayerCamera*) noexcept
{
	runningExit = exitDone = false;
	zoomStartTime = static_cast<float>(GameTime::CurTime());
	startPosition = parentState->GetCameraPosition().world;

	// Lock input
	parentState->LockInputState(RE::CameraState::kAnimated, true);
	parentState->LockInputState(RE::CameraState::kThirdPerson, true);
	parentState->LockInputState(RE::CameraState::kMount, true);
	parentState->LockInputState(RE::CameraState::kDragon, true);
	parentState->LockInputState(RE::CameraState::kBleedout, true);
}

void State::OblivionDialogue::OnEnd(RE::PlayerCharacter* player, RE::Actor* cameraRef,
	RE::PlayerCamera* playerCamera) noexcept
{
	parentState->MoveToGoalPosition(player, cameraRef, playerCamera);
	player->SetAlpha(1.0f);

	// Unlock input
	parentState->LockInputState(RE::CameraState::kAnimated, false);
	parentState->LockInputState(RE::CameraState::kThirdPerson, false);
	parentState->LockInputState(RE::CameraState::kMount, false);
	parentState->LockInputState(RE::CameraState::kDragon, false);
	parentState->LockInputState(RE::CameraState::kBleedout, false);
}

glm::vec3 State::OblivionDialogue::GetActorGoalPos(RE::TESObjectREFR* actor)
	const noexcept
{
	auto goal = skyrim_cast<RE::Actor*>(actor);
	if (goal && GameState::IsSitting(goal)) {
		auto node = parentState->FindFollowBone(actor, "Camera3rd [Cam3]");
		if (node)
			return glm::vec3{
				node->world.translate.x,
				node->world.translate.y,
				node->world.translate.z
			};
	}

	return parentState->GetCameraWorldPosition(actor);
}

glm::vec3 State::OblivionDialogue::GetActorFocalPos(RE::TESObjectREFR* actor) const noexcept {
	auto focalPoint = parentState->FindFocalBone(actor);
	if (!focalPoint) return { actor->GetPositionX(), actor->GetPositionY(), actor->GetPositionZ() };
	return {
		focalPoint->world.translate.x,
		focalPoint->world.translate.y,
		focalPoint->world.translate.z
	};
}

void State::OblivionDialogue::Update(RE::PlayerCharacter* player, RE::Actor*,
	RE::PlayerCamera* playerCamera) noexcept
{
	auto focalActor = GameState::GetDialogueTarget();
	if (!focalActor) return;

	auto cameraRot = parentState->GetCameraRotation();
	const auto goalPos = GetActorGoalPos(player);
	const auto lookPos = GetActorFocalPos(focalActor.get());
	const auto lookVector = glm::normalize(goalPos - lookPos);

	const auto zoomDuration = Config::GetCurrentConfig()->oblivionDialogue.zoomInDuration;
	auto goalFOV = Config::GetCurrentConfig()->oblivionDialogue.fovOffset;

	const auto fovDistPos = glm::vec3{ focalActor->GetPositionX(), focalActor->GetPositionY(), focalActor->GetPositionZ() + lookPos.z };
	const auto distance = glm::distance(goalPos, fovDistPos);

	// @Note: This isn't totally correct - We need to project the bounding extents of the focal bone to the screen
	// and figure an FOV which results in a goal occupany ratio of bone extents / screen space.
	if (distance <= 100.0f && distance >= 80.0f) {
		const auto fovScale = mmath::Remap(glm::clamp(distance, 100.0f, 80.0f), 80.0f, 100.0f, 0.0f, 1.0f);
		goalFOV = mmath::Interpolate(goalFOV, 0.0f, 1.0f - (1.0f + log10f(fovScale)));
	} else if (distance < 80.0f)
		goalFOV = 0.0f;

	const auto scalar = glm::clamp(
		static_cast<float>(GameTime::CurTime() - zoomStartTime) / glm::max(zoomDuration, 0.01f),
		0.0f, 1.0f
	);

	// Rot
	mmath::Rotation slerped;
	lookRotation.SetQuaternion(glm::quatLookAt(lookVector, { 0.0f, 0.0f, 1.0f }));
	slerped.SetQuaternion(glm::slerp(parentState->GetBaseGameRotation(playerCamera).quat, lookRotation.quat, scalar));
	parentState->SetCameraRotation(slerped, playerCamera);

	// Pos
	lastPosition = mmath::Interpolate<glm::vec3, float>(startPosition, goalPos, scalar);
	parentState->SetCameraPosition(lastPosition, player, playerCamera);
	
	// FOV
	curFOV = mmath::Interpolate(
		parentState->GetFOVTransitionState().currentPosition,
		goalFOV,
		scalar
	);
	parentState->SetFOVOffset(curFOV, true);

	// Fade
	auto fadeScalar = mmath::Remap(glm::min(scalar, 0.25f), 0.0f, 0.25f, 0.0f, 1.0f);
	player->SetAlpha(1.0f - fadeScalar);
}

void State::OblivionDialogue::ExitPoll(RE::PlayerCharacter* player, RE::Actor* cameraRef,
	RE::PlayerCamera* playerCamera) noexcept
{
	const auto exitDuration = Config::GetCurrentConfig()->oblivionDialogue.zoomOutDuration;
	const auto scalar = glm::clamp(
		static_cast<float>(GameTime::CurTime() - zoomStartTime) / glm::max(exitDuration, 0.0001f),
		0.0f, 1.0f
	);

	// Pos
	auto world = parentState->GetCameraWorldPosition(cameraRef) +
		parentState->GetTransformedCameraLocalPosition();
	parentState->SetCameraPosition(
		mmath::Interpolate<glm::vec3, float>(lastPosition, world, scalar),
		player, playerCamera
	);

	// Rot
	mmath::Rotation slerped;
	slerped.SetQuaternion(glm::slerp(lookRotation.quat, parentState->GetBaseGameRotation(playerCamera).quat, scalar));
	parentState->SetCameraRotation(slerped, playerCamera);

	// FOV
	const auto targetFOV = parentState->GetFOVTransitionState().currentPosition;
	const auto fov = mmath::Interpolate(
		curFOV,
		targetFOV,
		scalar
	);
	parentState->SetFOVOffset(fov, true);

	// Fade
	player->SetAlpha(scalar >= 0.75f ? 1.0f : 0.0f);

	if (scalar >= 1.0f)
		exitDone = true;
}

bool State::OblivionDialogue::CanExit(RE::PlayerCharacter*, RE::Actor*,
	RE::PlayerCamera*) noexcept
{
	if (!runningExit) {
		runningExit = true;
		zoomStartTime = static_cast<float>(GameTime::CurTime());
		return false;
	}

	return exitDone;
}
#endif