#ifdef DEVELOPER
#include "thirdperson.h"
#include "camera_states/thirdperson/dialogue/face_to_face.h"

using namespace Camera;

State::FaceToFaceDialogue::FaceToFaceDialogue(ThirdpersonDialogueState* parentState) noexcept
	: IThirdPersonDialogue(parentState)
{
#ifdef DEBUG
	auto& ctx = Render::GetContext();

	Render::CBufferCreateInfo perFrane;
	perFrane.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	perFrane.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	perFrane.size = sizeof(decltype(cbufPerFrameStaging));
	perFrane.initialData = &cbufPerFrameStaging;
	cbufPerFrame = eastl::make_shared<Render::CBuffer>(perFrane, ctx);
	segmentDrawer = eastl::make_unique<Render::LineDrawer>(ctx);

	Render::OnPresent(std::bind(&State::FaceToFaceDialogue::Draw, this, std::placeholders::_1));
#endif
}

State::FaceToFaceDialogue::~FaceToFaceDialogue() noexcept {}

#ifdef DEBUG
void State::FaceToFaceDialogue::Draw(Render::D3DContext& ctx) noexcept {
	if (!allowDraw || !drawOverlay) return;
	if (segments.size() <= 0) return;

	auto rot =  parentState->GetCameraRotation();// lastRotation;
	rot.SetEuler(rot.euler.x + mmath::half_pi, rot.euler.y);

	auto q = glm::rotate(glm::identity<glm::quat>(), rot.euler.y, { 0.0f, 0.0f, 1.0f });
	q = glm::rotate(q, rot.euler.x, { 1.0f, 0.0f, 0.0f });

	auto euler = glm::vec2{ glm::pitch(q), glm::roll(q) };

	const auto matProj = Render::GetProjectionMatrix(parentState->GetFrustum());
	const auto matView = Render::BuildViewMatrix(lastPosition, euler);
	cbufPerFrameStaging.matProjView = matProj * matView;
	cbufPerFrame->Update(
		&cbufPerFrameStaging, 0,
		sizeof(decltype(cbufPerFrameStaging)), ctx
	);

	cbufPerFrame->Bind(Render::PipelineStage::Vertex, 1, ctx);
	cbufPerFrame->Bind(Render::PipelineStage::Fragment, 1, ctx);

	// Setup depth and blending
	Render::SetDepthState(ctx, false, false, D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL);
	Render::SetBlendState(
		ctx, true,
		D3D11_BLEND_OP::D3D11_BLEND_OP_ADD, D3D11_BLEND_OP::D3D11_BLEND_OP_ADD,
		D3D11_BLEND::D3D11_BLEND_SRC_ALPHA, D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND::D3D11_BLEND_ONE, D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA
	);

	segmentDrawer->Submit(segments);
	segments.clear();
}

void State::FaceToFaceDialogue::DrawBounds(RE::NiAVObject* obj, const glm::vec4&& col) noexcept {
	auto center = glm::vec3{ obj->world.translate.x, obj->world.translate.y, obj->world.translate.z };
	auto extent = 15.0f;

	eastl::array<glm::vec3, 6> points = {
		center + glm::vec3{extent, 0.0f, 0.0f},
		center + glm::vec3{-extent, 0.0f, 0.0f},

		center + glm::vec3{0.0f, extent, 0.0f},
		center + glm::vec3{0.0f, -extent, 0.0f},

		center + glm::vec3{0.0f, 0.0f, extent},
		center + glm::vec3{0.0f, 0.0f, -extent}
	};

	for (auto& point : points) {
		segments.emplace_back(
			Render::Point(Render::ToRenderScale(center), col),
			Render::Point(Render::ToRenderScale(point), col)
		);
	}
}

void State::FaceToFaceDialogue::DrawBounds(const glm::vec3& loc, const glm::vec4&& col) noexcept {
	auto extent = 15.0f;

	eastl::array<glm::vec3, 6> points = {
		loc + glm::vec3{extent, 0.0f, 0.0f},
		loc + glm::vec3{-extent, 0.0f, 0.0f},

		loc + glm::vec3{0.0f, extent, 0.0f},
		loc + glm::vec3{0.0f, -extent, 0.0f},

		loc + glm::vec3{0.0f, 0.0f, extent},
		loc + glm::vec3{0.0f, 0.0f, -extent}
	};

	for (auto& point : points) {
		segments.emplace_back(
			Render::Point(Render::ToRenderScale(loc), col),
			Render::Point(Render::ToRenderScale(point), col)
		);
	}
}
#endif

void State::FaceToFaceDialogue::OnStart(RE::PlayerCharacter* player, RE::Actor*,
	RE::PlayerCamera* playerCamera) noexcept
{
#ifdef DEBUG
	allowDraw = true;
#endif
	zoomStartTime = actorDelayStartTime = static_cast<float>(GameTime::CurTime());
	runningExit = exitDone = false;
	awaitBufferDelay = true;
	lastFocalActor = nullptr;
	curMoveDuration = Config::GetCurrentConfig()->faceToFaceDialogue.zoomInDuration;

	// Setup initial position
	startPosition = parentState->GetCameraPosition().world;
	lastRotation = parentState->GetBaseGameRotation(playerCamera);
	lastRotation.SetEuler(lastRotation.euler.x - mmath::half_pi, lastRotation.euler.y);

	// Lock input
	parentState->LockInputState(RE::CameraState::kAnimated, true);
	parentState->LockInputState(RE::CameraState::kThirdPerson, true);
	parentState->LockInputState(RE::CameraState::kMount, true);
	parentState->LockInputState(RE::CameraState::kDragon, true);
	parentState->LockInputState(RE::CameraState::kBleedout, true);

	auto dialogueTarget = GameState::GetDialogueTarget();
	if (!dialogueTarget || !dialogueTarget.get()) return;
	glm::vec3 playerPos = { player->GetPositionX(), player->GetPositionY(), player->GetPositionZ() };
	glm::vec3 targetPos = { dialogueTarget->GetPositionX(), dialogueTarget->GetPositionY(), dialogueTarget->GetPositionZ() };

	auto q = glm::quatLookAt(playerPos - targetPos, { 0.0f, 0.0f, 1.0f });
	player->data.angle.z = -dialogueTarget->data.angle.z;
}

void State::FaceToFaceDialogue::OnEnd(RE::PlayerCharacter* player, RE::Actor* cameraRef,
	RE::PlayerCamera* playerCamera) noexcept
{
#ifdef DEBUG
	allowDraw = false;
#endif
	parentState->MoveToGoalPosition(player, cameraRef, playerCamera);

	// Unlock input
	parentState->LockInputState(RE::CameraState::kAnimated, false);
	parentState->LockInputState(RE::CameraState::kThirdPerson, false);
	parentState->LockInputState(RE::CameraState::kMount, false);
	parentState->LockInputState(RE::CameraState::kDragon, false);
	parentState->LockInputState(RE::CameraState::kBleedout, false);
}

glm::vec3 State::FaceToFaceDialogue::GetActorGoalPos(RE::TESObjectREFR* actor, const glm::mat4&& rotation,
	bool flipShoulder) noexcept
{
	auto side = Config::GetCurrentConfig()->faceToFaceDialogue.sideOffset * (flipShoulder ? -1.0f : 1.0f);
	auto shoulderPos = rotation * glm::vec4(
		side,
		Config::GetCurrentConfig()->faceToFaceDialogue.zoomOffset,
		0.0f,
		1.0f
	);
	shoulderPos.z += Config::GetCurrentConfig()->faceToFaceDialogue.upOffset;

	auto goal = skyrim_cast<RE::Actor*>(actor);
	if (goal && GameState::IsSitting(goal)) {
		auto node = parentState->FindFollowBone(actor, "Camera3rd [Cam3]");
		if (node)
			return glm::vec3{
				node->world.translate.x,
				node->world.translate.y,
				node->world.translate.z,
			} + static_cast<glm::vec3>(shoulderPos);
	}

	return parentState->GetCameraWorldPosition(actor) + static_cast<glm::vec3>(shoulderPos);
}

glm::vec3 State::FaceToFaceDialogue::GetActorFocalPos(RE::TESObjectREFR* actor) noexcept {
	auto focalPoint = parentState->FindFollowBone(actor, "Camera3rd [Cam3]");
	if (!focalPoint) return { actor->GetPositionX(), actor->GetPositionY(), actor->GetPositionZ() };

	return {
		focalPoint->world.translate.x,
		focalPoint->world.translate.y,
		focalPoint->world.translate.z
	};
}

RE::TESObjectREFR* State::FaceToFaceDialogue::SelectFocalActor(RE::PlayerCharacter* player,
	RE::TESObjectREFR* actor) noexcept
{
	auto focusAsActor = skyrim_cast<RE::Actor*>(actor);
	if (!focusAsActor) return player;

	if (awaitBufferDelay && !IsDelayTimerExpired()) return actor;
	if (GameState::IsActorTalking(focusAsActor)) return actor;

	return player;
}

bool State::FaceToFaceDialogue::IsDelayTimerExpired() noexcept {
	if (awaitBufferDelay) {
		const auto expired = GameTime::CurTime() > actorDelayStartTime + actorInitialWaitDelay;
		if (expired) awaitBufferDelay = false;
		return expired;
	} else {
		return GameTime::CurTime() > actorDelayStartTime + actorWaitDelay;
	}
}

void State::FaceToFaceDialogue::Update(RE::PlayerCharacter* player, RE::Actor*,
	RE::PlayerCamera* playerCamera) noexcept
{
	auto dialogueTarget = GameState::GetDialogueTarget();
	if (!dialogueTarget || !dialogueTarget.get()) return;

	auto focusAsActor = skyrim_cast<RE::Actor*>(dialogueTarget.get());
	if (focusAsActor && GameState::IsActorTalking(focusAsActor) && lastFocalActor != player) {
		actorDelayStartTime = static_cast<float>(GameTime::CurTime());
	}

	auto focalActor = SelectFocalActor(player, dialogueTarget.get());
	if (!focalActor) return;

	if (lastFocalActor && focalActor != lastFocalActor) {
		if (!IsDelayTimerExpired()) {
			focalActor = lastFocalActor;
		} else {
			// Flip actor focus
			zoomStartTime = static_cast<float>(GameTime::CurTime());
			curMoveDuration = Config::GetCurrentConfig()->faceToFaceDialogue.rotationDuration;
			startPosition = lastPosition;
			lastRotation = lookRotation;
			lastFocalActor = focalActor;
		}
	} else {
		lastFocalActor = focalActor;
	}

	auto positionActor = focalActor == player ? dialogueTarget.get() : player;
	const auto lookPos = GetActorFocalPos(focalActor);

	auto cameraRot = parentState->GetCameraRotation();
	mmath::Rotation r;

	auto com = RE::BSFixedString("NPC Neck [Neck]");
	auto act = skyrim_cast<RE::Actor*>(positionActor);
	auto rotBone = act->loadedData->data3D->GetObjectByName(com);
	auto e = rotBone ? mmath::NiMatrixToEuler(rotBone->world.rotate) : glm::vec3{};

	float heading, at, bank;
	rotBone->world.rotate.ToEulerAnglesXYZ(heading, at, bank);
	r.SetEuler(0.0f, positionActor->GetAngleZ()); 

	auto flip = false;
	if (parentState->IsShoulderSwapped()) flip = !flip;

	const auto goalPos = GetActorGoalPos(positionActor, r.ToRotationMatrix(), flip);
	const auto lookVector = glm::normalize(goalPos - lookPos);

#ifdef DEBUG
	DrawBounds(GetActorFocalPos(positionActor), { 0.5f, 1.0f, 0.5f, 1.0f });
	DrawBounds(GetActorFocalPos(focalActor), { 1.0f, 1.0f, 0.25f, 1.0f });
#endif

	const auto focalGoal = GetActorGoalPos(focalActor, r.ToRotationMatrix(), !flip);

#ifdef DEBUG
	DrawBounds(focalGoal, { 0.5f, 0.5f, 1.0f, 1.0f });
#endif

	const auto scalar = glm::clamp(
		static_cast<float>(GameTime::CurTime() - zoomStartTime) / glm::max(curMoveDuration, 0.01f),
		0.0f, 1.0f
	);

	// Rot
	mmath::Rotation slerped;
	lookRotation.SetQuaternion(glm::quatLookAt(lookVector, { 0.0f, 0.0f, 1.0f }));
	slerped.SetQuaternion(glm::slerp(lastRotation.quat, lookRotation.quat, scalar));
	parentState->SetCameraRotation(slerped, playerCamera);

	// Pos
	lastPosition = mmath::Interpolate<glm::vec3, float>(startPosition, goalPos, scalar);
	parentState->SetCameraPosition(lastPosition, player, playerCamera);
}

void State::FaceToFaceDialogue::ExitPoll(RE::PlayerCharacter* player, RE::Actor* cameraRef,
	RE::PlayerCamera* playerCamera) noexcept
{
	const auto world = parentState->GetCameraWorldPosition(cameraRef) +
		parentState->GetTransformedCameraLocalPosition();

	const auto exitDuration = Config::GetCurrentConfig()->faceToFaceDialogue.zoomOutDuration;
	const auto scalar = glm::clamp(
		static_cast<float>(GameTime::CurTime() - zoomStartTime) / glm::max(exitDuration, 0.0001f),
		0.0f, 1.0f
	);

	// Rot
	mmath::Rotation slerped;
	slerped.SetQuaternion(glm::slerp(lookRotation.quat, parentState->GetBaseGameRotation(playerCamera).quat, scalar));
	parentState->SetCameraRotation(slerped, playerCamera);

	// Pos
	parentState->SetCameraPosition(
		mmath::Interpolate<glm::vec3, float>(lastPosition, world, scalar),
		player, playerCamera
	);

	if (scalar >= 1.0f) exitDone = true;
}
	
bool State::FaceToFaceDialogue::CanExit(RE::PlayerCharacter*, RE::Actor*,
	RE::PlayerCamera*) noexcept
{
	if (!runningExit) {
		runningExit = true;
		zoomStartTime = static_cast<float>(GameTime::CurTime());

		// Unlock input
		parentState->LockInputState(RE::CameraState::kAnimated, false);
		parentState->LockInputState(RE::CameraState::kThirdPerson, false);
		parentState->LockInputState(RE::CameraState::kMount, false);
		parentState->LockInputState(RE::CameraState::kDragon, false);
		parentState->LockInputState(RE::CameraState::kBleedout, false);
		return false;
	}

	return exitDone;
}
#endif