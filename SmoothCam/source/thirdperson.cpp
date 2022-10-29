#include "thirdperson.h"
#include "compat.h"
#include "debug/eh.h"

extern Offsets* g_Offsets;

Camera::Thirdperson::Thirdperson(Camera* baseCamera) : ICamera(baseCamera, CameraID::Thirdperson) {
	config = Config::GetCurrentConfig();
	crosshair = eastl::make_unique<Crosshair::Manager>();

	thirdPersonState = eastl::make_unique<State::ThirdpersonState>(this);
	thirdPersonDialogueState = eastl::make_unique<State::ThirdpersonDialogueState>(this);
	thirdPersonVanityState = eastl::make_unique<State::ThirdpersonVanityState>(this);

	cameraStates[static_cast<size_t>(GameState::CameraState::ThirdPerson)] = thirdPersonState.get();
	cameraStates[static_cast<size_t>(GameState::CameraState::ThirdPersonCombat)] = thirdPersonState.get();
	cameraStates[static_cast<size_t>(GameState::CameraState::Horseback)] = thirdPersonState.get();
	cameraStates[static_cast<size_t>(GameState::CameraState::ThirdPersonDialogue)] = thirdPersonDialogueState.get();
	cameraStates[static_cast<size_t>(GameState::CameraState::Vanity)] = thirdPersonVanityState.get();

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
		graph_computeTime = eastl::make_unique<Render::LineGraph>(3, 128, 600, 128, Render::GetContext());
		graph_fov = eastl::make_unique<Render::LineGraph>(2, 128, 600, 128, Render::GetContext());

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
		graph_computeTime->SetLineColor(2, { 1.0f, 0.0f, 0.0f, 1.0f });
		graph_computeTime->SetName(L"Compute Time <pink> (Camera::Update()), Frame Time <orange>, seconds, Smooth Frame Time <red>");
		graph_computeTime->SetPosition(0, 768);

		graph_fov->SetLineColor(0, { 0.0f, 1.0f, 0.0f, 1.0f });
		graph_fov->SetLineColor(1, { 1.0f, 0.0f, 0.0f, 1.0f });
		graph_fov->SetName(L"FOV (Desired, Observed)");
		graph_fov->SetPosition(0, 896);

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

void Camera::Thirdperson::OnBegin(RE::PlayerCharacter* player, RE::PlayerCamera* camera, ICamera* lastState) noexcept {
	currentFocusObject = m_camera->GetCurrentCameraTarget(camera).get();
	if (!currentFocusObject)
		currentFocusObject = player;

	if (!lastState || lastState->m_id == CameraID::Firstperson) {
		// We want to invalidate any interpolation state when exiting first-person
		// If lastState is null, we should do this too
		MoveToGoalPosition(player, currentFocusObject, camera);
	}

	State::BaseThird* state = nullptr;
	switch (m_camera->GetCurrentCameraState()) {
		case GameState::CameraState::ThirdPerson: [[fallthrough]];
		case GameState::CameraState::ThirdPersonCombat: [[fallthrough]];
		case GameState::CameraState::Horseback: {
			state = thirdPersonState.get();
			break;
		}
		case GameState::CameraState::ThirdPersonDialogue: {
			state = thirdPersonDialogueState.get();
			break;
		}
		default: {
			break;
		}
	}

	// End whatever state we had, unless it is the same as our new state
	if (runningState && state != runningState)
		runningState->OnEnd(player, currentFocusObject, camera, state);
	
	// Start our new state, if we have one
	if (state)
		state->OnBegin(player, currentFocusObject, camera, runningState);

	runningState = state;
}

void Camera::Thirdperson::OnEnd(RE::PlayerCharacter* player, RE::PlayerCamera* camera, ICamera*) noexcept {
	if (runningState)
		runningState->OnEnd(player, m_camera->currentFocusObject, camera, nullptr, true);
	runningState = nullptr;
	crosshair->Reset();

	// Restore some position data
	const auto combat = GameState::GetCameraState(m_camera->currentFocusObject, camera) == GameState::CameraState::ThirdPersonCombat;

	const auto x = combat ? RE::INISettingCollection::GetSingleton()->GetSetting("fOverShoulderCombatPosX:Camera") :
		RE::INISettingCollection::GetSingleton()->GetSetting("fOverShoulderPosX:Camera");

	const auto y = combat ?
		RE::INISettingCollection::GetSingleton()->GetSetting("fOverShoulderCombatAddY:Camera") :
		nullptr;

	const auto z = combat ? RE::INISettingCollection::GetSingleton()->GetSetting("fOverShoulderCombatPosZ:Camera") :
		RE::INISettingCollection::GetSingleton()->GetSetting("fOverShoulderPosZ:Camera");

	auto state = reinterpret_cast<RE::ThirdPersonState*>(camera->cameraStates[RE::CameraState::kThirdPerson].get());
	if (state)
		state->posOffsetActual = state->posOffsetExpected = {
			x ? x->GetFloat() : 0.0f,
			y ? y->GetFloat() : 0.0f,
			z ? z->GetFloat() : 0.0f
		};
}

bool Camera::Thirdperson::OnPreGameUpdate(RE::PlayerCharacter*, RE::PlayerCamera* camera,
	RE::BSTSmartPointer<RE::TESCameraState>& nextState)
{
	if (Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken()) return false;
	if (camera->currentState->id != RE::CameraState::kPCTransition) return false;

	auto cstate = reinterpret_cast<SkyrimSE::PlayerCameraTransitionState*>(
		camera->cameraStates[RE::CameraState::kPCTransition].get()
	);

	auto horse = camera->cameraStates[RE::CameraState::kMount].get();
	auto tps = camera->cameraStates[RE::CameraState::kThirdPerson].get();
	if (cstate->fromState == horse && cstate->toState == tps) {
		if (!GameState::IsFirstPerson(camera))
		{
			// Getting off a horse, MURDER THIS STATE
			// @TODO: Write our own transition for this state - It still looks nicer with it disabled anyways though
			nextState.reset(cstate->toState);

			auto tpsState = reinterpret_cast<RE::ThirdPersonState*>(
				camera->cameraStates[RE::CameraState::kThirdPerson].get()
			);

			auto horseState = reinterpret_cast<SkyrimSE::HorseCameraState*>(
				camera->cameraStates[RE::CameraState::kMount].get()
			);

			// Just copy a whole wack ton of stuff
			tpsState->posOffsetActual = horseState->posOffsetActual;
			tpsState->translation = horseState->translation;
			tpsState->savedZoomOffset = horseState->savedZoomOffset;
			tpsState->targetZoomOffset = horseState->targetZoomOffset;
			tpsState->currentZoomOffset = horseState->currentZoomOffset;
			tpsState->posOffsetExpected = horseState->posOffsetExpected;
			tpsState->thirdPersonFOVControl->world = tpsState->thirdPersonFOVControl->world;
			stateCopyData.wantMoveToGoal = true;
			return true;
		}
	}

	return false;
}

void Camera::Thirdperson::UpdateInterpolators(const RE::Actor* forRef, const RE::PlayerCamera* camera) noexcept {
	const auto currentOffset = GetCurrentCameraOffset(forRef);
	const auto curTime = GameTime::CurTime();

	// Update ref position
	currentPosition.SetRef(forRef->data.location, forRef->data.angle);

	// Update transition states
	mmath::UpdateTransitionState<glm::vec3, mmath::OffsetTransition>(
		curTime,
		config->enableOffsetInterpolation,
		config->offsetInterpDurationSecs,
		config->offsetScalar,
		offsetTransitionState,
		{ currentOffset.x, currentOffset.y, currentOffset.z }
	);

	if (!povWasPressed && !povTransitionState.running) {
		mmath::UpdateTransitionState<float, mmath::FloatTransition>(
			curTime,
			config->enableZoomInterpolation,
			config->zoomInterpDurationSecs,
			config->zoomScalar,
			zoomTransitionState,
			GetCurrentCameraDistance(camera)
		);
		stateCopyData.lastZoomScalar = GetCurrentCameraZoom(camera);
	} else {
		lastZoomValue = zoomTransitionState.lastPosition = zoomTransitionState.currentPosition =
			zoomTransitionState.targetPosition = GetCurrentCameraDistance(camera);
	}

	mmath::UpdateTransitionState<float, mmath::FloatTransition>(
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
	
	UpdateOffsetSmoothers(forRef, static_cast<float>(curTime));
}

void Camera::Thirdperson::OnUpdateCamera(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
	RE::BSTSmartPointer<RE::TESCameraState>&)
{
#ifdef WITH_CHARTS
	StopWatch prof;
#endif
	const auto wantsControl = Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken();
	const auto wantsUpdates = Messaging::SmoothCamInterface::GetInstance()->WantsInterpolatorUpdates();
	
	// Invalidate while we are in a loading screen state
	// Also if we were in the dialog menu - I'm not able to replicate this particular issue but it doesn't hurt
	// to do it in this case also.
	if (m_camera->InLoadingScreen() || m_camera->wasDialogOpen)
		crosshair->InvalidateEnablementCache();

	// Update our current crosshair selection if required
	if (Render::HasContext() && config->useWorldCrosshair)
		crosshair->Set3DCrosshairType(config->worldCrosshairType);

	currentFocusObject = m_camera->GetCurrentCameraTarget(camera).get();
	if (!currentFocusObject)
		currentFocusObject = player;

	offsetState.currentGroup = GetOffsetForState(m_camera->GetCurrentCameraActionState());

	// Don't continue to update transition states if we aren't running update code
	bool shouldRun = false;
	switch (m_camera->GetCurrentCameraState()) {
		case GameState::CameraState::ThirdPerson: [[fallthrough]];
		case GameState::CameraState::ThirdPersonCombat: [[fallthrough]];
		case GameState::CameraState::ThirdPersonDialogue: [[fallthrough]];
		case GameState::CameraState::Vanity: [[fallthrough]];
		case GameState::CameraState::Horseback:
			shouldRun = true;
			break;
		default: break;
	}

	// Save our last position
	lastPosition = currentPosition;
	
	if (shouldRun) {
		// And set our current reference position
		if (m_camera->wasLoading || (!wantsControl && m_camera->wasCameraAPIControlled &&
		Messaging::SmoothCamInterface::GetInstance()->WantsMoveToGoal()) ||
			stateCopyData.wantMoveToGoal)
		{
			// Exiting a loading screen, moving from horse state or returning from API control
			MoveToGoalPosition(player, currentFocusObject, camera);
			Messaging::SmoothCamInterface::GetInstance()->ClearMoveToGoalFlag();
			stateCopyData.wantMoveToGoal = false;
		}

		UpdateInterpolators(currentFocusObject, camera);

		// Now run the active camera state
		if (!wantsControl || (wantsControl && wantsUpdates)) {
			SetFOVOffset(offsetState.fov);

#ifdef WITH_D2D
			stateOverlay->SetThirdPersonState(runningState);
#endif
			if (runningState) {
				UpdateInternalRotation(camera);
				runningState->Update(player, currentFocusObject, camera);
			}
		}

		// Update POV switch smoothing
		if (!Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken())
			switch (m_camera->GetCurrentCameraState()) {
				case GameState::CameraState::ThirdPerson: [[fallthrough]];
				case GameState::CameraState::ThirdPersonCombat: {
					UpdatePOVSwitchState(camera, RE::CameraState::kThirdPerson);
					break;
				}
				case GameState::CameraState::Horseback: {
					UpdatePOVSwitchState(camera, RE::CameraState::kMount);
					break;
				}
				case GameState::CameraState::Dragon: {
					UpdatePOVSwitchState(camera, RE::CameraState::kDragon);
					break;
				}
				case GameState::CameraState::Bleedout: {
					UpdatePOVSwitchState(camera, RE::CameraState::kBleedout);
					break;
				}
			}
	}

	povWasPressed = false;

	if (!m_camera->InLoadingScreen())
		crosshair->Update(currentFocusObject);

#ifdef WITH_CHARTS
	lastProfSnap = static_cast<float>(prof.Snap());
#endif
}

void Camera::Thirdperson::Render(Render::D3DContext& ctx) noexcept {
	if (m_camera->InLoadingScreen() || m_camera->InMenuMode()) return;
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
			const auto worldPos = GetCurrentCameraTargetWorldPosition(currentFocusObject);
			graph_worldPosTarget->AddPoint(0, worldPos.x);
			graph_worldPosTarget->AddPoint(1, worldPos.y);
			graph_worldPosTarget->AddPoint(2, worldPos.z);

			graph_localSpace->AddPoint(0, lastPosition.local.x);
			graph_localSpace->AddPoint(1, lastPosition.local.y);
			graph_localSpace->AddPoint(2, lastPosition.local.z);

			graph_offsetPos->AddPoint(0, offsetState.position.x);
			graph_offsetPos->AddPoint(1, offsetState.position.y);
			graph_offsetPos->AddPoint(2, offsetState.position.z);

			const auto ofsPos = GetCurrentCameraOffset(RE::PlayerCharacter::GetSingleton());
			graph_targetOffsetPos->AddPoint(0, ofsPos.x);
			graph_targetOffsetPos->AddPoint(1, ofsPos.y);
			graph_targetOffsetPos->AddPoint(2, ofsPos.z);

			graph_rotation->AddPoint(0, rotation.euler.x);
			graph_rotation->AddPoint(1, rotation.euler.y);

			if (GameState::IsThirdPerson(RE::PlayerCamera::GetSingleton())) {
				auto tps = reinterpret_cast<const RE::ThirdPersonState*>(RE::PlayerCamera::GetSingleton()->currentState.get());
				graph_tpsRotation->AddPoint(0, tps->targetYaw);
				graph_tpsRotation->AddPoint(1, tps->currentYaw);
				graph_tpsRotation->AddPoint(2, tps->freeRotation.y);
				graph_tpsRotation->AddPoint(3, RE::PlayerCamera::GetSingleton()->yaw);
			}

			graph_computeTime->AddPoint(0, lastProfSnap);
			graph_computeTime->AddPoint(1, static_cast<float>(GameTime::GetFrameDelta()));
			graph_computeTime->AddPoint(2, static_cast<float>(GameTime::GetSmoothFrameDelta()));

			graph_fov->AddPoint(0, desiredFOV);
			graph_fov->AddPoint(1, *REL::Relocation<float*>(g_Offsets->FOVOffset));

			graph_worldPosTarget->Draw(ctx);
			graph_localSpace->Draw(ctx);
			graph_offsetPos->Draw(ctx);
			graph_targetOffsetPos->Draw(ctx);
			graph_rotation->Draw(ctx);
			graph_tpsRotation->Draw(ctx);
			graph_computeTime->Draw(ctx);
			graph_fov->Draw(ctx);

			break;
		}
		case DisplayMode::NodeTree: {
			focusTargetNodeTree->SetPosition(0, 0);
			focusTargetNodeTree->SetSize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
			if (currentFocusObject->loadedData->data3D)
				focusTargetNodeTree->Draw(ctx, skyrim_cast<RE::NiNode*>(currentFocusObject->loadedData->data3D.get()));
			break;
		}
		case DisplayMode::StateOverlay: {
			stateOverlay->SetPosition(static_cast<uint32_t>((size.x / 2) - 300), static_cast<uint32_t>(size.y - 600));
			stateOverlay->SetSize(600, 400);
			stateOverlay->Draw(currentFocusObject, offsetState.currentGroup, ctx);
			break;
		}
	}
#endif
}

void Camera::Thirdperson::OnTogglePOV(RE::ButtonEvent*) noexcept {
	const auto cam = RE::PlayerCamera::GetSingleton();
	switch (cam->currentState->id) {
		case RE::CameraState::kThirdPerson: [[fallthrough]];
		case RE::CameraState::kAnimated: [[fallthrough]];
		case RE::CameraState::kBleedout: [[fallthrough]];
		case RE::CameraState::kMount: [[fallthrough]];
		case RE::CameraState::kDragon: {
			const auto tps = reinterpret_cast<RE::ThirdPersonState*>(cam->currentState.get());
			stateCopyData.savedZoomValue = tps->targetZoomOffset;
			break;
		}
	}

	povWasPressed = true;
}

bool Camera::Thirdperson::OnKeyPress(const RE::ButtonEvent* ev) noexcept {
	if (m_camera->InMenuMode()) return false;

	auto code = static_cast<int32_t>(ev->idCode);
	if (code <= 0x6 && ev->device == RE::INPUT_DEVICE::kMouse)
		code += 0x100;
	else if (ev->device == RE::INPUT_DEVICE::kGamepad)
		code = Util::GamepadMaskToKeycode(code);

	if (config->shoulderSwapKey >= 0 && config->shoulderSwapKey == code && ev->heldDownSecs <= 0.000001f) {
		shoulderSwap = shoulderSwap == 1 ? -1 : 1;
		return true;
	} else if (config->applyZOffsetKey >= 0 && config->applyZOffsetKey == code && ev->heldDownSecs <= 0.000001f) {
		config->zOffsetActive = !config->zOffsetActive;
		return true;
	} else if (config->toggleUserDefinedOffsetGroupKey >= 0 && config->toggleUserDefinedOffsetGroupKey == code && ev->heldDownSecs <= 0.000001f) {
		config->userDefinedOffsetActive = !config->userDefinedOffsetActive;
		return true;
	}

	return false;
}

bool Camera::Thirdperson::OnMenuOpenClose(MenuID id, const RE::MenuOpenCloseEvent* const) noexcept {
	switch (id) {
		case MenuID::MapMenu: {
			SetFOVOffset(0.0f, true);
			return true;
		}
	}
	return false;
}

void Camera::Thirdperson::OnCameraActionStateTransition(const RE::PlayerCharacter*, const CameraActionState,
	const CameraActionState) noexcept
{}

bool Camera::Thirdperson::OnCameraStateTransition(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
	const GameState::CameraState newState, const GameState::CameraState oldState) noexcept
{
	auto oldCameraState = cameraStates.at(static_cast<size_t>(oldState));
	auto newCameraState = cameraStates.at(static_cast<size_t>(newState));
	if (oldCameraState == newCameraState) return false;

	bool holdSwitch = false;

	switch (oldState) {
		case GameState::CameraState::UsingObject: {
			MoveToGoalPosition(player, currentFocusObject, camera);
			break;
		}
		case GameState::CameraState::FirstPerson: {
			break;
		}
		case GameState::CameraState::ThirdPerson: [[fallthrough]];
		case GameState::CameraState::ThirdPersonDialogue: [[fallthrough]];
		case GameState::CameraState::Vanity: [[fallthrough]];
		case GameState::CameraState::ThirdPersonCombat: {
			holdSwitch = oldCameraState->OnEnd(player, currentFocusObject, camera, newCameraState);
			break;
		}
		case GameState::CameraState::Horseback: {
			holdSwitch = oldCameraState->OnEnd(player, currentFocusObject, camera, newCameraState);

			if (!holdSwitch && newState == GameState::CameraState::Tweening) {
				// The game clears horseYaw to 0 when going from tween back to horseback, which results in a rotation jump
				// once the horse starts moving again. We can store the horse yaw value here to restore later.
				stateCopyData.horseYaw = reinterpret_cast<SkyrimSE::HorseCameraState*>(
					camera->cameraStates[RE::CameraState::kMount].get()
				)->horseYaw;

			} else if (newState == GameState::CameraState::ThirdPerson) {
				MoveToGoalPosition(player, currentFocusObject, camera);
			}

			break;
		}
		case GameState::CameraState::Tweening: [[fallthrough]];
		case GameState::CameraState::Transitioning: {
			// Don't copy positions from these states
			break;
		}
		default:
			break;
	}

	switch (newState) {
		case GameState::CameraState::Vanity: [[fallthrough]];
		case GameState::CameraState::ThirdPerson: [[fallthrough]];
		case GameState::CameraState::ThirdPersonDialogue: [[fallthrough]];
		case GameState::CameraState::ThirdPersonCombat: {
			if (holdSwitch) return holdSwitch;
			runningState = newCameraState;
			newCameraState->OnBegin(player, currentFocusObject, camera, oldCameraState);
			break;
		}
		case GameState::CameraState::Horseback: {
			if (holdSwitch) return holdSwitch;
			runningState = newCameraState;
			newCameraState->OnBegin(player, currentFocusObject, camera, oldCameraState);

			if (oldState == GameState::CameraState::Tweening) {
				// Fix annoying horse rotation reset
				reinterpret_cast<SkyrimSE::HorseCameraState*>(
					camera->cameraStates[RE::CameraState::kMount].get()
				)->horseYaw = stateCopyData.horseYaw;
			}

			break;
		}
		default: {
			// Do this once here on transition to a new state we don't run in
			crosshair->Reset();
			break;
		}
	}

	return false;
}

bool Camera::Thirdperson::IsInputLocked(RE::TESCameraState* state) const noexcept {
	return inputLockers[state->id];
}

void Camera::Thirdperson::LockInputState(uint8_t stateID, bool locked) noexcept {
	inputLockers[stateID] = locked;
}

glm::vec3 Camera::Thirdperson::GetCurrentCameraTargetWorldPosition(const RE::TESObjectREFR* ref) const noexcept
{
	if (ref->loadedData && ref->loadedData->data3D) {
		RE::NiAVObject* node;
		if (m_camera->currentState == GameState::CameraState::Horseback)
			node = ref->loadedData->data3D->GetObjectByName(Strings.spine1);
		else
			node = FindFollowBone(ref);

		if (node)
			return glm::vec3(
				ref->data.location.x,
				ref->data.location.y,
				node->world.translate.z
			);
	}

	return {
		ref->data.location.x,
		ref->data.location.y,
		ref->data.location.z
	};
}

void Camera::Thirdperson::GetCameraGoalPosition(glm::vec3& world, glm::vec3& local,
	const RE::TESObjectREFR* forRef)
{
	const auto cameraLocal = offsetState.position;
	auto translated = rotation.ToRotationMatrix() * glm::vec4(
		cameraLocal.x,
		cameraLocal.y - config->minCameraFollowDistance + zoomTransitionState.currentPosition + GetPitchZoom(),
		0.0f,
		1.0f
	);
	translated.z += cameraLocal.z;

	local = static_cast<glm::vec3>(translated);
	world = 
		GetCurrentCameraTargetWorldPosition(forRef ? forRef : currentFocusObject) +
		local;
}

glm::vec2 Camera::Thirdperson::GetAimRotation(const RE::TESObjectREFR* ref, const RE::PlayerCamera* camera) const {
	if (GameState::IsInHorseCamera(camera)) {
		// In horse camera, we need to clamp our local y axis
		
		// Unless requested via an API consumer
		if (Messaging::SmoothCamInterface::GetInstance()->IsHorseAimUnlocked())
			return rotation.euler;

		const auto yaw = ref->data.angle.z + glm::pi<float>(); // convert to 0-tau
		const auto yawLocal = glm::mod(rotation.euler.y - yaw, glm::pi<float>() * 2.0f); // confine to tau
		const auto clampedLocal = glm::clamp(yawLocal, mmath::half_pi, mmath::half_pi * 3.0f); // clamp it to faceforward zone

		return {
			rotation.euler.x,
			yaw + clampedLocal
		};
	} else {
		const auto tps = reinterpret_cast<const RE::ThirdPersonState*>(camera->cameraStates[RE::CameraState::kThirdPerson].get());
		if ((tps && tps->freeRotationEnabled) || GameState::InPOVSlideMode()) {
			// In free rotation mode aim yaw is locked to player rotation
			// POVSlideMode - The character stays still while the camera orbits

			// @Note: PR #48: Hopefully this still works correctly as both cases are checked,
			// with POVSlideMode checked last
			return {
				rotation.euler.x,
				ref->data.angle.z
			};
		}
	}

	return rotation.euler;
}

const mmath::Rotation& Camera::Thirdperson::GetCameraRotation() const noexcept {
	return rotation;
}

void Camera::Thirdperson::SetCameraRotation(mmath::Rotation& rot, RE::PlayerCamera* camera) noexcept {
	auto mtx = glm::rotate(glm::identity<glm::mat4>(), -mmath::half_pi, { 1.0f, 0.0f, 0.0f });
	mtx = glm::rotate(mtx, -rot.euler.x -mmath::half_pi, { 0.0f, 1.0f, 0.0f });
	mtx = glm::rotate(mtx, rot.euler.y -mmath::half_pi, { 0.0f, 0.0f, 1.0f });

	RE::NiMatrix3 cameraNiT;
	cameraNiT.entry[0][0] = mtx[0][0];
	cameraNiT.entry[0][1] = mtx[0][1];
	cameraNiT.entry[0][2] = mtx[0][2];
	cameraNiT.entry[1][0] = mtx[1][0];
	cameraNiT.entry[1][1] = mtx[1][1];
	cameraNiT.entry[1][2] = mtx[1][2];
	cameraNiT.entry[2][0] = mtx[2][0];
	cameraNiT.entry[2][1] = mtx[2][1];
	cameraNiT.entry[2][2] = mtx[2][2];
	m_camera->cameraNi->world.rotate = cameraNiT;

	RE::NiMatrix3 cameraNodeT;
	cameraNodeT.entry[0][1] = cameraNiT.entry[0][0];
	cameraNodeT.entry[0][2] = cameraNiT.entry[0][1];
	cameraNodeT.entry[0][0] = cameraNiT.entry[0][2];
	cameraNodeT.entry[1][1] = cameraNiT.entry[1][0];
	cameraNodeT.entry[1][2] = cameraNiT.entry[1][1];
	cameraNodeT.entry[1][0] = cameraNiT.entry[1][2];
	cameraNodeT.entry[2][1] = cameraNiT.entry[2][0];
	cameraNodeT.entry[2][2] = cameraNiT.entry[2][1];
	cameraNodeT.entry[2][0] = cameraNiT.entry[2][2];

	// If an API consumer is controlling the camera and has requested updates, we don't want to be setting the rotation
	if (!Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken()) {
		camera->cameraRoot->world.rotate = cameraNodeT;
		camera->cameraRoot->local.rotate = cameraNodeT;
		m_camera->UpdateInternalWorldToScreenMatrix();

		const auto tps = reinterpret_cast<RE::ThirdPersonState*>(camera->currentState.get());
		tps->rotation = rot.ToNiQuat();
	}

	rotation = rot;
}

void Camera::Thirdperson::SetPosition(const glm::vec3& pos, const RE::PlayerCamera* camera) noexcept {
	// If an API consumer is controlling the camera and has requested updates, we don't want to be setting the position
	if (Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken()) return;
	m_camera->SetPosition(pos, camera);
	m_camera->UpdateInternalWorldToScreenMatrix();
}

const mmath::Position& Camera::Thirdperson::GetPosition() const noexcept {
	return lastPosition;
}

Crosshair::Manager* Camera::Thirdperson::GetCrosshairManager() noexcept {
	return crosshair.get();
}

void Camera::Thirdperson::MoveToGoalPosition(const RE::PlayerCharacter* player, const RE::Actor* forRef,
	const RE::PlayerCamera* camera) noexcept
{
	[[maybe_unused]] const auto pov = m_camera->UpdateCameraPOVState(camera);
	const auto actionState = m_camera->UpdateCurrentCameraActionState(player, forRef, camera);
	offsetState.currentGroup = GetOffsetForState(actionState);

	auto ofs = GetCurrentCameraOffset(forRef);
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
	GetCameraGoalPosition(currentPosition.world, currentPosition.local, forRef);
	lastPosition = currentPosition;
}

bool Camera::Thirdperson::IsShoulderSwapped() const noexcept {
	return shoulderSwap != 1;
}

void Camera::Thirdperson::UpdateInternalRotation(const RE::PlayerCamera* camera) noexcept {
	switch (camera->currentState->id) {
		case RE::CameraState::kAnimated: [[fallthrough]];
		case RE::CameraState::kBleedout: [[fallthrough]];
		case RE::CameraState::kMount: [[fallthrough]];
		case RE::CameraState::kDragon: [[fallthrough]];
		case RE::CameraState::kFurniture: [[fallthrough]];
		case RE::CameraState::kThirdPerson: {
			// Alright, just lie and force the game to compute yaw for us
			// We get some jitter when things like magic change our character, not sure why yet @TODO
			auto tps = reinterpret_cast<RE::ThirdPersonState*>(camera->currentState.get());
			if (!tps) return;

			const auto last = tps->freeRotationEnabled;
			tps->freeRotationEnabled = true;
			tps->UpdateRotation();
				rotation.SetQuaternion(tps->rotation);
			tps->freeRotationEnabled = last;
			tps->UpdateRotation();
			break;
		}
		case RE::CameraState::kAutoVanity: {
			auto state = reinterpret_cast<SkyrimSE::AutoVanityState*>(camera->currentState.get());
			rotation.SetEuler(0.0f, state->yaw);
			rotation.UpdateQuaternion();
			break;
		}
		default: {
			logger::warn("Unhandled camera state during rotation update '{}'", std::to_underlying(camera->currentState->id));
			break;
		}
	}
}

RE::NiAVObject* Camera::Thirdperson::FindFollowBone(const RE::TESObjectREFR* ref, const eastl::string_view& filterBone)
	const noexcept
{
	if (!ref->loadedData || !ref->loadedData->data3D) return nullptr;
	auto& boneList = Config::GetBonePriorities();

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

float Camera::Thirdperson::GetCurrentCameraDistance(const RE::PlayerCamera* camera) const noexcept {
	return -(GetCurrentCameraZoom(camera) * config->zoomMul);
}

float Camera::Thirdperson::GetPitchZoom() const noexcept {
	if (!config->enablePitchZoom || rotation.euler.x < 0.0f) return 0.0f;
	const auto scalar = glm::clamp(rotation.euler.x / glm::radians(config->pitchZoomMaxAngle), 0.0f, 1.0f);
	return mmath::Interpolate(
		0.0f, config->pitchZoomMax, mmath::RunScalarFunction(config->pitchZoomMethod, scalar)
	);
}

float Camera::Thirdperson::GetCurrentCameraZoom(const RE::PlayerCamera* camera) const noexcept {
	switch (camera->currentState->id) {
		case RE::CameraState::kAutoVanity: {
			return stateCopyData.lastZoomScalar;
		}
		case RE::CameraState::kAnimated: [[fallthrough]];
		case RE::CameraState::kThirdPerson: [[fallthrough]];
		case RE::CameraState::kMount: [[fallthrough]];
		case RE::CameraState::kDragon: [[fallthrough]];
		case RE::CameraState::kBleedout: {
			return GetCameraZoomScalar(camera, camera->currentState->id);
		} default:
			return 0.0f;
	}
}

float Camera::Thirdperson::GetCameraZoomScalar(const RE::PlayerCamera* camera, uint32_t cameraState) const noexcept {
	const auto state = reinterpret_cast<const RE::ThirdPersonState*>(camera->cameraStates[cameraState].get());
	if (!state) return 0.0f;

	auto minZoom = 0.2f;
	static auto minZoomConf = RE::INISettingCollection::GetSingleton()->GetSetting("fMinCurrentZoom:Camera");
	if (minZoomConf) minZoom = minZoomConf->GetFloat();

	if (povWasPressed) // So we don't get jumping at the start of a pov switch
		return stateCopyData.savedZoomValue + (minZoom * -1);

	return state->targetZoomOffset + (minZoom * -1);
}

bool Camera::Thirdperson::IsInterpAllowed(const RE::Actor* player) const noexcept {
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

bool Camera::Thirdperson::IsInterpOverloaded(const RE::Actor* player) const noexcept {
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

bool Camera::Thirdperson::IsLocalInterpOverloaded(const RE::Actor* player) const noexcept {
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

const Config::OffsetGroupScalar* Camera::Thirdperson::GetCurrentScalarGroup(const RE::Actor* player,
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
	if (state == CameraActionState::Vanity) return &config->vanity;

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

float Camera::Thirdperson::GetActiveWeaponStateForwardOffset(const RE::Actor* player,
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

float Camera::Thirdperson::GetActiveWeaponStateUpOffset(const RE::Actor* player,
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

float Camera::Thirdperson::GetActiveWeaponStateSideOffset(const RE::Actor* player,
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

float Camera::Thirdperson::GetActiveWeaponStateFOVOffset(const RE::Actor* player,
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

float Camera::Thirdperson::GetCurrentCameraForwardOffset(const RE::Actor* player) const noexcept {
	if (m_camera->currentState == GameState::CameraState::Vanity)
		return offsetState.currentGroup->zoomOffset;

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
		case CameraActionState::DisMounting: [[fallthrough]];
		case CameraActionState::Sleeping: [[fallthrough]];
		case CameraActionState::Sitting: [[fallthrough]];
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->zoomOffset;
		}
		case CameraActionState::Aiming: {
			if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive)
				return config->bowAim.combatMeleeZoomOffset;
			return offsetState.currentGroup->zoomOffset;
		}
		case CameraActionState::Sneaking: [[fallthrough]];
		case CameraActionState::VampireLord: [[fallthrough]];
		case CameraActionState::Werewolf: [[fallthrough]];
		case CameraActionState::Sprinting: [[fallthrough]];
		case CameraActionState::Walking: [[fallthrough]];
		case CameraActionState::Running: [[fallthrough]];
		case CameraActionState::Standing: {
			return GetActiveWeaponStateForwardOffset(player, offsetState.currentGroup);
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

float Camera::Thirdperson::GetCurrentCameraHeight(const RE::Actor* player) const noexcept {
	if (m_camera->currentState == GameState::CameraState::Vanity)
		return offsetState.currentGroup->upOffset;

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
		case CameraActionState::DisMounting: [[fallthrough]];
		case CameraActionState::Sleeping: [[fallthrough]];
		case CameraActionState::Sitting: [[fallthrough]];
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->upOffset;
		}
		case CameraActionState::Aiming: {
			if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive)
				return config->bowAim.combatMeleeUpOffset;
			return offsetState.currentGroup->upOffset;
		}
		case CameraActionState::Sneaking: [[fallthrough]];
		case CameraActionState::VampireLord: [[fallthrough]];
		case CameraActionState::Werewolf: [[fallthrough]];
		case CameraActionState::Sprinting: [[fallthrough]];
		case CameraActionState::Walking: [[fallthrough]];
		case CameraActionState::Running: [[fallthrough]];
		case CameraActionState::Standing: {
			return GetActiveWeaponStateUpOffset(player, offsetState.currentGroup);
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

float Camera::Thirdperson::GetCurrentCameraSideOffset(const RE::Actor* player) const noexcept
{
	if (m_camera->currentState == GameState::CameraState::Vanity)
		return offsetState.currentGroup->sideOffset * shoulderSwap;

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
		case CameraActionState::DisMounting: [[fallthrough]];
		case CameraActionState::Sleeping: [[fallthrough]];
		case CameraActionState::Sitting: [[fallthrough]];
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->sideOffset * shoulderSwap;
		}
		case CameraActionState::Aiming: {
			if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive)
				return config->bowAim.combatMeleeSideOffset * shoulderSwap;
			return offsetState.currentGroup->sideOffset * shoulderSwap;
		}
		case CameraActionState::Sneaking: [[fallthrough]];
		case CameraActionState::VampireLord: [[fallthrough]];
		case CameraActionState::Werewolf: [[fallthrough]];
		case CameraActionState::Sprinting: [[fallthrough]];
		case CameraActionState::Walking: [[fallthrough]];
		case CameraActionState::Running: [[fallthrough]];
		case CameraActionState::Standing: {
			return GetActiveWeaponStateSideOffset(player, offsetState.currentGroup) * shoulderSwap;
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

float Camera::Thirdperson::GetCurrentCameraFOVOffset(const RE::Actor* player) const noexcept {
	if (m_camera->currentState == GameState::CameraState::Vanity)
		return offsetState.currentGroup->fovOffset;

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
		case CameraActionState::DisMounting: [[fallthrough]];
		case CameraActionState::Sleeping: [[fallthrough]];
		case CameraActionState::Sitting: [[fallthrough]];
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->fovOffset;
		}
		case CameraActionState::Aiming:{
			if (GameState::IsSneaking(player) && !config->userDefinedOffsetActive)
				return config->bowAim.combatMeleeFOVOffset;
			return offsetState.currentGroup->fovOffset;
		}
		case CameraActionState::Sneaking: [[fallthrough]];
		case CameraActionState::VampireLord: [[fallthrough]];
		case CameraActionState::Werewolf: [[fallthrough]];
		case CameraActionState::Sprinting: [[fallthrough]];
		case CameraActionState::Walking: [[fallthrough]];
		case CameraActionState::Running: [[fallthrough]];
		case CameraActionState::Standing: {
			return GetActiveWeaponStateFOVOffset(player, offsetState.currentGroup);
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

glm::vec4 Camera::Thirdperson::GetCurrentCameraOffset(const RE::Actor* player) const noexcept
{
	return {
		GetCurrentCameraSideOffset(player),
		GetCurrentCameraForwardOffset(player),
		GetCurrentCameraHeight(player) + (config->zOffsetActive ? config->customZOffset : 0.0f),
		GetCurrentCameraFOVOffset(player)
	};
}

// @TODO: Pretty much all code paths here except SepZ are dead code now!
double Camera::Thirdperson::GetCurrentSmoothingScalar(const RE::Actor* player, const float distance,
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
		const auto delta = glm::max(GameTime::GetSmoothFrameDelta(), minZero);
		const auto lambda = 1.0 - glm::pow(1.0 - remapped, delta * 60.0);
		return glm::clamp(lambda, 0.0, 1.0);
	} else {
		interpValue = remapped;
	}

	return mmath::RunScalarFunction<double>(scalarMethod, interpValue);
}

std::tuple<glm::vec3, glm::vec3> Camera::Thirdperson::GetDistanceClamping() const noexcept {
	float minsX = config->cameraDistanceClampXMin;
	float maxsX = config->cameraDistanceClampXMax;

	if (config->swapXClamping && shoulderSwap < 1) {
		eastl::swap(minsX, maxsX);
		maxsX *= -1.0f;
		minsX *= -1.0f;
	}

	return {
		glm::vec3{ minsX, config->cameraDistanceClampYMin, config->cameraDistanceClampZMin },
		glm::vec3{ maxsX, config->cameraDistanceClampYMax, config->cameraDistanceClampZMax }
	};
}

void Camera::Thirdperson::SetFOVOffset(float fov, bool force) noexcept {
	auto adr = REL::Relocation<float*>(g_Offsets->FOVOffset);
	const auto cam = RE::PlayerCamera::GetSingleton();
	
#ifdef WITH_CHARTS
	desiredFOV = fov;
#endif

	if (force) {
		// When in the map menu, this won't be reset, thus we have to force it.
		// Opening the map menu when zoomed in with a bow does the same thing.
		*adr = fov;

		// This is caused by the reset code being the job of thirdperson cameras (Third, Horse, Dragon)
		// The map menu is a different camera

	} else {
		// FOV reset each frame relies on the controller node being valid
		// If this isn't the case, we should do it manually here
		// Controller node becomes null in certain cases - Undeath lich form, transformation spells, so on
		auto curState = cam->currentState.get();
		RE::NiNode* node = nullptr;
		if (curState == cam->cameraStates[RE::CameraState::kThirdPerson].get()) {
			node = reinterpret_cast<RE::ThirdPersonState*>(curState)->thirdPersonFOVControl;

		} else if (curState == cam->cameraStates[RE::CameraState::kMount].get()) {
			node = reinterpret_cast<SkyrimSE::HorseCameraState*>(curState)->thirdPersonFOVControl;

		} else if (curState == cam->cameraStates[RE::CameraState::kDragon].get()) {
			node = reinterpret_cast<SkyrimSE::DragonCameraState*>(curState)->thirdPersonFOVControl;
		}

		if (!node)
			*adr = 0.0f;
		else {
			if (node->local.translate.z != 0.0f) {
				// @Note: controllerNode->localTransform.pos.z is the fov offset set by base game logic.
				// Normally not an issue except for sprinting with weapons apparently.
				// Check for sprinting so we don't nuke bow zooming or anything like that.
				if (currentFocusObject && GameState::IsSprinting(currentFocusObject)
					&& GameState::IsWeaponDrawn(currentFocusObject))
					*adr = 0.0f;
			}
		}
	}

	if (fov == 0.0f) return;

	// This is an offset value applied to any call to SetFOV.
	// Used internally in the game by things like zooming with the bow.
	// Appears to be set by the game every frame so we can just add our offset on top like this.
	const auto baseValue = cam->worldFOV;
	const auto currentOffset = *adr;

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

	*adr += finalOffset;
}

mmath::OffsetTransition& Camera::Thirdperson::GetOffsetTransitionState() noexcept {
	return offsetTransitionState;
}

mmath::FloatTransition& Camera::Thirdperson::GetFOVTransitionState() noexcept {
	return fovTransitionState;
}

// Return the current Zoom transition state, for mutation
mmath::FloatTransition& Camera::Thirdperson::GetZoomTransitionState() noexcept {
	return zoomTransitionState;
}

bool Camera::Thirdperson::UpdatePOVSwitchState(RE::PlayerCamera* camera, uint16_t cameraState) noexcept {
	auto state = reinterpret_cast<RE::ThirdPersonState*>(camera->cameraStates[cameraState].get());
	if (state->stateNotActive) {
		auto minZoom = 0.2f;
		static auto minZoomConf = RE::INISettingCollection::GetSingleton()->GetSetting("fMinCurrentZoom:Camera");
		if (minZoomConf) minZoom = minZoomConf->GetFloat();
		const auto curTime = GameTime::CurTime();
		constexpr auto duration = 0.5f;

		// We are switching over to first person
		if (povWasPressed) {
			// We just started the switch
			povTransitionState.running = true;
			povTransitionState.startTime = curTime;
			stateCopyData.savedZoomValue = povTransitionState.lastValue =
				stateCopyData.savedZoomValue; // Set by our toggle pov callback
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

		state->targetZoomOffset = state->currentZoomOffset = povTransitionState.lastValue;
		
		// Switch mid-zoom
		if (scalar >= 0.5f) {
			povTransitionState.running = false;
			typedef void(*SwitchToFirstPerson)(RE::PlayerCamera*);
			REL::Relocation<SwitchToFirstPerson>(g_Offsets->SwitchToFPV)(camera);
			return true;
		}

	} else {
		// Record the current camera zoom value for use during switches
		lastZoomValue = state->targetZoomOffset;
		povTransitionState.running = false;
	}

	return false;
}

void Camera::Thirdperson::UpdateOffsetSmoothers(const RE::Actor* player, float curTime) noexcept {
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