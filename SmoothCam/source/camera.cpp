#include "camera.h"
#include "arrow_fixes.h"
#ifdef _DEBUG
#include "debug_drawing.h"
#endif

double GetFrameDelta() noexcept;
double GetTime() noexcept;

Camera::SmoothCamera::SmoothCamera() noexcept : config(Config::GetCurrentConfig()) {
	cameraStates[static_cast<size_t>(GameState::CameraState::ThirdPerson)] =
		std::move(std::make_unique<State::ThirdpersonState>(this));
	cameraStates[static_cast<size_t>(GameState::CameraState::ThirdPersonCombat)] =
		std::move(std::make_unique<State::ThirdpersonCombatState>(this));
	cameraStates[static_cast<size_t>(GameState::CameraState::Horseback)] =
		std::move(std::make_unique<State::ThirdpersonHorseState>(this));
}

// Called when the player toggles the POV
void Camera::SmoothCamera::OnTogglePOV(const ButtonEvent* ev) noexcept {
	povIsThird = !povIsThird;
	povWasPressed = true;
}

void Camera::SmoothCamera::OnDialogMenuChanged(const MenuOpenCloseEvent* const ev) noexcept {
	dialogMenuOpen = ev->opening;
}

glm::vec3 Camera::SmoothCamera::GetCurrentPosition() const noexcept {
	return currentPosition;
}

// Updates our POV state to the true value the game expects for each state
const bool Camera::SmoothCamera::UpdateCameraPOVState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) noexcept {
	const auto zoom = reinterpret_cast<const CorrectedThirdPersonState*>(camera)->cameraZoom;
	const auto lzoom = reinterpret_cast<const CorrectedThirdPersonState*>(camera)->cameraLastZoom;
	povIsThird = zoom == 0.0f || GameState::IsInAutoVanityCamera(camera) || GameState::IsInTweenCamera(camera) ||
		GameState::IsInCameraTransition(camera) || GameState::IsInUsingObjectCamera(camera) || GameState::IsInKillMove(camera) ||
		GameState::IsInBleedoutCamera(camera) || GameState::IsInFurnitureCamera(camera) || GameState::IsInHorseCamera(camera) ||
		GameState::IsInDragonCamera(camera) || GameState::IsThirdPerson(camera);
	return povIsThird;
}

#pragma region Camera state updates
// Returns the current camera state for use in selecting an update method
const GameState::CameraState Camera::SmoothCamera::GetCurrentCameraState(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) {
	GameState::CameraState newState = GameState::CameraState::Unknown;
	if (!povWasPressed && !GameState::IsInHorseCamera(camera) && !GameState::IsInDragonCamera(camera) && GameState::IsSitting(player) 
		&& !GameState::IsSleeping(player) && config->compatIC_FirstPersonSitting)
	{
		const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraState);
		if (tps && tps->cameraZoom < -1.0f && tps->cameraLastZoom < -1.0f) {
			newState = GameState::CameraState::FirstPerson;
			goto applyStateTransition; // SUE ME
		}
	}

	newState = GameState::GetCameraState(player, camera);
	if (newState == GameState::CameraState::Horseback && config->comaptIC_FirstPersonHorse) {
		const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraState);
		if (tps) {
			if ((tps->cameraZoom == -SKYRIM_MIN_ZOOM_FRACTION && tps->cameraLastZoom == -SKYRIM_MIN_ZOOM_FRACTION) ||
				currentActionState == CameraActionState::FirstPersonHorseback)
			{
				if (povWasPressed)
					newState = GameState::CameraState::Horseback;
				else
					newState = GameState::CameraState::FirstPerson;
			} else {
				newState = GameState::CameraState::Horseback;
			}
		} else {
			newState = GameState::CameraState::Horseback;
		}
	} else if (newState == GameState::CameraState::Dragon && config->comaptIC_FirstPersonDragon) {
		const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraState);
		if (tps) {
			if ((tps->cameraZoom == -SKYRIM_MIN_ZOOM_FRACTION && tps->cameraLastZoom == -SKYRIM_MIN_ZOOM_FRACTION) ||
				currentActionState == CameraActionState::FirstPersonDragon)
			{
				newState = GameState::CameraState::FirstPerson;
			} else {
				newState = GameState::CameraState::Dragon;
			}
		} else {
			newState = GameState::CameraState::Dragon;
		}
	}

applyStateTransition:
	if (newState != currentState) {
		lastState = currentState;
		currentState = newState;
		OnCameraStateTransition(player, camera, newState, lastState);
	}
	return newState;
}

// Returns the current camera action state for use in the selected update method
const Camera::CameraActionState Camera::SmoothCamera::GetCurrentCameraActionState(const PlayerCharacter* player,
	const CorrectedPlayerCamera* camera) noexcept
{
	CameraActionState newState = CameraActionState::Unknown;

	if (GameState::IsInHorseCamera(camera)) {
		// Improved camera compat
		if (!povIsThird) {
			newState = CameraActionState::FirstPersonHorseback;
		} else if (GameState::IsDisMountingHorse(player)) {
			newState = CameraActionState::DisMounting;
		}
	} else if (GameState::IsInDragonCamera(camera)) {
		// Improved camera compat
		if (currentState == GameState::CameraState::FirstPerson) {
			newState = CameraActionState::FirstPersonDragon;
		}
	} else if (GameState::IsSleeping(player)) {
		newState = CameraActionState::Sleeping;
	} else if (GameState::IsInFurnitureCamera(camera)) {
		newState = CameraActionState::SittingTransition;
	} else if (GameState::IsSitting(player)) {
		// Improved camera compat
		if (currentState == GameState::CameraState::FirstPerson) {
			newState = CameraActionState::FirstPersonSitting;
		} else {
			newState = CameraActionState::Sitting;
		}
	} else if (GameState::IsSneaking(player)) {
		newState = CameraActionState::Sneaking;
	} else if (GameState::IsBowDrawn(player)) {
		newState = CameraActionState::Aiming;
	} else if (GameState::IsSwimming(player)) {
		newState = CameraActionState::Swimming;
	} else if (GameState::IsSprinting(player)) {
		newState = CameraActionState::Sprinting;
	} else if (GameState::IsWalking(player)) {
		newState = CameraActionState::Walking;
	} else if (GameState::IsRunning(player)) {
		newState = CameraActionState::Running;
	} else {
		newState = CameraActionState::Standing;
	}

	if (newState != currentActionState) {
		lastActionState = currentActionState;
		currentActionState = newState;
#ifdef _DEBUG
		OnCameraActionStateTransition(player, newState, lastActionState);
#endif
	}

	return newState;
}

#ifdef _DEBUG
// Triggers when the camera action state changes, for debugging
void Camera::SmoothCamera::OnCameraActionStateTransition(const PlayerCharacter* player,
	const CameraActionState newState, const CameraActionState oldState) const noexcept
{
	// For debugging
}
#endif

// Triggers when the camera state changes
void Camera::SmoothCamera::OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const GameState::CameraState newState, const GameState::CameraState oldState) const
{
	switch (oldState) {
		case GameState::CameraState::ThirdPerson: {
			if (cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson))) {
				dynamic_cast<State::ThirdpersonState*>(
					cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson)).get()
				)->OnEnd(player, camera);
				break;
			}
		}
		case GameState::CameraState::ThirdPersonCombat: {
			if (cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat))) {
				dynamic_cast<State::ThirdpersonCombatState*>(
					cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat)).get()
				)->OnEnd(player, camera);
				break;
			}
		}
		case GameState::CameraState::Horseback: {
			if (cameraStates.at(static_cast<size_t>(GameState::CameraState::Horseback))) {
				dynamic_cast<State::ThirdpersonHorseState*>(
					cameraStates.at(static_cast<size_t>(GameState::CameraState::Horseback)).get()
				)->OnEnd(player, camera);
				break;
			}
		}
		default:
			break;
	}

	switch (newState) {
		case GameState::CameraState::ThirdPerson: {
			if (cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson))) {
				dynamic_cast<State::ThirdpersonState*>(
					cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson)).get()
				)->OnBegin(player, camera);
				break;
			}
		}
		case GameState::CameraState::ThirdPersonCombat: {
			if (cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat))) {
				dynamic_cast<State::ThirdpersonCombatState*>(
					cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat)).get()
				)->OnBegin(player, camera);
				break;
			}
		}
		case GameState::CameraState::Horseback: {
			if (cameraStates.at(static_cast<size_t>(GameState::CameraState::Horseback))) {
				dynamic_cast<State::ThirdpersonHorseState*>(
					cameraStates.at(static_cast<size_t>(GameState::CameraState::Horseback)).get()
				)->OnBegin(player, camera);
				break;
			}
		}
		default:
			break;
	}
}
#pragma endregion

#pragma region Camera position calculations
// Returns the zoom value set from the given camera state
float Camera::SmoothCamera::GetCurrentCameraZoom(const CorrectedPlayerCamera* camera, const GameState::CameraState currentState) const noexcept {
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

// Returns an offset group for the current player movement state
const Config::OffsetGroup* Camera::SmoothCamera::GetOffsetForState(const CameraActionState state) const noexcept {
	switch (state) {
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

// Selects the right offset from an offset group for the player's weapon state
float Camera::SmoothCamera::GetActiveWeaponStateUpOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept {
	if (!GameState::IsWeaponDrawn(player)) return group->upOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedUpOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicUpOffset;
	}
	return group->combatMeleeUpOffset;
}

// Selects the right offset from an offset group for the player's weapon state
float Camera::SmoothCamera::GetActiveWeaponStateSideOffset(PlayerCharacter* player, const Config::OffsetGroup* group) const noexcept {
	if (!GameState::IsWeaponDrawn(player)) return group->sideOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group->combatRangedSideOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group->combatMagicSideOffset;
	}
	return group->combatMeleeSideOffset;
}

// Returns the camera height for the current player state
float Camera::SmoothCamera::GetCurrentCameraHeight(PlayerCharacter* player) const noexcept {
	switch (currentState) {
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

	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Aiming:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->upOffset;
		}
		case CameraActionState::Sneaking:
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

// Returns the camera side offset for the current player state
float Camera::SmoothCamera::GetCurrentCameraSideOffset(PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept {
	switch (currentState) {
		case GameState::CameraState::Horseback: {
			if (GameState::IsBowDrawn(player)) {
				return config->bowAim.horseSideOffset;
			} else {
				return GetActiveWeaponStateSideOffset(player, &config->horseback);
			}
		}
		default:
			break;
	}

	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Aiming:
		case CameraActionState::Swimming: {
			return offsetState.currentGroup->sideOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateSideOffset(player, offsetState.currentGroup);
		}
		default: {
			break;
		}
	}
	return 0.0f;
}

// Returns the ideal camera distance for the current zoom level
float Camera::SmoothCamera::GetCurrentCameraDistance(const CorrectedPlayerCamera* camera) const noexcept {
	return -(config->minCameraFollowDistance + (GetCurrentCameraZoom(camera, currentState) * config->zoomMul));
}

// Returns the full local-space camera offset for the current player state
glm::vec3 Camera::SmoothCamera::GetCurrentCameraOffset(PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept {
	return {
		GetCurrentCameraSideOffset(player, camera),
		GetCurrentCameraDistance(camera),
		GetCurrentCameraHeight(player)
	};
}

// Returns the full world-space camera target postion for the current player state
glm::vec3 Camera::SmoothCamera::GetCurrentCameraTargetWorldPosition(const PlayerCharacter* player,
	const CorrectedPlayerCamera* camera) const
{
	if (player->loadedState && player->loadedState->node) {
		NiAVObject* node;
		if (currentState == GameState::CameraState::Horseback) {
			BSFixedString nodeName = "NPC Spine1 [Spn1]";
			node = player->loadedState->node->GetObjectByName(&nodeName.data);
		} else {
			BSFixedString nodeName = "Camera3rd [Cam3]";
			node = player->loadedState->node->GetObjectByName(&nodeName.data);
		}

		if (node) {
			return glm::vec3(
				player->pos.x,
				player->pos.y,
				node->m_worldTransform.pos.z
			);
		}
	}

	return {
		player->pos.x,
		player->pos.y,
		player->pos.z
	};
}

void Camera::SmoothCamera::SetPosition(const glm::vec3& pos, const CorrectedPlayerCamera* camera) noexcept {
	auto cameraNode = camera->cameraNode;
	auto cameraNi = reinterpret_cast<NiCamera*>(
		cameraNode->m_children.m_size == 0 ?
		nullptr :
		cameraNode->m_children.m_data[0]
	);
	if (!cameraNi) return;

	currentPosition = pos;

#ifdef _DEBUG
	if (!mmath::IsValid(currentPosition)) {
		__debugbreak();
		// Oops, go ahead and clear both
		lastPosition = currentPosition = {
			cameraNode->m_worldTransform.pos.x,
			cameraNode->m_worldTransform.pos.y,
			cameraNode->m_worldTransform.pos.z
		};
		return;
	}
#endif

	const NiPoint3 niPos = { currentPosition.x, currentPosition.y, currentPosition.z };
	cameraNode->m_localTransform.pos = niPos;
	cameraNode->m_worldTransform.pos = niPos;
	cameraNi->m_worldTransform.pos = niPos;

	if (currentState == GameState::CameraState::ThirdPerson || currentState == GameState::CameraState::ThirdPersonCombat) {
		auto state = reinterpret_cast<CorrectedThirdPersonState*>(camera->cameraState);
		state->translation = niPos;
	}

	// Update world to screen matrices
	UpdateInternalWorldToScreenMatrix(cameraNi, GetCameraPitchRotation(camera), GetCameraYawRotation(camera));
	Offsets::Get<UpdateWorldToScreenMtx>(69271)(cameraNi);
}

void Camera::SmoothCamera::UpdateInternalWorldToScreenMatrix(NiCamera* camera, float pitch, float yaw) noexcept {
	auto lastRotation = camera->m_worldTransform.rot;
	camera->m_worldTransform.rot = mmath::ToddHowardTransform(pitch, yaw);
	// Force the game to compute the matrix for us
	Offsets::Get<UpdateWorldToScreenMtx>(69271)(camera);
	// Grab it
	worldToScreen = *reinterpret_cast<mmath::NiMatrix44*>(camera->m_aafWorldToCam);
	// Now restore the normal camera rotation
	camera->m_worldTransform.rot = lastRotation;
}

// Returns the current smoothing scalar to use for the given distance to the player
float Camera::SmoothCamera::GetCurrentSmoothingScalar(const float distance, ScalarSelector method) const {
	Config::ScalarMethods scalarMethod;

	float scalar = 1.0f;
	float interpValue = 1.0f;
	float remapped = 1.0f;

	if (method == ScalarSelector::SepZ) {
		const auto max = config->separateZMaxSmoothingDistance;
		scalar = glm::clamp(1.0f - ((max - distance) / max), 0.0f, 1.0f);
		remapped = mmath::Remap<float>(scalar, 0.0f, 1.0f, config->separateZMinFollowRate, config->separateZMaxFollowRate);
		scalarMethod = config->separateZScalar;
	} else if (method == ScalarSelector::LocalSpace) {
		remapped = distance;
		scalarMethod = config->separateLocalScalar;
	} else {
		const auto max = config->zoomMaxSmoothingDistance;
		scalar = glm::clamp(1.0f - ((max - distance) / max), 0.0f, 1.0f);
		remapped = mmath::Remap<float>(scalar, 0.0f, 1.0f, config->minCameraFollowRate, config->maxCameraFollowRate);
		scalarMethod = config->currentScalar;
	}

	if (!config->disableDeltaTime) {
		const auto delta = GetFrameDelta();
		const auto fps = 1.0 / delta;
		const auto mul = -fps * glm::log2(1.0f - remapped);
		interpValue = static_cast<float>(glm::clamp(1.0 - glm::exp2(-mul * delta), 0.0, 1.0));
	} else {
		interpValue = remapped;
	}

	return mmath::RunScalarFunction<float>(scalarMethod, interpValue);
}

// Returns true if interpolation is allowed in the current state
bool Camera::SmoothCamera::IsInterpAllowed(PlayerCharacter* player) const noexcept {
	auto ofs = offsetState.currentGroup;
	if (currentState == GameState::CameraState::Horseback) {
		if (GameState::IsWeaponDrawn(player) && GameState::IsBowDrawn(player)) {
			return config->bowAim.interpHorseback;
		} else {
			ofs = &config->horseback;
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

// Constructs the view matrix for the camera
glm::mat4 Camera::SmoothCamera::GetViewMatrix(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept {
	// Build a rotation matrix to rotate the camera around pitch and yaw axes.
	auto m = glm::identity<glm::mat4>();
	m = glm::rotate(m, -GetCameraYawRotation(camera), Camera::UNIT_UP); // yaw
	m = glm::rotate(m, -GetCameraPitchRotation(camera), Camera::UNIT_FORWARD); // pitch
	return m;
}
#pragma endregion

#pragma region Crosshair stuff
// Updates the screen position of the crosshair for correct aiming
void Camera::SmoothCamera::UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* camera) {
	NiPoint3 niOrigin = { 0.01f, 0.01f, 0.01f };
	NiPoint3 niNormal = { 0.0f, 1.00f, 0.0f };
	const auto bowDrawn = GameState::IsBowDrawn(player);
	BSFixedString handNodeName = "WEAPON";

	if (currentState != GameState::CameraState::Horseback) {
		// @TODO: Add CommonLibSSE during next major refactor
		typedef void(__thiscall PlayerCharacter::* GetEyeVector)(NiPoint3& origin, NiPoint3& normal, bool factorCameraOffset);
		(player->*reinterpret_cast<GetEyeVector>(&PlayerCharacter::Unk_C2))(niOrigin, niNormal, false);
	} else {
		// EyeVector is busted on horseback
		BSFixedString nodeName = "Throat 2"; // Gets me the closest to niOrigin
		const auto node = player->loadedState->node->GetObjectByName(&nodeName.data);
		if (node) {
			niOrigin = NiPoint3(player->pos.x, player->pos.y, node->m_worldTransform.pos.z);
		}

		const auto n = mmath::GetViewVector(
			glm::vec3(0.0, 1.0, 0.0),
			GetCameraPitchRotation(camera),
			GetCameraYawRotation(camera)
		);
		niNormal = NiPoint3(n.x, n.y, n.z);
	}

	if (bowDrawn) {
		const auto handNode = static_cast<NiNode*>(player->loadedState->node->GetObjectByName(&handNodeName.data));
		if (handNode && handNode->m_children.m_size > 0) {
			const auto arrow = static_cast<NiNode*>(handNode->m_children.m_data[0]);
			niOrigin = arrow->m_worldTransform.pos;
			// @Note: I'm sure there is some way to make this perfect, but this is close enough
			constexpr auto fac = ArrowFixes::arrowPitchModFactor * 0.5f;
			const auto n = mmath::GetViewVector(
				glm::vec3(0.0, 1.0, 0.0),
				GetCameraPitchRotation(camera) - fac,
				GetCameraYawRotation(camera)
			);
			niNormal = NiPoint3(n.x, n.y, n.z);
		}
	} else if (GameState::IsMagicDrawn(player)) {
		BSFixedString nodeName = "MagicEffectsNode";
		const auto node = player->loadedState->node->GetObjectByName(&nodeName.data);
		if (node) {
			niOrigin = NiPoint3(player->pos.x, player->pos.y, node->m_worldTransform.pos.z);
		}

		const auto n = mmath::GetViewVector(
			glm::vec3(0.0, 1.0, 0.0),
			GetCameraPitchRotation(camera),
			GetCameraYawRotation(camera)
		);
		niNormal = NiPoint3(n.x, n.y, n.z);
	}

	// Cast the aim ray
	constexpr auto rayLength = 6000.0f; // Range of most (all?) arrows
	auto origin = glm::vec4(niOrigin.x, niOrigin.y, niOrigin.z, 0.0f);
	auto ray = glm::vec4(niNormal.x, niNormal.y, niNormal.z, 0.0f) * rayLength;
	const auto result = Raycast::CastRay(origin, origin + ray, 0.1f, true);

	auto port = NiRect<float>();
	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		auto rect = menu->view->GetVisibleFrameRect();
		port.m_left = rect.left;
		port.m_right = rect.right;
		port.m_top = rect.bottom;
		port.m_bottom = rect.top;
	}

	glm::vec2 crosshairSize(baseCrosshairData.xScale, baseCrosshairData.yScale);
	glm::vec2 crosshairPos(port.m_right * 0.5f, port.m_top * 0.5f);
	if (result.hit) {
		auto pt = NiPoint3(
			result.hitPos.x,
			result.hitPos.y,
			result.hitPos.z
		);

		auto rangeScalar = glm::clamp((rayLength - result.rayLength) / rayLength, 0.0f, 1.0f);
		auto sz = mmath::Remap(rangeScalar, 0.0f, 1.0f, config->crosshairMinDistSize, config->crosshairMaxDistSize);
		crosshairSize = { sz, sz };

		if (result.hitCharacter)
			crosshairSize += config->crosshairNPCHitGrowSize * rangeScalar;

		glm::vec3 screen = {};
		(*WorldPtToScreenPt3_Internal)(
			reinterpret_cast<float*>(worldToScreen.data),
			&port, &pt,
			&screen.x, &screen.y, &screen.z, 9.99999975e-06
		);

		crosshairPos = {
			screen.x,
			screen.y
		};
	}

#ifdef _DEBUG
	auto lineStart = mmath::PointToScreen(origin);
	auto lineEnd = mmath::PointToScreen(result.hit ? result.hitPos : origin + ray);
	DebugDrawing::Submit(DebugDrawing::DrawLine(lineStart, lineEnd, { 0.0f, 1.0f, 0.0f }));
#endif

	SetCrosshairPosition(crosshairPos);
	SetCrosshairSize(crosshairSize);
}

void Camera::SmoothCamera::ReadInitialCrosshairInfo() {
	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (!menu || !menu->view) return;
	
	GFxValue va;
	menu->view->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._x");
	baseCrosshairData.xOff = va.GetNumber();

	menu->view->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._y");
	baseCrosshairData.yOff = va.GetNumber();

	menu->view->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._width");
	baseCrosshairData.xScale = va.GetNumber();

	menu->view->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._height");
	baseCrosshairData.yScale = va.GetNumber();

	baseCrosshairData.captured = true;
}

void Camera::SmoothCamera::SetCrosshairPosition(const glm::vec2& pos) const {
	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		auto rect = menu->view->GetVisibleFrameRect();
		auto half_x = pos.x - (rect.right * 0.5f);
		auto half_y = pos.y - (rect.bottom * 0.5f);
		
		auto x = static_cast<double>(half_x) + baseCrosshairData.xOff;
		auto y = static_cast<double>(half_y) + baseCrosshairData.yOff;

		GFxValue va;
		va.SetNumber(x);
		menu->view->SetVariable("_root.HUDMovieBaseInstance.CrosshairInstance._x", &va, 0);
		va.SetNumber(y);
		menu->view->SetVariable("_root.HUDMovieBaseInstance.CrosshairInstance._y", &va, 0);
	}
}

void Camera::SmoothCamera::SetCrosshairSize(const glm::vec2& size) const {
	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		GFxValue va;
		va.SetNumber(static_cast<double>(size.x));
		menu->view->SetVariable("_root.HUDMovieBaseInstance.Crosshair._width", &va, 0);
		va.SetNumber(static_cast<double>(size.y));
		menu->view->SetVariable("_root.HUDMovieBaseInstance.Crosshair._height", &va, 0);
	}
}

void Camera::SmoothCamera::SetCrosshairEnabled(bool enabled) const {
	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		GFxValue result;
		GFxValue args[2];
		args[0].SetString("SetCrosshairEnabled");
		args[1].SetBool(enabled);
		menu->view->Invoke("call", &result, static_cast<GFxValue*>(args), 2);
	}
}
#pragma endregion

#pragma region Camera getters
// Returns the camera's pitch
float Camera::SmoothCamera::GetCameraPitchRotation(const CorrectedPlayerCamera* camera) const noexcept {
	const auto mat = camera->cameraNode->m_localTransform.rot;
	const auto a = glm::clamp(-mat.data[2][1], -1.0f, 1.0f);
	return glm::asin(a);
}

// Returns the camera's yaw
float Camera::SmoothCamera::GetCameraYawRotation(const CorrectedPlayerCamera* camera) const noexcept {
	const auto mtx = reinterpret_cast<NiCamera*>(camera->cameraNode->m_children.m_data[0])->m_worldTransform.rot;
	if (mtx.data[0][0] <= 0.0000001f || mtx.data[2][2] <= 0.0000001f) {
		auto ab = glm::atan(mtx.data[0][2], mtx.data[1][2]);
		return ab - mmath::half_pi;
	}

	return glm::atan(mtx.data[0][0], mtx.data[1][0]);

}

// Returns the camera's current zoom level - Camera must extend ThirdPersonState
float Camera::SmoothCamera::GetCameraZoomScalar(const CorrectedPlayerCamera* camera, uint16_t cameraState) const noexcept {
	const auto state = reinterpret_cast<const CorrectedThirdPersonState*>(camera->cameraStates[cameraState]);
	if (!state) return 0.0f;
	return state->cameraZoom + SKYRIM_MIN_ZOOM_FRACTION;
}
#pragma endregion

// Selects the correct update method and positions the camera
void Camera::SmoothCamera::UpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera) {
#ifdef _DEBUG
	Profiler prof;
#endif;

	if (!baseCrosshairData.captured) {
		ReadInitialCrosshairInfo();
	}

	auto cameraNode = camera->cameraNode;
	config = Config::GetCurrentConfig();

	// Update states & effects
	const auto pov = UpdateCameraPOVState(player, camera);
	const auto state = GetCurrentCameraState(player, camera);
	const auto actionState = GetCurrentCameraActionState(player, camera);
	offsetState.currentGroup = GetOffsetForState(actionState);
	const auto currentOffset = GetCurrentCameraOffset(player, camera);
	const auto curTime = GetTime();

	// Perform a bit of setup to smooth out camera loading
	if (!firstFrame) {
		lastPosition = lastWorldPosition = currentPosition = {
			cameraNode->m_worldTransform.pos.x,
			cameraNode->m_worldTransform.pos.y,
			cameraNode->m_worldTransform.pos.z
		};
		firstFrame = true;
	}

	// Update transition states
	UpdateTransitionState<glm::vec2, OffsetTransition>(
		curTime,
		config->enableOffsetInterpolation,
		config->offsetInterpDurationSecs,
		config->offsetScalar,
		offsetTransitionState,
		{ currentOffset.x, currentOffset.z }
	);

	if (!povWasPressed) {
		UpdateTransitionState<float, ZoomTransition>(
			curTime,
			config->enableZoomInterpolation,
			config->zoomInterpDurationSecs,
			config->zoomScalar,
			zoomTransitionState,
			currentOffset.y
			);
	} else {
		zoomTransitionState.lastPosition = zoomTransitionState.currentPosition =
			zoomTransitionState.targetPosition = currentOffset.y;
	}
	offsetState.position = {
		offsetTransitionState.currentPosition.x,
		zoomTransitionState.currentPosition,
		offsetTransitionState.currentPosition.y
	};
	
	// Save the camera position
	lastPosition = currentPosition;

	if (config->disableDuringDialog && dialogMenuOpen) {
		lastPosition = lastWorldPosition = currentPosition = {
			cameraNode->m_worldTransform.pos.x,
			cameraNode->m_worldTransform.pos.y,
			cameraNode->m_worldTransform.pos.z
		};
	} else {
		switch (state) {
			case GameState::CameraState::ThirdPerson: {
				if (cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson))) {
					dynamic_cast<State::ThirdpersonState*>(
						cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPerson)).get()
					)->Update(player, camera);
					break;
				}
			}
			case GameState::CameraState::ThirdPersonCombat: {
				if (cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat))) {
					dynamic_cast<State::ThirdpersonCombatState*>(
						cameraStates.at(static_cast<size_t>(GameState::CameraState::ThirdPersonCombat)).get()
					)->Update(player, camera);
					break;
				}
			}
			case GameState::CameraState::Horseback: {
				if (cameraStates.at(static_cast<size_t>(GameState::CameraState::Horseback))) {
					dynamic_cast<State::ThirdpersonHorseState*>(
						cameraStates.at(static_cast<size_t>(GameState::CameraState::Horseback)).get()
					)->Update(player, camera);
					break;
				}
			}

			// Here just for my own reference that these are unused (for now)
			case GameState::CameraState::FirstPerson:
			case GameState::CameraState::KillMove:
			case GameState::CameraState::Tweening:
			case GameState::CameraState::Transitioning:
			case GameState::CameraState::UsingObject:
			case GameState::CameraState::Vanity:
			case GameState::CameraState::Free:
			case GameState::CameraState::IronSights:
			case GameState::CameraState::Furniture:
			case GameState::CameraState::Bleedout:
			case GameState::CameraState::Dragon:
			case GameState::CameraState::Unknown:
			default: {
				SetCrosshairEnabled(true);
				SetCrosshairPosition({ 640.0f, 360.0f });
				SetCrosshairSize({ baseCrosshairData.xScale, baseCrosshairData.yScale });
				lastPosition = lastWorldPosition = currentPosition = {
					cameraNode->m_worldTransform.pos.x,
					cameraNode->m_worldTransform.pos.y,
					cameraNode->m_worldTransform.pos.z
				};
				break;
			}
		}
	}

	povWasPressed = false;

#ifdef _DEBUG
	auto snap = prof.Snap();
	int z = 0;
#endif;
}