#include "camera.h"

float GetFrameDelta() noexcept;

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
const Config::OffsetGroup Camera::SmoothCamera::GetOffsetForState(const CameraActionState state) const noexcept {
	switch (state) {
		case CameraActionState::DisMounting: {
			return config->standing; // Better when dismounting
		}
		case CameraActionState::Sleeping: {
			return config->sitting;
		}
		case CameraActionState::Sitting: {
			return config->sitting;
		}
		case CameraActionState::Sneaking: {
			return config->sneaking;
		}
		case CameraActionState::Aiming: {
			return config->bowAim;
		}
		case CameraActionState::Swimming: {
			return config->swimming;
		}
		case CameraActionState::Sprinting: {
			return config->sprinting;
		}
		case CameraActionState::Walking: {
			return config->walking;
		}
		case CameraActionState::Running: {
			return config->running;
		}
		case CameraActionState::Standing: {
			return config->standing;
		}
		default: {
			return config->standing;
		}
	}
}

// Selects the right offset from an offset group for the player's weapon state
float Camera::SmoothCamera::GetActiveWeaponStateUpOffset(PlayerCharacter* player, const Config::OffsetGroup& group) const noexcept {
	if (!GameState::IsWeaponDrawn(player)) return group.upOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group.combatRangedUpOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group.combatMagicUpOffset;
	}
	return group.combatMeleeUpOffset;
}

// Selects the right offset from an offset group for the player's weapon state
float Camera::SmoothCamera::GetActiveWeaponStateSideOffset(PlayerCharacter* player, const Config::OffsetGroup& group) const noexcept {
	if (!GameState::IsWeaponDrawn(player)) return group.sideOffset;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return group.combatRangedSideOffset;
	}
	if (GameState::IsMagicDrawn(player)) {
		return group.combatMagicSideOffset;
	}
	return group.combatMeleeSideOffset;
}

// Returns the camera height for the current player state
float Camera::SmoothCamera::GetCurrentCameraHeight(PlayerCharacter* player) const noexcept {
	switch (currentState) {
		case GameState::CameraState::Horseback: {
			if (GameState::IsBowDrawn(player)) {
				return config->bowAim.horseUpOffset;
			} else {
				return GetActiveWeaponStateUpOffset(player, config->horseback);
			}
		}
		default:
			break;
	}

	const auto ofs = GetOffsetForState(currentActionState);

	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Aiming:
		case CameraActionState::Swimming: {
			return ofs.upOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateUpOffset(player, ofs);
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
				return GetActiveWeaponStateSideOffset(player, config->horseback);
			}
		}
		default:
			break;
	}

	const auto ofs = GetOffsetForState(currentActionState);
	
	switch (currentActionState) {
		case CameraActionState::DisMounting:
		case CameraActionState::Sleeping:
		case CameraActionState::Sitting:
		case CameraActionState::Aiming:
		case CameraActionState::Swimming: {
			return ofs.sideOffset;
		}
		case CameraActionState::Sneaking:
		case CameraActionState::Sprinting:
		case CameraActionState::Walking:
		case CameraActionState::Running:
		case CameraActionState::Standing: {
			return GetActiveWeaponStateSideOffset(player, ofs);
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
		BSFixedString nodeName = currentState == GameState::CameraState::Horseback ? "HorseSpine2" : "Camera3rd [Cam3]";
		const NiAVObject* node = player->loadedState->node->GetObjectByName(&nodeName.data);
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

	// Data breakpoints for the win!
	// Recalculate the world to screen matrix
	typedef void(*UpdateWorldToScreenMtx)(NiCamera*);
	Offsets::Get<UpdateWorldToScreenMtx>(69271)(cameraNi);
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
		const auto fps = 1.0f / delta;
		const auto mul = -fps * glm::log2(1.0f - remapped);
		interpValue = glm::clamp(1.0f - glm::exp2(-mul * delta), 0.0f, 1.0f);
	} else {
		interpValue = remapped;
	}

	return mmath::RunScalarFunction<float>(scalarMethod, interpValue);
}

// Returns true if interpolation is allowed in the current state
bool Camera::SmoothCamera::IsInterpAllowed(PlayerCharacter* player) const noexcept {
	auto ofs = GetOffsetForState(currentActionState);
	if (currentState == GameState::CameraState::Horseback) {
		if (GameState::IsWeaponDrawn(player) && GameState::IsBowDrawn(player)) {
			return config->bowAim.interpHorseback;
		} else {
			ofs = config->horseback;
		}
	}

	if (!GameState::IsWeaponDrawn(player)) return ofs.interp;
	if (GameState::IsRangedWeaponDrawn(player)) {
		return ofs.interpRangedCombat;
	}
	if (GameState::IsMagicDrawn(player)) {
		return ofs.interpMagicCombat;
	}
	return ofs.interpMeleeCombat;
}

// Constructs the view matrix for the camera
glm::mat4 Camera::SmoothCamera::GetViewMatrix(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) const noexcept {
	// Build a rotation matrix to rotate the camera around pitch and yaw axes.
	auto m = glm::identity<glm::mat4>();
	if (GameState::IsWalking(player) || GameState::IsRunning(player) || GameState::IsSprinting(player)) {
		// When moving the camera yaw is locked to the player yaw
		m = glm::rotate(m, -player->rot.z, Camera::UNIT_UP); // yaw
	} else {
		// Rotate around the player based on camera angles instead
		m = glm::rotate(m, -GetCameraYawRotation(camera), Camera::UNIT_UP); // yaw
	}
	m = glm::rotate(m, -GetCameraPitchRotation(camera), Camera::UNIT_FORWARD); // pitch
	return m;
}
#pragma endregion

#pragma region Crosshair stuff
// Updates the screen position of the crosshair for correct aiming
void Camera::SmoothCamera::UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* camera) const {
	NiPoint3 niOrigin = { 0.01f, 0.01f, 0.01f };
	NiPoint3 niNormal = { 0.0f, 1.00f, 0.0f };

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

	// Bow
	if (GameState::IsBowDrawn(player)) {
		BSFixedString handNodeName = "WEAPON";
		const auto handNode = player->loadedState->node->GetObjectByName(&handNodeName.data);
		if (handNode) {
			niOrigin = handNode->m_worldTransform.pos;

			// Assuming the weapon node is close enough, the arrow seems to use a different normal
			// This gets pretty close
			const auto n = mmath::GetViewVector(
				glm::vec3(0.0, 1.0, 0.0),
				GetCameraPitchRotation(camera) + 0.025f,
				GetCameraYawRotation(camera) + 0.015f
			);
			niNormal = NiPoint3(n.x, n.y, n.z);
		}

	}

	// Cast the aim ray
	constexpr auto rayLength = 8192.0f;
	const auto origin = glm::vec4(niOrigin.x, niOrigin.y, niOrigin.z, 0.0f);
	const auto ray = glm::vec4(niNormal.x, niNormal.y, niNormal.z, 0.0f) * rayLength;
	const auto result = Raycast::CastRay(origin, origin + ray, 0.01f);

	glm::vec2 crosshairPos(0.0f, 0.0f);
	if (result.hit) {
		glm::vec3 screen = {};
		auto pt = NiPoint3(
			result.hitPos.x,
			result.hitPos.y,
			result.hitPos.z
		);

		auto cameraNode = camera->cameraNode;
		auto cameraNi = reinterpret_cast<NiCamera*>(
			cameraNode->m_children.m_size == 0 ?
			nullptr :
			cameraNode->m_children.m_data[0]
		);
		if (!cameraNi) return;

		// Project to screen
		(*WorldPtToScreenPt3_Internal)(
			(float*)cameraNi->m_aafWorldToCam,
			g_viewPort, &pt,
			&screen.x, &screen.y, &screen.z, 1.0f
		);

		// Remap to scaleform coords
		constexpr auto w = 1280.0f;
		constexpr auto h = 720.0f;
		constexpr auto half_w = w * 0.5f;
		constexpr auto half_h = h * 0.5f;
		crosshairPos.x = (screen.x * w) - half_w;
		crosshairPos.y = (screen.y * h) - half_h;
	}

	SetCrosshairPosition(crosshairPos);
}

void Camera::SmoothCamera::SetCrosshairPosition(const glm::vec2& pos) const {
	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		GFxValue result;
		GFxValue args[3];
		args[0].SetString("SetCrosshairPosition");
		args[1].SetNumber(static_cast<double>(pos.x) + 23.0); // @TODO: These offsets were obtained by just comparing screenshots
		args[2].SetNumber(static_cast<double>(pos.y) - 115.0); // I really need to figure out what is actually going on here, rather than use magic numbers
		menu->view->Invoke("call", &result, static_cast<GFxValue*>(args), 3);
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
	const auto a = glm::clamp(-mat.data[2][1], -0.99f, 0.99f);
	return glm::asin(a);
}

// Returns the camera's yaw
float Camera::SmoothCamera::GetCameraYawRotation(const CorrectedPlayerCamera* camera) const noexcept {
	return camera->lookYaw;
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
	auto cameraNode = camera->cameraNode;
	config = Config::GetCurrentConfig();

	// Update states & effects
	const auto pov = UpdateCameraPOVState(player, camera);
	const auto state = GetCurrentCameraState(player, camera);
	const auto actionState = GetCurrentCameraActionState(player, camera);

	// Perform a bit of setup to smooth out camera loading
	if (!firstFrame) {
		lastPosition = currentPosition = {
			cameraNode->m_worldTransform.pos.x,
			cameraNode->m_worldTransform.pos.y,
			cameraNode->m_worldTransform.pos.z
		};
		firstFrame = true;
	}

	// Save the camera position
	lastPosition = currentPosition;

	if (config->disableDuringDialog && dialogMenuOpen) {
		currentPosition = {
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
				currentPosition = {
					cameraNode->m_worldTransform.pos.x,
					cameraNode->m_worldTransform.pos.y,
					cameraNode->m_worldTransform.pos.z
				};
				break;
			}
		}
	}

	povWasPressed = false;
}