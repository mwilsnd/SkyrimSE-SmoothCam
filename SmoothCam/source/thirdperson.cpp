#include "thirdperson.h"
#include "compat.h"
#include "debug/eh.h"

Camera::Thirdperson::Thirdperson(Camera* baseCamera) : ICamera(baseCamera, CameraID::Thirdperson) {
	config = Config::GetCurrentConfig();
	crosshair = eastl::make_unique<Crosshair::Manager>();

	cameraStates[static_cast<size_t>(GameState::CameraState::ThirdPerson)] =
		eastl::move(eastl::make_unique<State::ThirdpersonState>(this));
	cameraStates[static_cast<size_t>(GameState::CameraState::ThirdPersonCombat)] =
		eastl::move(eastl::make_unique<State::ThirdpersonCombatState>(this));
	cameraStates[static_cast<size_t>(GameState::CameraState::Horseback)] =
		eastl::move(eastl::make_unique<State::ThirdpersonHorseState>(this));

#ifdef WITH_CHARTS
	if (Render::HasContext()) {
		focusTargetNodeTree = eastl::make_unique<Render::NiNodeTreeDisplay>(600, 1080, Render::GetContext());
		stateOverlay = eastl::make_unique<Render::StateOverlay>(600, 128, this, Render::GetContext());

		graph_worldPosTarget = eastl::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		graph_offsetPos = eastl::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		graph_targetOffsetPos = eastl::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		graph_localSpace = eastl::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		graph_rotation = eastl::make_unique<Render::LineGraph>(2, 128, 600, 128, Render::GetContext());
		graph_tpsRotation = eastl::make_unique<Render::LineGraph>(4, 128, 600, 128, Render::GetContext());
		graph_computeTime = eastl::make_unique<Render::LineGraph>(2, 128, 600, 128, Render::GetContext());

		const auto xColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		const auto yColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		const auto zColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		graph_worldPosTarget->SetLineColor(0, xColor);
		graph_worldPosTarget->SetLineColor(1, yColor);
		graph_worldPosTarget->SetLineColor(2, zColor);
		graph_worldPosTarget->SetName(L"World Pos");

		graph_localSpace->SetLineColor(0, xColor);
		graph_localSpace->SetLineColor(1, yColor);
		graph_localSpace->SetLineColor(2, zColor);
		graph_localSpace->SetPosition(0, 128);
		graph_localSpace->SetName(L"Local Space");

		graph_offsetPos->SetLineColor(0, xColor);
		graph_offsetPos->SetLineColor(1, yColor);
		graph_offsetPos->SetLineColor(2, zColor);
		graph_offsetPos->SetPosition(0, 256);
		graph_offsetPos->SetName(L"Offset Pos");

		graph_targetOffsetPos->SetLineColor(0, xColor);
		graph_targetOffsetPos->SetLineColor(1, yColor);
		graph_targetOffsetPos->SetLineColor(2, zColor);
		graph_targetOffsetPos->SetPosition(0, 384);
		graph_targetOffsetPos->SetName(L"Target Offset Pos");

		graph_rotation->SetLineColor(0, xColor);
		graph_rotation->SetLineColor(1, yColor);
		graph_rotation->SetPosition(0, 512);
		graph_rotation->SetName(L"Current Rotation (Pitch, Yaw)");

		graph_tpsRotation->SetLineColor(0, xColor);
		graph_tpsRotation->SetLineColor(1, yColor);
		graph_tpsRotation->SetLineColor(2, zColor);
		graph_tpsRotation->SetLineColor(3, { 1.0f, 1.0f, 0.0f, 1.0f });
		graph_tpsRotation->SetPosition(0, 640);
		graph_tpsRotation->SetName(L"TPS Rotation (Yaw1, Yaw2, Yaw), camera->lookYaw");

		graph_computeTime->SetLineColor(0, { 1.0f, 0.266f, 0.984f, 1.0f });
		graph_computeTime->SetLineColor(1, { 1.0f, 0.541f, 0.218f, 1.0f });
		graph_computeTime->SetName(L"Compute Time <pink> (Camera::Update()), Frame Time <orange>, seconds");
		graph_computeTime->SetPosition(0, 768);

		Render::CBufferCreateInfo cbuf;
		cbuf.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		cbuf.cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbuf.size = sizeof(glm::mat4);
		cbuf.initialData = &orthoMatrix;
		perFrameBuffer = eastl::make_unique<Render::CBuffer>(cbuf, Render::GetContext());
	}
#endif
}

Camera::Thirdperson::~Thirdperson() {}

void Camera::Thirdperson::OnBegin(PlayerCharacter* player, CorrectedPlayerCamera* camera, ICamera* lastState) noexcept {
	currentFocusObject = m_camera->GetCurrentCameraTarget(camera);
	if (!currentFocusObject)
		currentFocusObject = player;

	if (!lastState || lastState->m_id == CameraID::Firstperson) {
		// We want to invalidate any interpolation state when exiting first-person
		// If lastState is null, we should do this too
		MoveToGoalPosition(player, camera);
	}
}

void Camera::Thirdperson::OnEnd(PlayerCharacter* player, CorrectedPlayerCamera* camera, ICamera* newState) noexcept {
	crosshair->Reset();
}

bool Camera::Thirdperson::OnPreGameUpdate(PlayerCharacter* player, CorrectedPlayerCamera* camera,
	BSTSmartPointer<TESCameraState>& nextState)
{
	if (Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken()) return false;

	const auto state = GameState::GetCameraState(player, camera);
	if (state != GameState::CameraState::Transitioning) return false;
	auto cstate = reinterpret_cast<CorrectedPlayerCameraTransitionState*>(
		camera->cameraStates[CorrectedPlayerCamera::kCameraState_Transition]
	);

	auto horse = camera->cameraStates[CorrectedPlayerCamera::kCameraState_Horse];
	auto tps = camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2];
	if (cstate->fromState == horse && cstate->toState == tps) {
		if (!GameState::IsFirstPerson(camera))
		{
			// Getting off a horse, MURDER THIS STATE
			// @TODO: Write our own transition for this state - It still looks nicer with it disabled anyways though
			if (nextState.ptr)
				InterlockedDecrement(&nextState.ptr->refCount.m_refCount);
			InterlockedIncrement(&cstate->toState->refCount.m_refCount);
			nextState.ptr = cstate->toState;

			auto tpsState = reinterpret_cast<CorrectedThirdPersonState*>(
				camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2]
			);

			auto horseState = reinterpret_cast<CorrectedThirdPersonState*>(
				camera->cameraStates[CorrectedPlayerCamera::kCameraState_Horse]
			);

			// Just copy a whole wack ton of stuff
			tpsState->offsetVector = horseState->offsetVector;
			tpsState->translation = horseState->translation;
			tpsState->zoom = horseState->zoom;
			tpsState->cameraZoom = horseState->cameraZoom;
			tpsState->cameraLastZoom = horseState->cameraLastZoom;
			tpsState->fOverShoulderPosX = horseState->fOverShoulderPosX;
			tpsState->fOverShoulderCombatAddY = horseState->fOverShoulderCombatAddY;
			tpsState->fOverShoulderPosZ = horseState->fOverShoulderPosZ;
			tpsState->controllerNode->m_worldTransform = tpsState->controllerNode->m_oldWorldTransform;
			MoveToGoalPosition(player, camera);
			return true;
		}
	}

	return false;
}

void Camera::Thirdperson::UpdateInterpolators(PlayerCharacter* player, CorrectedPlayerCamera* camera) noexcept {
	currentFocusObject = m_camera->GetCurrentCameraTarget(camera);
	if (!currentFocusObject)
		currentFocusObject = player;

	const auto currentOffset = GetCurrentCameraOffset(player, camera);
	const auto curTime = GameTime::CurTime();

	// Update ref position
	currentPosition.SetRef(currentFocusObject->pos, currentFocusObject->rot);

	// Update transition states
	mmath::UpdateTransitionState<glm::vec3, OffsetTransition>(
		curTime,
		config->enableOffsetInterpolation,
		config->offsetInterpDurationSecs,
		config->offsetScalar,
		offsetTransitionState,
		{ currentOffset.x, currentOffset.y, currentOffset.z }
	);

	if (!povWasPressed && !povTransitionState.running) {
		mmath::UpdateTransitionState<float, ZoomTransition>(
			curTime,
			config->enableZoomInterpolation,
			config->zoomInterpDurationSecs,
			config->zoomScalar,
			zoomTransitionState,
			GetCurrentCameraDistance(camera)
		);
	} else {
		zoomTransitionState.lastPosition = zoomTransitionState.currentPosition =
			zoomTransitionState.targetPosition = GetCurrentCameraDistance(camera);
	}

	mmath::UpdateTransitionState<float, ZoomTransition>(
		curTime,
		config->enableFOVInterpolation,
		config->fovInterpDurationSecs,
		config->fovScalar,
		fovTransitionState,
		currentOffset.w
	);

	offsetState.position = {
		offsetTransitionState.currentPosition.x,
		offsetTransitionState.currentPosition.y,
		offsetTransitionState.currentPosition.z
	};
	offsetState.fov = fovTransitionState.currentPosition;
	
	UpdateOffsetSmoothers(player, curTime);
}

void Camera::Thirdperson::OnUpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera,
	BSTSmartPointer<TESCameraState>& nextState)
{
#ifdef WITH_CHARTS
	Profiler prof;
#endif
	const auto wantsControl = Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken();
	const auto wantsUpdates = Messaging::SmoothCamInterface::GetInstance()->WantsInterpolatorUpdates();

	// Update POV switch smoothing
	if (!Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken())
		switch (m_camera->GetCurrentCameraState()) {
			case GameState::CameraState::ThirdPerson:
			case GameState::CameraState::ThirdPersonCombat: {
				if (UpdatePOVSwitchState(camera, PlayerCamera::kCameraState_ThirdPerson2)) return;
				break;
			}
			case GameState::CameraState::Horseback: {
				if (UpdatePOVSwitchState(camera, PlayerCamera::kCameraState_Horse)) return;
				break;
			}
			case GameState::CameraState::Dragon: {
				if (UpdatePOVSwitchState(camera, PlayerCamera::kCameraState_Dragon)) return;
				break;
			}
			case GameState::CameraState::Bleedout: {
				if (UpdatePOVSwitchState(camera, PlayerCamera::kCameraState_Bleedout)) return;
				break;
			}
		}

	// Invalidate while we are in a loading screen state
	// Also if we were in the dialog menu - I'm not able to replicate this particular issue but it doesn't hurt
	// to do it in this case also.
	if (m_camera->InLoadingScreen() || m_camera->wasDialogOpen)
		crosshair->InvalidateEnablementCache();

	// Update our current crosshair selection if required
	if (Render::HasContext() && config->useWorldCrosshair)
		crosshair->Set3DCrosshairType(config->worldCrosshairType);

	currentFocusObject = m_camera->GetCurrentCameraTarget(camera);
	if (!currentFocusObject)
		currentFocusObject = player;

	offsetState.currentGroup = GetOffsetForState(m_camera->GetCurrentCameraActionState());

	// Don't continue to update transition states if we aren't running update code
	bool shouldRun = false;
	switch (m_camera->GetCurrentCameraState()) {
		case GameState::CameraState::ThirdPerson:
		case GameState::CameraState::ThirdPersonCombat:
		case GameState::CameraState::Horseback:
			shouldRun = true;
			break;
		default: break;
	}

	// And set our current reference position
	if (m_camera->wasLoading || (!wantsControl && m_camera->wasCameraAPIControlled &&
		Messaging::SmoothCamInterface::GetInstance()->WantsMoveToGoal()))
	{
		// Exiting a loading screen or returning from API control
		MoveToGoalPosition(player, camera);
		Messaging::SmoothCamInterface::GetInstance()->ClearMoveToGoalFlag();
	}

	// Save our last position
	lastPosition = currentPosition;
	
	if (!shouldRun) {
		//

	} else {
		UpdateInterpolators(player, camera);

		// Now run the active camera state
		if (!wantsControl || (wantsControl && wantsUpdates)) {
			SetFOVOffset(offsetState.fov);

			switch (m_camera->GetCurrentCameraState()) {
				case GameState::CameraState::ThirdPerson: {
					UpdateInternalRotation(camera);
					auto& state = cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson));
					state->Update(player, currentFocusObject, camera);
#ifdef WITH_D2D
					stateOverlay->SetThirdPersonState(state.get());
#endif
					break;
				}
				case GameState::CameraState::ThirdPersonCombat: {
					UpdateInternalRotation(camera);
					auto& state = cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat));
					state->Update(player, currentFocusObject, camera);
#ifdef WITH_D2D
					stateOverlay->SetThirdPersonState(state.get());
#endif
					break;
				}
				case GameState::CameraState::Horseback: {
					UpdateInternalRotation(camera);
					auto& state = cameraStates.at(static_cast<size_t>(GameState::CameraState::Horseback));
					state->Update(player, currentFocusObject, camera);
#ifdef WITH_D2D
					stateOverlay->SetThirdPersonState(state.get());
#endif
					break;
				}
				default: {
#ifdef WITH_D2D
					stateOverlay->SetThirdPersonState(nullptr);
#endif
					break;
				}
			}
		}
	}

	povWasPressed = false;

	if (!m_camera->InLoadingScreen())
		crosshair->Update(player, camera);

#ifdef WITH_CHARTS
	lastProfSnap = prof.Snap();
#endif
}

void Camera::Thirdperson::Render(Render::D3DContext& ctx) noexcept {
	if (m_camera->InLoadingScreen()) return;
	crosshair->Render(ctx, currentPosition.world, rotation.euler, m_camera->GetFrustum());

#ifdef WITH_CHARTS
	if (!currentFocusObject) return;
	if (GetAsyncKeyState(VK_UP)) {
		if (!dbgKeyDown) {
			switch (curDebugMode) {
				case DisplayMode::None:
					curDebugMode = DisplayMode::Graphs;
					break;
				case DisplayMode::Graphs:
					curDebugMode = DisplayMode::NodeTree;
					break;
				case DisplayMode::NodeTree:
					curDebugMode = DisplayMode::StateOverlay;
					break;
				case DisplayMode::StateOverlay:
					curDebugMode = DisplayMode::None;
					break;
			}
			dbgKeyDown = true;
		}
	} else {
		dbgKeyDown = false;
	}

	const auto& size = ctx.windowSize;
	orthoMatrix = glm::ortho(0.0f, size.x, size.y, 0.0f);
	perFrameBuffer->Update(&orthoMatrix, 0, sizeof(glm::mat4), ctx);
	perFrameBuffer->Bind(Render::PipelineStage::Vertex, 1, ctx);

	switch (curDebugMode) {
		case DisplayMode::None:
			break;

		case DisplayMode::Graphs: {
			const auto worldPos = GetCurrentCameraTargetWorldPosition(currentFocusObject, CorrectedPlayerCamera::GetSingleton());
			graph_worldPosTarget->AddPoint(0, worldPos.x);
			graph_worldPosTarget->AddPoint(1, worldPos.y);
			graph_worldPosTarget->AddPoint(2, worldPos.z);

			graph_localSpace->AddPoint(0, lastPosition.local.x);
			graph_localSpace->AddPoint(1, lastPosition.local.y);
			graph_localSpace->AddPoint(2, lastPosition.local.z);

			graph_offsetPos->AddPoint(0, offsetState.position.x);
			graph_offsetPos->AddPoint(1, offsetState.position.y);
			graph_offsetPos->AddPoint(2, offsetState.position.z);

			const auto ofsPos = GetCurrentCameraOffset(*g_thePlayer, CorrectedPlayerCamera::GetSingleton());
			graph_targetOffsetPos->AddPoint(0, ofsPos.x);
			graph_targetOffsetPos->AddPoint(1, ofsPos.y);
			graph_targetOffsetPos->AddPoint(2, ofsPos.z);

			graph_rotation->AddPoint(0, rotation.euler.x);
			graph_rotation->AddPoint(1, rotation.euler.y);

			if (GameState::IsThirdPerson(CorrectedPlayerCamera::GetSingleton())) {
				auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(CorrectedPlayerCamera::GetSingleton()->cameraState);
				graph_tpsRotation->AddPoint(0, tps->yaw1);
				graph_tpsRotation->AddPoint(1, tps->yaw2);
				graph_tpsRotation->AddPoint(2, tps->yaw);
				graph_tpsRotation->AddPoint(3, CorrectedPlayerCamera::GetSingleton()->lookYaw);
			}

			graph_computeTime->AddPoint(0, lastProfSnap);
			graph_computeTime->AddPoint(1, static_cast<float>(GameTime::GetFrameDelta()));

			graph_worldPosTarget->Draw(ctx);
			graph_localSpace->Draw(ctx);
			graph_offsetPos->Draw(ctx);
			graph_targetOffsetPos->Draw(ctx);
			graph_rotation->Draw(ctx);
			graph_tpsRotation->Draw(ctx);
			graph_computeTime->Draw(ctx);

			break;
		}
		case DisplayMode::NodeTree: {
			focusTargetNodeTree->SetPosition(0, 0);
			focusTargetNodeTree->SetSize(size.x, size.y);
			if (currentFocusObject->loadedState->node)
				focusTargetNodeTree->Draw(ctx, currentFocusObject->loadedState->node);
			break;
		}
		case DisplayMode::StateOverlay: {
			stateOverlay->SetPosition((size.x / 2) - 300, size.y - 600);
			stateOverlay->SetSize(600, 400);
			stateOverlay->Draw(currentFocusObject, offsetState.currentGroup, ctx);
			break;
		}
	}
#endif
}

void Camera::Thirdperson::OnTogglePOV(const ButtonEvent* ev) noexcept {
	povWasPressed = true;
}

bool Camera::Thirdperson::OnKeyPress(const ButtonEvent* ev) noexcept {
	auto code = ev->keyMask;
	if (code <= 0x6 && ev->deviceType == kDeviceType_Mouse)
		code += 0x100;

	if (config->shoulderSwapKey >= 0 && config->shoulderSwapKey == code && ev->timer <= 0.000001f) {
		shoulderSwap = shoulderSwap == 1 ? -1 : 1;
		return true;
	} else if (config->applyZOffsetKey >= 0 && config->applyZOffsetKey == code && ev->timer <= 0.000001f) {
		config->zOffsetActive = !config->zOffsetActive;
	} else if (config->toggleUserDefinedOffsetGroupKey >= 0 && config->toggleUserDefinedOffsetGroupKey == code && ev->timer <= 0.000001f) {
		config->userDefinedOffsetActive = !config->userDefinedOffsetActive;
	}

	return false;
}

bool Camera::Thirdperson::OnMenuOpenClose(MenuID id, const MenuOpenCloseEvent* const ev) noexcept {
	switch (id) {
		case MenuID::MapMenu: {
			SetFOVOffset(0.0f, true);
			return true;
		}
	}
	return false;
}

void Camera::Thirdperson::OnCameraActionStateTransition(const PlayerCharacter* player, const CameraActionState newState,
	const CameraActionState oldState) noexcept
{

}

void Camera::Thirdperson::OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const GameState::CameraState newState, const GameState::CameraState oldState) noexcept
{
	auto& oldCameraState = cameraStates.at(static_cast<size_t>(oldState));
	auto& newCameraState = cameraStates.at(static_cast<size_t>(newState));

	switch (oldState) {
		case GameState::CameraState::UsingObject: {
			MoveToGoalPosition(player, camera);
			break;
		}
		case GameState::CameraState::FirstPerson: {
			break;
		}
		case GameState::CameraState::ThirdPerson: {
			oldCameraState->OnEnd(player, currentFocusObject, camera, newCameraState.get());
			break;
		}
		case GameState::CameraState::ThirdPersonCombat: {
			oldCameraState->OnEnd(player, currentFocusObject, camera, newCameraState.get());
			break;
		}
		case GameState::CameraState::Horseback: {
			oldCameraState->OnEnd(player, currentFocusObject, camera, newCameraState.get());

			if (newState == GameState::CameraState::Tweening) {
				// The game clears horseYaw to 0 when going from tween back to horseback, which results in a rotation jump
				// once the horse starts moving again. We can store the horse yaw value here to restore later.
				stateCopyData.horseYaw = reinterpret_cast<CorrectedHorseCameraState*>(
					camera->cameraStates[CorrectedPlayerCamera::kCameraState_Horse]
				)->horseYaw;

			} else if (newState == GameState::CameraState::ThirdPerson) {
				MoveToGoalPosition(player, camera);
			}

			break;
		}
		case GameState::CameraState::Tweening:
		case GameState::CameraState::Transitioning: {
			// Don't copy positions from these states
			break;
		}
		default:
			break;
	}

	switch (newState) {
		case GameState::CameraState::ThirdPerson: {
			newCameraState->OnBegin(player, currentFocusObject, camera, oldCameraState.get());
			break;
		}
		case GameState::CameraState::ThirdPersonCombat: {
			newCameraState->OnBegin(player, currentFocusObject, camera, oldCameraState.get());
			break;
		}
		case GameState::CameraState::Horseback: {
			newCameraState->OnBegin(player, currentFocusObject, camera, oldCameraState.get());

			if (oldState == GameState::CameraState::Tweening) {
				// Fix annoying horse rotation reset
				reinterpret_cast<CorrectedHorseCameraState*>(
					camera->cameraStates[CorrectedPlayerCamera::kCameraState_Horse]
				)->horseYaw = stateCopyData.horseYaw;
			}

			break;
		}
		case GameState::CameraState::Free: {
			// Forward our position to the free cam state
			auto state = reinterpret_cast<FreeCameraState*>(camera->cameraState);
			state->unk30[0] = lastPosition.world.x;
			state->unk30[1] = lastPosition.world.y;
			state->unk30[2] = lastPosition.world.z;

			const auto quat = rotation.ToNiQuat();

			//FUN_140848880((longlong)ppuVar3,local_18);
			typedef void(__fastcall* UpdateFreeCamTransform)(FreeCameraState*, const NiQuaternion&);
			Offsets::Get<UpdateFreeCamTransform>(49816)(state, quat);
		}
		[[fallthrough]];
		default: {
			// Do this once here on transition to a new state we don't run in
			crosshair->Reset();
			break;
		}
	}
}

glm::vec3 Camera::Thirdperson::GetCurrentCameraTargetWorldPosition(const TESObjectREFR* ref,
	const CorrectedPlayerCamera* camera) const
{
	if (ref->loadedState && ref->loadedState->node && Strings.spine1.data) {
		NiAVObject* node;
		if (m_camera->currentState == GameState::CameraState::Horseback)
			node = ref->loadedState->node->GetObjectByName(&Strings.spine1.data);
		else
			node = FindFollowBone(ref);

		if (node)
			return glm::vec3(
				ref->pos.x,
				ref->pos.y,
				node->m_worldTransform.pos.z
			);
	}

	return {
		ref->pos.x,
		ref->pos.y,
		ref->pos.z
	};
}

void Camera::Thirdperson::GetCameraGoalPosition(const CorrectedPlayerCamera* camera, glm::vec3& world, glm::vec3& local,
	const TESObjectREFR* forRef)
{
	const auto cameraLocal = offsetState.position;
	auto translated = rotation.ToRotationMatrix() * glm::vec4(
		cameraLocal.x,
		cameraLocal.y - config->minCameraFollowDistance + zoomTransitionState.currentPosition,
		0.0f,
		1.0f
	);
	translated.z += cameraLocal.z;

	local = static_cast<glm::vec3>(translated);
	world = 
		GetCurrentCameraTargetWorldPosition(forRef ? forRef : currentFocusObject, camera) +
		local;
}

glm::vec2 Camera::Thirdperson::GetAimRotation(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) const {
	if (GameState::IsInHorseCamera(ref, camera)) {
		// In horse camera, we need to clamp our local y axis
		const auto yaw = ref->rot.z + glm::pi<float>(); // convert to 0-tau
		const auto yawLocal = glm::mod(rotation.euler.y - yaw, glm::pi<float>() * 2.0f); // confine to tau
		const auto clampedLocal = glm::clamp(yawLocal, mmath::half_pi, mmath::half_pi*3.0f); // clamp it to faceforward zone

		return {
			rotation.euler.x,
			yaw + clampedLocal
		};
	} else {
		const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2]);
		if ((tps && tps->freeRotationEnabled) || GameState::InPOVSlideMode()) {
			// In free rotation mode aim yaw is locked to player rotation
			// POVSlideMode - The character stays still while the camera orbits

			// @Note: PR #48: Hopefully this still works correctly as both cases are checked,
			// with POVSlideMode checked last
			return {
				rotation.euler.x,
				ref->rot.z
			};
		}
	}

	return rotation.euler;
}

const mmath::Rotation& Camera::Thirdperson::GetCameraRotation() const noexcept {
	return rotation;
}

void Camera::Thirdperson::SetPosition(const glm::vec3& pos, const CorrectedPlayerCamera* camera) noexcept {
	// If an API consumer is controlling the camera and has requested updates, we don't want to be setting the position
	if (Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken()) return;

	m_camera->SetPosition(pos, camera);
	m_camera->UpdateInternalWorldToScreenMatrix(rotation, camera);
}

const mmath::Position& Camera::Thirdperson::GetPosition() const noexcept {
	return lastPosition;
}

Crosshair::Manager* Camera::Thirdperson::GetCrosshairManager() noexcept {
	return crosshair.get();
}

void Camera::Thirdperson::MoveToGoalPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const TESObjectREFR* forRef, NiCamera* niCamera) noexcept
{
	const auto pov = m_camera->UpdateCameraPOVState(player, camera);
	const auto actionState = m_camera->UpdateCurrentCameraActionState(player, camera);
	offsetState.currentGroup = GetOffsetForState(actionState);

	auto ofs = GetCurrentCameraOffset(player, camera);
	zoomTransitionState.currentPosition = zoomTransitionState.lastPosition = GetCurrentCameraDistance(camera);
	zoomTransitionState.running = false;

	offsetTransitionState.currentPosition = offsetTransitionState.lastPosition = {
		ofs.x,
		ofs.y,
		ofs.z
	};
	offsetTransitionState.running = false;

	fovTransitionState.currentPosition = fovTransitionState.lastPosition = ofs.w;
	fovTransitionState.running = false;

	UpdateInternalRotation(camera);
	GetCameraGoalPosition(camera, currentPosition.world, currentPosition.local, forRef);
	lastPosition = currentPosition;
}

void Camera::Thirdperson::UpdateInternalRotation(const CorrectedPlayerCamera* camera) noexcept {
	CorrectedThirdPersonState* tps;

	// this is true when dismounting a horse, the camera is transitioning between the horseback and 3rd person camera
	// see OnPreGameUpdate for setting the transition states
	if (camera->cameraState == camera->cameraStates[CorrectedPlayerCamera::kCameraState_Transition]) {
		auto cstate = reinterpret_cast<CorrectedPlayerCameraTransitionState*>(camera->cameraState);
		tps = reinterpret_cast<CorrectedThirdPersonState*>(cstate->toState);
	} else if (camera->cameraState == camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2]) {
		tps = reinterpret_cast<CorrectedThirdPersonState*>(camera->cameraState);
	} else {
		return;
	}

	if (!tps) return;

	// Alright, just lie and force the game to compute yaw for us
	// We get some jitter when things like magic change our character, not sure why yet @TODO
	auto last = tps->freeRotationEnabled;
	tps->freeRotationEnabled = true;
	tps->UpdateRotation();
		rotation.SetQuaternion(tps->rotation);
	tps->freeRotationEnabled = last;
	tps->UpdateRotation();
}

NiAVObject* Camera::Thirdperson::FindFollowBone(const TESObjectREFR* ref) const noexcept {
	if (!ref->loadedState || !ref->loadedState->node) return nullptr;
	auto& boneList = Config::GetBonePriorities();

	for (auto it = boneList.begin(); it != boneList.end(); it++) {
		auto node = ref->loadedState->node->GetObjectByName(&it->data);
		if (node) return node;
	}

	return nullptr;
}

float Camera::Thirdperson::GetCurrentCameraDistance(const CorrectedPlayerCamera* camera) const noexcept {
	return -(GetCurrentCameraZoom(camera, m_camera->currentState) * config->zoomMul);
}

float Camera::Thirdperson::GetCurrentCameraZoom(const CorrectedPlayerCamera* camera,
	const GameState::CameraState currentState) const noexcept
{
	switch (currentState) {
		case GameState::CameraState::ThirdPerson:
		case GameState::CameraState::ThirdPersonCombat: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_ThirdPerson2);
		}
		case GameState::CameraState::Horseback: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_Horse);
		}
		case GameState::CameraState::Dragon: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_Dragon);
		}
		case GameState::CameraState::Bleedout: {
			return GetCameraZoomScalar(camera, PlayerCamera::kCameraState_Bleedout);
		}
		default:
			return 0.0f;
	}
}

float Camera::Thirdperson::GetCameraZoomScalar(const CorrectedPlayerCamera* camera, uint16_t cameraState) const noexcept {
	const auto state = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraStates[cameraState]);
	if (!state) return 0.0f;

	auto minZoom = 0.2f;
	static auto minZoomConf = (*g_iniSettingCollection)->Get("fMinCurrentZoom:Camera");
	if (minZoomConf) minZoom = minZoomConf->data.f32;

	return state->cameraZoom + (minZoom *-1);
}

bool Camera::Thirdperson::IsInterpAllowed(const Actor* player) const noexcept {
	auto ofs = offsetState.currentGroup;
	if (m_camera->currentState == GameState::CameraState::Horseback && !config->userDefinedOffsetActive) {
		if (GameState::IsBowDrawn(player)) {
			return config->bowAim.interpHorseback;
		} else {
			ofs = &config->horseback;
		}
	}

	if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive) {
		if (GameState::IsBowDrawn(player)) {
			return config->bowAim.interpMeleeCombat;
		}
	}

	if (!GameState::IsWeaponDrawn(player)) return ofs->interp;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return ofs->interpRangedCombat;
	}
	if (GameState::IsMagicDrawn(player)) {
		return ofs->interpMagicCombat;
	}
	return ofs->interpMeleeCombat;
}

bool Camera::Thirdperson::IsInterpOverloaded(const Actor* player) const noexcept {
	auto ofs = offsetState.currentGroup;
	if (m_camera->currentState == GameState::CameraState::Horseback && !config->userDefinedOffsetActive) {
		if (GameState::IsBowDrawn(player)) {
			return config->bowAim.interpHorsebackConf.overrideInterp;
		} else {
			ofs = &config->horseback;
		}
	}

	if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive) {
		if (GameState::IsBowDrawn(player)) {
			return config->bowAim.interpMeleeConf.overrideInterp;
		}
	}

	if (!GameState::IsWeaponDrawn(player)) return ofs->interpConf.overrideInterp;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return ofs->interpRangedConf.overrideInterp;
	}
	if (GameState::IsMagicDrawn(player)) {
		return ofs->interpMagicConf.overrideInterp;
	}
	return ofs->interpMeleeConf.overrideInterp;
}

bool Camera::Thirdperson::IsLocalInterpOverloaded(const Actor* player) const noexcept {
	auto ofs = offsetState.currentGroup;
	if (m_camera->currentState == GameState::CameraState::Horseback && !config->userDefinedOffsetActive) {
		if (GameState::IsBowDrawn(player)) {
			return config->bowAim.interpHorsebackConf.overrideLocalInterp;
		} else {
			ofs = &config->horseback;
		}
	}

	if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive) {
		if (GameState::IsBowDrawn(player)) {
			return config->bowAim.interpMeleeConf.overrideLocalInterp;
		}
	}

	if (!GameState::IsWeaponDrawn(player)) return ofs->interpConf.overrideLocalInterp;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return ofs->interpRangedConf.overrideLocalInterp;
	}
	if (GameState::IsMagicDrawn(player)) {
		return ofs->interpMagicConf.overrideLocalInterp;
	}
	return ofs->interpMeleeConf.overrideLocalInterp;
}

const Config::OffsetGroupScalar* Camera::Thirdperson::GetCurrentScalarGroup(const Actor* player,
	const Config::OffsetGroup* currentGroup) const noexcept
{
	if (m_camera->currentState == GameState::CameraState::Horseback && !config->userDefinedOffsetActive) {
		if (GameState::IsBowDrawn(player)) {
			// Aiming with a bow on horseback
			return &currentGroup->interpHorsebackConf;
		}
	}

	if (GameState::IsBowDrawn(player)) {
		if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive) // Aiming with a bow while sneaking
			return &currentGroup->interpMeleeConf;
		else
			return &currentGroup->interpRangedConf; // All other bow aim cases use ranged

	} else if (GameState::IsMagicDrawn(player)) {
		return &currentGroup->interpMagicConf;
	} else if (GameState::IsRangedWeaponDrawn(player)) {
		return &currentGroup->interpRangedConf;
	} else if (GameState::IsWeaponDrawn(player)) {
		return &currentGroup->interpMeleeConf;
	} else {
		return &currentGroup->interpConf;
	}
}

const Config::OffsetGroup* Camera::Thirdperson::GetOffsetForState(const CameraActionState state) const noexcept {
	if (config->userDefinedOffsetActive)
		return &config->userDefined;

	switch (state) {
		case CameraActionState::Horseback: {
			return &config->horseback;
		}
		case CameraActionState::Dragon: {
			return &config->dragon;
		}
		case CameraActionState::VampireLord: {
			return &config->vampireLord;
		}
		case CameraActionState::Werewolf: {
			return &config->werewolf;
		}
		case CameraActionState::DisMounting: {
			return &config->standing; // Better when dismounting
		}
		case CameraActionState::Sleeping: {
			return &config->sitting;
		}
		case CameraActionState::Sitting: {
			return &config->sitting;
		}
		case CameraActionState::Sneaking: {
			return &config->sneaking;
		}
		case CameraActionState::Aiming: {
			return &config->bowAim;
		}
		case CameraActionState::Swimming: {
			return &config->swimming;
		}
		case CameraActionState::Sprinting: {
			return &config->sprinting;
		}
		case CameraActionState::Walking: {
			return &config->walking;
		}
		case CameraActionState::Running: {
			return &config->running;
		}
		case CameraActionState::Standing: {
			return &config->standing;
		}
		default: {
			return &config->standing;
		}
	}
}

float Camera::Thirdperson::GetActiveWeaponStateForwardOffset(const Actor* player,
	const Config::OffsetGroup* group) const noexcept
{
	if (!GameState::IsWeaponDrawn(player)) return group->zoomOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedZoomOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicZoomOffset;
	}
	return group->combatMeleeZoomOffset;
}

float Camera::Thirdperson::GetActiveWeaponStateUpOffset(const Actor* player,
	const Config::OffsetGroup* group) const noexcept
{
	if (!GameState::IsWeaponDrawn(player)) return group->upOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedUpOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicUpOffset;
	}
	return group->combatMeleeUpOffset;
}

float Camera::Thirdperson::GetActiveWeaponStateSideOffset(const Actor* player,
	const Config::OffsetGroup* group) const noexcept
{
	if (!GameState::IsWeaponDrawn(player)) return group->sideOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedSideOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicSideOffset;
	}
	return group->combatMeleeSideOffset;
}

float Camera::Thirdperson::GetActiveWeaponStateFOVOffset(const Actor* player,
	const Config::OffsetGroup* group) const noexcept
{
	if (!GameState::IsWeaponDrawn(player)) return group->fovOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedFOVOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicFOVOffset;
	}
	return group->combatMeleeFOVOffset;
}

float Camera::Thirdperson::GetCurrentCameraForwardOffset(const Actor* player) const noexcept {
	if (!config->userDefinedOffsetActive)
		switch (m_camera->currentState) {
			case GameState::CameraState::Horseback: {
				if (GameState::IsBowDrawn(player)) {
					return config->bowAim.horseZoomOffset;
				} else {
					return GetActiveWeaponStateForwardOffset(player, &config->horseback);
				}
			}
			default:
				break;
		}

	switch (m_camera->currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->zoomOffset;
		}
		case CameraActionState::Aiming: {
			if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive)
				return config->bowAim.combatMeleeZoomOffset;
			return offsetState.currentGroup->zoomOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::VampireLord:
		case CameraActionState::Werewolf:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateForwardOffset(player, offsetState.currentGroup);
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

float Camera::Thirdperson::GetCurrentCameraHeight(const Actor* player) const noexcept {
	if (!config->userDefinedOffsetActive)
		switch (m_camera->currentState) {
			case GameState::CameraState::Horseback: {
				if (GameState::IsBowDrawn(player)) {
					return config->bowAim.horseUpOffset;
				} else {
					return GetActiveWeaponStateUpOffset(player, &config->horseback);
				}
			}
			default:
				break;
		}

	switch (m_camera->currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->upOffset;
		}
		case CameraActionState::Aiming: {
			if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive)
				return config->bowAim.combatMeleeUpOffset;
			return offsetState.currentGroup->upOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::VampireLord:
		case CameraActionState::Werewolf:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateUpOffset(player, offsetState.currentGroup);
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

float Camera::Thirdperson::GetCurrentCameraSideOffset(const Actor* player,
	const CorrectedPlayerCamera* camera) const noexcept
{
	if (!config->userDefinedOffsetActive)
		switch (m_camera->currentState) {
			case GameState::CameraState::Horseback: {
				if (GameState::IsBowDrawn(player)) {
					return config->bowAim.horseSideOffset * shoulderSwap;
				} else {
					return GetActiveWeaponStateSideOffset(player, &config->horseback) * shoulderSwap;
				}
			}
			default:
				break;
		}

	switch (m_camera->currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->sideOffset * shoulderSwap;
		}
		case CameraActionState::Aiming: {
			if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive)
				return config->bowAim.combatMeleeSideOffset * shoulderSwap;
			return offsetState.currentGroup->sideOffset * shoulderSwap;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::VampireLord:
		case CameraActionState::Werewolf:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateSideOffset(player, offsetState.currentGroup) * shoulderSwap;
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

float Camera::Thirdperson::GetCurrentCameraFOVOffset(const Actor* player) const noexcept {
	if (!config->userDefinedOffsetActive)
		switch (m_camera->currentState) {
			case GameState::CameraState::Horseback: {
				if (GameState::IsBowDrawn(player)) {
					return config->bowAim.horseFOVOffset;
				} else {
					return GetActiveWeaponStateFOVOffset(player, &config->horseback);
				}
			}
			default:
				break;
		}

	switch (m_camera->currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->fovOffset;
		}
		case CameraActionState::Aiming:{
			if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive)
				return config->bowAim.combatMeleeFOVOffset;
			return offsetState.currentGroup->fovOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::VampireLord:
		case CameraActionState::Werewolf:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateFOVOffset(player, offsetState.currentGroup);
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

glm::vec4 Camera::Thirdperson::GetCurrentCameraOffset(const Actor* player,
	const CorrectedPlayerCamera* camera) const noexcept
{
	return {
		GetCurrentCameraSideOffset(player, camera),
		GetCurrentCameraForwardOffset(player),
		GetCurrentCameraHeight(player) + (config->zOffsetActive ? config->customZOffset : 0.0f),
		GetCurrentCameraFOVOffset(player)
	};
}

double Camera::Thirdperson::GetCurrentSmoothingScalar(const Actor* player, const float distance,
	ScalarSelector method) const
{
	Config::ScalarMethods scalarMethod;

	// Work in FP64 here to eek out some more precision
	// Avoid a divide-by-zero error by clamping to this lower bound
	constexpr const double minZero = 0.000000000001;

	double scalar = 1.0;
	double interpValue = 1.0;
	double remapped = 1.0;

	if (method == ScalarSelector::SepZ) {
		const auto max = static_cast<double>(config->separateZMaxSmoothingDistance);
		scalar = glm::clamp(glm::max(1.0 - (max - distance), minZero) / max, 0.0, 1.0);
		remapped = mmath::Remap<double>(
			scalar, 0.0, 1.0, static_cast<double>(config->separateZMinFollowRate), static_cast<double>(config->separateZMaxFollowRate)
		);
		scalarMethod = config->separateZScalar;
	} else if (method == ScalarSelector::LocalSpace) {
		auto max = static_cast<double>(config->localMaxSmoothingDistance);
		auto minFollow = static_cast<double>(config->localMinFollowRate);
		auto maxFollow = static_cast<double>(config->localMaxFollowRate);
		scalarMethod = config->separateLocalScalar;

		if (IsLocalInterpOverloaded(player)) {
			const auto group = GetCurrentScalarGroup(player, offsetState.currentGroup);
			max = static_cast<double>(group->localMaxSmoothingDistance);
			minFollow = static_cast<double>(group->localMinFollowRate);
			maxFollow = static_cast<double>(group->localMaxFollowRate);
			scalarMethod = group->separateLocalScalar;
		}

		scalar = glm::clamp(glm::max(1.0 - (max - distance), minZero) / max, 0.0, 1.0);
		remapped = mmath::Remap<double>(
			scalar, 0.0, 1.0, minFollow, maxFollow
		);
		
	} else {
		const auto max = static_cast<double>(config->zoomMaxSmoothingDistance);
		scalar = glm::clamp(glm::max(1.0 - (max - distance), minZero) / max, 0.0, 1.0);
		remapped = mmath::Remap<double>(
			scalar, 0.0, 1.0, static_cast<double>(config->minCameraFollowRate), static_cast<double>(config->maxCameraFollowRate)
		);
		scalarMethod = config->currentScalar;
	}

	if (!config->disableDeltaTime) {
		const double delta = glm::max(GameTime::GetFrameDelta(), minZero);
		const double fps = 1.0 / delta;
		const double mul = -fps * glm::log2(1.0 - remapped);
		interpValue = glm::clamp(1.0 - glm::exp2(-mul * delta), 0.0, 1.0);
	} else {
		interpValue = remapped;
	}

	return mmath::RunScalarFunction<double>(scalarMethod, interpValue);
}

std::tuple<glm::vec3, glm::vec3> Camera::Thirdperson::GetDistanceClamping() const noexcept {
	float minsX = config->cameraDistanceClampXMin;
	float maxsX = config->cameraDistanceClampXMax;

	if (config->swapXClamping && shoulderSwap < 1) {
		std::swap(minsX, maxsX);
		maxsX *= -1.0f;
		minsX *= -1.0f;
	}

	return std::tie(
		glm::vec3{ minsX, config->cameraDistanceClampYMin, config->cameraDistanceClampZMin },
		glm::vec3{ maxsX, config->cameraDistanceClampYMax, config->cameraDistanceClampZMax }
	);
}

void Camera::Thirdperson::SetFOVOffset(float fov, bool force) noexcept {
	if (force) {
		// When in the map menu, this won't be reset, thus we have to force it.
		// Opening the map menu when zoomed in with a bow does the same thing.
		*Offsets::Get<float*>(527997) = fov;

		// This is caused by the reset code being the job of thirdperson cameras (Third, Horse, Dragon)
		// The map menu is a different camera

	} else {
		// FOV reset each frame relies on the controller node being valid
		// If this isn't the case, we should do it manually here
		// Controller node becomes null in certain cases - Undeath lich form, transformation spells, so on
		auto cam = CorrectedPlayerCamera::GetSingleton();
		auto curState = cam->cameraState;
		if (curState == cam->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2]) {
			if (reinterpret_cast<CorrectedThirdPersonState*>(curState)->controllerNode == nullptr)
				*Offsets::Get<float*>(527997) = 0.0f;

		} else if (curState == cam->cameraStates[CorrectedPlayerCamera::kCameraState_Horse]) {
			if (reinterpret_cast<CorrectedHorseCameraState*>(curState)->controllerNode == nullptr)
				*Offsets::Get<float*>(527997) = 0.0f;

		} else if (curState == cam->cameraStates[CorrectedPlayerCamera::kCameraState_Dragon]) {
			if (reinterpret_cast<DragonCameraState*>(curState)->controllerNode == nullptr)
				*Offsets::Get<float*>(527997) = 0.0f;
		}
	}

	if (fov == 0.0f) return;

	// This is an offset value applied to any call to SetFOV.
	// Used internally in the game by things like zooming with the bow.
	// Appears to be set by the game every frame so we can just add our offset on top like this.
	const auto baseValue = CorrectedPlayerCamera::GetSingleton()->worldFOV;
	const auto currentOffset = *Offsets::Get<float*>(527997);

	// We never want the FOV to go negative or past 180, so clamp it to a sane-ish range
	// (10-170)
	const auto actualFov = baseValue + currentOffset;
	constexpr auto fovMax = 170.0f;
	constexpr auto fovMin = 10.0f;

	// Break early if we are already past our limit
	if (actualFov >= fovMax) return;
	if (actualFov <= fovMin) return;

	// Compute the maximal range we can offset before hitting the limit
	const auto fullOffset = currentOffset + fov;
	const auto f = baseValue + fullOffset;
	auto finalOffset = fov;

	if (f > fovMax) {
		auto delta = f - fovMax;
		finalOffset -= delta;
	} else if (f < fovMin) {
		auto delta = fovMin - f;
		finalOffset += delta;
	}

	*Offsets::Get<float*>(527997) += finalOffset;
}

bool Camera::Thirdperson::UpdatePOVSwitchState(CorrectedPlayerCamera* camera, uint16_t cameraState) noexcept {
	auto state = reinterpret_cast<CorrectedThirdPersonState*>(camera->cameraStates[cameraState]);
	if (state->stateNotActive) {
		auto minZoom = 0.2f;
		static auto minZoomConf = (*g_iniSettingCollection)->Get("fMinCurrentZoom:Camera");
		if (minZoomConf) minZoom = minZoomConf->data.f32;
		const auto curTime = GameTime::CurTime();
		constexpr auto duration = 0.5f;

		// We are switching over to first person
		if (povWasPressed) {
			// We just started the switch
			povTransitionState.running = true;
			povTransitionState.startTime = curTime;
			povTransitionState.lastValue = state->cameraZoom = lastZoomValue;
		}

		// Switch running
		const auto scalar = glm::clamp(
			static_cast<float>(curTime - povTransitionState.startTime) / glm::max(duration, 0.01f),
			0.0f, 1.0f
		);
		povTransitionState.lastValue = mmath::Interpolate<float, float>(
			povTransitionState.lastValue,
			minZoom,
			mmath::RunScalarFunction<float>(Config::ScalarMethods::LINEAR, scalar)
		);

		lastZoomValue = state->cameraZoom = state->cameraLastZoom = povTransitionState.lastValue;

		// Switch mid-zoom
		if (scalar >= 0.5f) {
			povTransitionState.running = false;
			typedef void(*SwitchToFirstPerson)(CorrectedPlayerCamera*);
			Offsets::Get<SwitchToFirstPerson>(49858)(camera);
			return true;
		}

	} else {
		// Record the current camera zoom value for use during switches
		lastZoomValue = state->cameraZoom;
		povTransitionState.running = false;
	}

	return false;
}

void Camera::Thirdperson::UpdateOffsetSmoothers(const Actor* player, float curTime) noexcept {
	globalValues.currentScalar = config->currentScalar;
	globalValues.minCameraFollowRate = config->minCameraFollowRate;
	globalValues.maxCameraFollowRate = config->maxCameraFollowRate;
	globalValues.zoomMaxSmoothingDistance = config->zoomMaxSmoothingDistance;

	globalValues.separateLocalScalar = config->separateLocalScalar;
	globalValues.localMinFollowRate = config->localMinFollowRate;
	globalValues.localMaxFollowRate = config->localMaxFollowRate;
	globalValues.localMaxSmoothingDistance = config->localMaxSmoothingDistance;

	// Check if we need to move to a new scalar state
	const Config::OffsetGroupScalar* curS = GetCurrentScalarGroup(player, offsetState.currentGroup);

	const auto globalGroup = IsInterpOverloaded(player) ? curS : &globalValues;
	const auto localGroup = IsLocalInterpOverloaded(player) ? curS : &globalValues;

	// First run - enforce state
	if (!globalSmoother.hasEverUpdated)
		globalSmoother.MoveTo(globalGroup, config->globalInterpOverrideMethod, curTime, config->globalInterpOverrideSmoothing);

	if (!localSmoother.hasEverUpdated)
		localSmoother.MoveTo(localGroup, config->localInterpOverrideMethod, curTime, config->localInterpOverrideSmoothing);

	// Global
	if (globalSmoother.GetGoal() != globalGroup)
		globalSmoother.MoveTo(globalGroup, config->globalInterpOverrideMethod, curTime, config->globalInterpOverrideSmoothing);
	else
		globalSmoother.Update(curTime);

	// Local
	if (localSmoother.GetGoal() != localGroup)
		localSmoother.MoveTo(localGroup, config->localInterpOverrideMethod, curTime, config->localInterpOverrideSmoothing);
	else
		localSmoother.Update(curTime);
}