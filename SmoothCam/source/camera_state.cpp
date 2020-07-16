#include "camera_state.h"
#include "camera.h"

Camera::State::BaseCameraState::BaseCameraState(Camera::SmoothCamera* camera) noexcept : camera(camera) {}

Camera::State::BaseCameraState::~BaseCameraState() {}

// Returns the current camera state
GameState::CameraState Camera::State::BaseCameraState::GetCameraState() const noexcept {
	return camera->currentState;
}

// Returns the current camera action state
Camera::CameraActionState Camera::State::BaseCameraState::GetCameraActionState() const noexcept {
	return camera->currentActionState;
}

// Returns the position of the camera during the last frame
glm::vec3 Camera::State::BaseCameraState::GetLastCameraPosition() const noexcept {
	return camera->lastPosition;
}

// Sets the camera position
void Camera::State::BaseCameraState::SetCameraPosition(const glm::vec3& pos, const CorrectedPlayerCamera* playerCamera) noexcept {
	camera->SetPosition(pos, playerCamera);
}

// Performs a ray cast and returns a new position based on the result
glm::vec3 Camera::State::BaseCameraState::ComputeRaycast(const glm::vec3& rayStart, const glm::vec3& rayEnd) {
	constexpr float hullSize = 15.0f;
	const auto rayStart4 = glm::vec4(rayStart.x, rayStart.y, rayStart.z, 0.0f);
	const auto rayEnd4 = glm::vec4(rayEnd.x, rayEnd.y, rayEnd.z, 0.0f);
	const auto result = Raycast::CastRay(rayStart4, rayEnd4, hullSize);
	return result.hit ?
		result.hitPos + (result.rayNormal * glm::min(result.rayLength, hullSize)) :
		rayEnd;
}

// Clamps the camera position based on offset clamp settings
glm::vec3 Camera::State::BaseCameraState::ComputeOffsetClamping(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera,
	const glm::vec3& cameraLocalOffset, const glm::vec3& cameraWorldTarget, const glm::vec3& cameraPosition) const
{
	const auto local = cameraLocalOffset;
	const auto target = cameraWorldTarget;
	const auto matView = camera->GetViewMatrix(player, playerCamera);

	// This is the position we ideally want to be in, before interpolation
	const auto expectedPosition = target + static_cast<glm::vec3>(local);
	// This extracts the interpolation vector from the camera position
	const auto interpVector = cameraPosition - expectedPosition;

	// Decompose the interpVector to player-local axes
	glm::vec3 forward, right, up, coef;
	mmath::DecomposeToBasis(
		interpVector,
		{ player->rot.x, player->rot.y, player->rot.z },
		forward, right, up, coef
	);

	// Now we can do whatever we want to the interp vector in axis aligned space
	if (camera->config->cameraDistanceClampXEnable)
		coef.x = glm::clamp(coef.x, camera->config->cameraDistanceClampXMin, camera->config->cameraDistanceClampXMax);
	if (camera->config->cameraDistanceClampYEnable)
		coef.y = glm::clamp(coef.y, camera->config->cameraDistanceClampYMin, camera->config->cameraDistanceClampYMax);
	if (camera->config->cameraDistanceClampZEnable)
		coef.z = glm::clamp(coef.z, camera->config->cameraDistanceClampZMin, camera->config->cameraDistanceClampZMax);

	// Now recompose with the new coefficients and add back the world position
	return (forward * coef.x) + (right * coef.y) + (up * coef.z) + expectedPosition;
}

glm::vec3 Camera::State::BaseCameraState::ComputeOffsetClamping(PlayerCharacter* player, const glm::vec3& cameraWorldTarget,
	const glm::vec3& cameraPosition) const
{
	const auto local = cameraPosition - cameraWorldTarget;
	glm::vec3 forward, right, up, coef;
	mmath::DecomposeToBasis(
		local,
		{ player->rot.x, player->rot.y, player->rot.z },
		forward, right, up, coef
	);

	// Now we can do whatever we want to the interp vector in axis aligned space
	if (camera->config->cameraDistanceClampXEnable)
		coef.x = glm::clamp(coef.x, camera->config->cameraDistanceClampXMin, camera->config->cameraDistanceClampXMax);
	if (camera->config->cameraDistanceClampYEnable)
		coef.y = glm::clamp(coef.y, camera->config->cameraDistanceClampYMin, camera->config->cameraDistanceClampYMax);
	if (camera->config->cameraDistanceClampZEnable)
		coef.z = glm::clamp(coef.z, camera->config->cameraDistanceClampZMin, camera->config->cameraDistanceClampZMax);

	// Now recompose with the new coefficients and add back the world position
	return (forward * coef.x) + (right * coef.y) + (up * coef.z) + cameraWorldTarget/* + expectedPosition*/;
}

void Camera::State::BaseCameraState::UpdateCrosshair(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const {
	auto use3D = false;
	if (GameState::IsRangedWeaponDrawn(player)) {
		use3D = GameState::IsBowDrawn(player) && GetConfig()->use3DBowAimCrosshair;
	} else if (GameState::IsMagicDrawn(player)) {
		use3D = GetConfig()->use3DMagicCrosshair;
	}

	if (IsWeaponDrawn(player)) {
		if (GetConfig()->hideCrosshairMeleeCombat && IsMeleeWeaponDrawn(player)) {
			SetCrosshairEnabled(false);
		} else {
			SetCrosshairEnabled(true);

			if (use3D) {
				UpdateCrosshairPosition(player, playerCamera);
			} else {
				CenterCrosshair();
				camera->SetCrosshairSize({ camera->baseCrosshairData.xScale, camera->baseCrosshairData.yScale });
			}
		}
	} else {
		if (GetConfig()->hideNonCombatCrosshair) {
			SetCrosshairEnabled(false);
		} else {
			SetCrosshairEnabled(true);
			CenterCrosshair();
			camera->SetCrosshairSize({ camera->baseCrosshairData.xScale, camera->baseCrosshairData.yScale });
		}
	}
}

// Updates the 3D crosshair position, performing all logic internally
void Camera::State::BaseCameraState::UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const {
	camera->UpdateCrosshairPosition(player, playerCamera);
}

// Directly sets the crosshair position
void Camera::State::BaseCameraState::SetCrosshairPosition(const glm::vec2& pos) const {
	camera->SetCrosshairPosition(pos);
}

void Camera::State::BaseCameraState::CenterCrosshair() const {
	camera->CenterCrosshair();
}

// Toggles visibility of the crosshair
void Camera::State::BaseCameraState::SetCrosshairEnabled(bool enabled) const {
	camera->SetCrosshairEnabled(enabled);
}

// Returns a rotation matrix to use with rotating the camera
glm::mat4 Camera::State::BaseCameraState::GetViewMatrix(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const noexcept {
	return camera->GetViewMatrix(player, playerCamera);
}

// Returns the euler rotation of the camera
glm::vec2 Camera::State::BaseCameraState::GetCameraRotation(const CorrectedPlayerCamera* playerCamera) const noexcept {
	return {
		camera->GetCameraPitchRotation(playerCamera),
		camera->GetCameraYawRotation(playerCamera)
	};
}

// Returns the local offsets to apply to the camera
glm::vec3 Camera::State::BaseCameraState::GetCameraLocalPosition(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const noexcept {
	//return camera->GetCurrentCameraOffset(player, playerCamera);
	return camera->offsetState.position;
}

// Returns the world position to apply local offsets to
glm::vec3 Camera::State::BaseCameraState::GetCameraWorldPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const {
	return camera->GetCurrentCameraTargetWorldPosition(player, playerCamera);
}

// Performs all camera offset math using the view rotation matrix and local offsets, returns a local position
glm::vec3 Camera::State::BaseCameraState::GetTransformedCameraLocalPosition(PlayerCharacter* player,
	const CorrectedPlayerCamera* playerCamera) const
{
	const auto cameraLocal = GetCameraLocalPosition(player, playerCamera);
	auto translated = camera->GetViewMatrix(player, playerCamera) * glm::vec4(
		cameraLocal.x,
		cameraLocal.y,
		0.0f,
		1.0f
	);
	translated.z += cameraLocal.z;
	return static_cast<glm::vec3>(translated);
}

// Interpolates the given position
glm::vec3 Camera::State::BaseCameraState::UpdateInterpolatedLocalPosition(PlayerCharacter* player, const glm::vec3& rot) {
	if (camera->lastLocalPosition == glm::vec3(0.0f)) {
		// Store the first valid position for future interpolation
		StoreLastLocalPosition(rot);
		return rot;
	}
	
	const auto pos = mmath::Interpolate<glm::vec3, float>(
		camera->lastLocalPosition, rot, camera->GetCurrentSmoothingScalar(
			camera->config->localScalarRate,
			ScalarSelector::LocalSpace
		)
	);
	StoreLastLocalPosition(pos);
	return pos;
}

// Interpolates the given position, stores last interpolated position
glm::vec3 Camera::State::BaseCameraState::UpdateInterpolatedWorldPosition(PlayerCharacter* player, const glm::vec3& pos, const float distance) {
	if (!GetConfig()->enableInterp) {
		return pos;
	}

	if (!camera->IsInterpAllowed(player)) {
		return pos;
	}

	if (GetConfig()->separateZInterp) {
		const auto xy = mmath::Interpolate<glm::vec3, float>(
			camera->lastWorldPosition, pos, camera->GetCurrentSmoothingScalar(distance)
		);

		const auto z = mmath::Interpolate<glm::vec3, float>(
			camera->lastWorldPosition, pos, camera->GetCurrentSmoothingScalar(distance, ScalarSelector::SepZ)
		);

		return { xy.x, xy.y, z.z };

	} else {
		const auto ret = mmath::Interpolate<glm::vec3, float>(
			camera->lastWorldPosition, pos, camera->GetCurrentSmoothingScalar(distance)
		);
		return ret;
	}
}

void Camera::State::BaseCameraState::ApplyLocalSpaceGameOffsets(const glm::vec3& pos, const PlayerCharacter* player,
	const CorrectedPlayerCamera* playerCamera)
{
	auto state = reinterpret_cast<CorrectedThirdPersonState*>(playerCamera->cameraState);
	const auto rot = GetCameraRotation(playerCamera);
	glm::vec3 forward, right, up, coef;
	mmath::DecomposeToBasis(
		pos,
		{ rot.x, 0.0f, rot.y },
		forward, right, up, coef
	);

	auto view = glm::quat_cast(GetViewMatrix(player, playerCamera));
	state->rotation.m_fW = view.w;
	state->rotation.m_fX = view.x;
	state->rotation.m_fY = view.y;
	state->rotation.m_fZ = view.z;
	state->yaw1 = rot.y;
	state->yaw2 = rot.y;

	if (GetCameraState() == GameState::CameraState::ThirdPersonCombat && GameState::IsBowDrawn(player)) {
		state->fOverShoulderPosX = 0.0f;
		state->fOverShoulderCombatAddY = 0.0f;
		state->fOverShoulderPosZ = 0.0f;
	} else {
		state->fOverShoulderPosX = coef.x;
		state->fOverShoulderCombatAddY = coef.y;
		state->fOverShoulderPosZ = coef.z;
	}

	state->offsetVector.x = state->fOverShoulderPosX;
	state->offsetVector.y = state->fOverShoulderCombatAddY;
	state->offsetVector.z = state->fOverShoulderPosZ;
}

void Camera::State::BaseCameraState::StoreLastLocalPosition(const glm::vec3& pos) {
	camera->lastLocalPosition = pos;
}

void Camera::State::BaseCameraState::StoreLastWorldPosition(const glm::vec3& pos) {
	camera->lastWorldPosition = pos;
}

glm::vec3 Camera::State::BaseCameraState::GetLastLocalPosition() {
	return camera->lastLocalPosition;
}

glm::vec3 Camera::State::BaseCameraState::GetLastWorldPosition() {
	return camera->lastWorldPosition;
}

// Returns true if the player is moving
bool Camera::State::BaseCameraState::IsPlayerMoving(const PlayerCharacter* player) const noexcept {
	return (GameState::IsWalking(player) || GameState::IsRunning(player) || GameState::IsSprinting(player));
}

// Returns true if any kind of weapon is drawn
bool Camera::State::BaseCameraState::IsWeaponDrawn(const PlayerCharacter* player) const noexcept {
	return GameState::IsWeaponDrawn(player);
}

// Returns true if a melee weapon is drawn
bool Camera::State::BaseCameraState::IsMeleeWeaponDrawn(PlayerCharacter* player) const noexcept {
	return GameState::IsMeleeWeaponDrawn(player);
}

// Returns the user config
const Config::UserConfig* const Camera::State::BaseCameraState::GetConfig() const noexcept {
	return camera->config;
}