#include "camera_state.h"
#include "camera.h"
#include "raycast.h"
#include "crosshair.h"

double CurTime() noexcept;

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
glm::vec3 Camera::State::BaseCameraState::ComputeOffsetClamping(const TESObjectREFR* ref,
	const glm::vec3& cameraLocalOffset, const glm::vec3& cameraWorldTarget, const glm::vec3& cameraPosition) const
{
	const auto local = cameraLocalOffset;
	const auto target = cameraWorldTarget;
	const auto matView = camera->GetViewMatrix();

	// This is the position we ideally want to be in, before interpolation
	const auto expectedPosition = target + static_cast<glm::vec3>(local);
	// This extracts the interpolation vector from the camera position
	const auto interpVector = cameraPosition - expectedPosition;

	// Decompose the interpVector to player-local axes
	glm::vec3 forward, right, up, coef;
	mmath::DecomposeToBasis(
		interpVector,
		{ ref->rot.x, ref->rot.y, ref->rot.z },
		forward, right, up, coef
	);

	auto [mins, maxs] = camera->GetDistanceClamping();

	// Now we can do whatever we want to the interp vector in axis aligned space
	if (camera->config->cameraDistanceClampXEnable)
		coef.x = glm::clamp(coef.x, mins.x, maxs.x);
	if (camera->config->cameraDistanceClampYEnable)
		coef.y = glm::clamp(coef.y, mins.y, maxs.y);
	if (camera->config->cameraDistanceClampZEnable)
		coef.z = glm::clamp(coef.z, mins.z, maxs.z);

	// Now recompose with the new coefficients and add back the world position
	return (forward * coef.x) + (right * coef.y) + (up * coef.z) + expectedPosition;
}

glm::vec3 Camera::State::BaseCameraState::ComputeOffsetClamping(const TESObjectREFR* ref, const glm::vec3& cameraWorldTarget,
	const glm::vec3& cameraPosition) const
{
	const auto local = cameraPosition - cameraWorldTarget;
	glm::vec3 forward, right, up, coef;
	mmath::DecomposeToBasis(
		local,
		{ ref->rot.x, ref->rot.y, ref->rot.z },
		forward, right, up, coef
	);

	auto [mins, maxs] = camera->GetDistanceClamping();

	// Now we can do whatever we want to the interp vector in axis aligned space
	if (camera->config->cameraDistanceClampXEnable)
		coef.x = glm::clamp(coef.x, mins.x, maxs.x);
	if (camera->config->cameraDistanceClampYEnable)
		coef.y = glm::clamp(coef.y, mins.y, maxs.y);
	if (camera->config->cameraDistanceClampZEnable)
		coef.z = glm::clamp(coef.z, mins.z, maxs.z);

	// Now recompose with the new coefficients and add back the world position
	return (forward * coef.x) + (right * coef.y) + (up * coef.z) + cameraWorldTarget/* + expectedPosition*/;
}

void Camera::State::BaseCameraState::UpdateCrosshair(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const {
	if (camera->InLoadingScreen()) return;

	auto use3D = false;
	if (GameState::IsRangedWeaponDrawn(player)) {
		use3D = GameState::IsBowDrawn(player) && GetConfig()->use3DBowAimCrosshair;
	} else if (GameState::IsMagicDrawn(player)) {
		use3D = GetConfig()->use3DMagicCrosshair;
	}

	if (IsWeaponDrawn(player)) {
		if (GetConfig()->hideCrosshairMeleeCombat && IsMeleeWeaponDrawn(player)) {
			camera->crosshair->SetCrosshairEnabled(false);
		} else {
			if (use3D) {
				UpdateCrosshairPosition(player, playerCamera);
			} else {
				camera->crosshair->SetCrosshairEnabled(true);
				camera->crosshair->CenterCrosshair();
				camera->crosshair->SetDefaultSize();
			}
		}
	} else {
		if (GetConfig()->hideNonCombatCrosshair) {
			camera->crosshair->SetCrosshairEnabled(false);
		} else {
			camera->crosshair->SetCrosshairEnabled(true);
			camera->crosshair->CenterCrosshair();
			camera->crosshair->SetDefaultSize();
		}
	}
}

// Updates the 3D crosshair position, performing all logic internally
void Camera::State::BaseCameraState::UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const {
	if (camera->InLoadingScreen()) return;

	camera->crosshair->UpdateCrosshairPosition(
		player,
		playerCamera,
		camera->GetAimRotation(camera->currentFocusObject, playerCamera),
		camera->worldToScaleform
	);
}

// Returns a rotation matrix to use with rotating the camera
glm::mat4 Camera::State::BaseCameraState::GetViewMatrix() const noexcept {
	return camera->GetViewMatrix();
}

// Returns the euler rotation of the camera
glm::vec2 Camera::State::BaseCameraState::GetCameraRotation() const noexcept {
	return {
		camera->GetCameraPitchRotation(),
		camera->GetCameraYawRotation()
	};
}

// Returns the local offsets to apply to the camera
glm::vec3 Camera::State::BaseCameraState::GetCameraLocalPosition() const noexcept {
	return camera->offsetState.position;
}

// Returns the world position to apply local offsets to
glm::vec3 Camera::State::BaseCameraState::GetCameraWorldPosition(const TESObjectREFR* ref, const CorrectedPlayerCamera* playerCamera) const {
	return camera->GetCurrentCameraTargetWorldPosition(ref, playerCamera);
}

// Performs all camera offset math using the view rotation matrix and local offsets, returns a local position
glm::vec3 Camera::State::BaseCameraState::GetTransformedCameraLocalPosition(PlayerCharacter* player,
	const CorrectedPlayerCamera* playerCamera) const
{
	const auto cameraLocal = GetCameraLocalPosition();
	auto translated = camera->GetViewMatrix() * glm::vec4(
		cameraLocal.x,
		cameraLocal.y,
		0.0f,
		1.0f
	);
	translated.z += cameraLocal.z;
	return static_cast<glm::vec3>(translated);
}

// Interpolates the given position
glm::vec3 Camera::State::BaseCameraState::UpdateInterpolatedLocalPosition(const glm::vec3& rot) {
	if (camera->lastLocalPosition == glm::vec3(0.0f)) {
		// Store the first valid position for future interpolation
		StoreLastLocalPosition(rot);
		return rot;
	}
	
	const auto pos = mmath::Interpolate<glm::dvec3, double>(
		camera->lastLocalPosition, rot, camera->GetCurrentSmoothingScalar(
			camera->config->localScalarRate,
			ScalarSelector::LocalSpace
		)
	);
	StoreLastLocalPosition(static_cast<glm::vec3>(pos));
	return pos;
}

// Interpolates the given position, stores last interpolated position
glm::vec3 Camera::State::BaseCameraState::UpdateInterpolatedWorldPosition(PlayerCharacter* player, const glm::vec3& pos, const float distance) {
	if (!GetConfig()->enableInterp) {
		// If interp is off, smooth the transition to a fixed position state
		if (wasInterpLastFrame) {
			wasInterpLastFrame = false;
			interpSmoother.startTime = CurTime();
			interpSmoother.running = true;
			interpSmoother.lastPosition = camera->lastWorldPosition;
		}

		if (interpSmoother.running) {
			return mmath::UpdateFixedTransitionGoal<glm::vec3, InterpSmoother>(
				CurTime(), 2.0f, Config::ScalarMethods::CUBIC_OUT, interpSmoother, pos
			);
		} else {
			return pos;
		}
	}

	// And here
	if (!camera->IsInterpAllowed(player)) {
		if (wasInterpLastFrame) {
			wasInterpLastFrame = false;
			interpSmoother.startTime = CurTime();
			interpSmoother.running = true;
			interpSmoother.lastPosition = camera->lastWorldPosition;
		}

		if (interpSmoother.running) {
			return mmath::UpdateFixedTransitionGoal<glm::vec3, InterpSmoother>(
				CurTime(), 2.0f, Config::ScalarMethods::CUBIC_OUT, interpSmoother, pos
			);
		} else {
			return pos;
		}
	}

	// Otherwise, we lerp
	wasInterpLastFrame = true;

	if (GetConfig()->separateZInterp) {
		const auto xy = mmath::Interpolate<glm::dvec3, double>(
			camera->lastWorldPosition, pos, camera->GetCurrentSmoothingScalar(distance)
		);

		const auto z = mmath::Interpolate<glm::dvec3, double>(
			camera->lastWorldPosition, pos, camera->GetCurrentSmoothingScalar(distance, ScalarSelector::SepZ)
		);

		return { xy.x, xy.y, z.z };

	} else {
		const auto ret = mmath::Interpolate<glm::dvec3, double>(
			camera->lastWorldPosition, pos, camera->GetCurrentSmoothingScalar(distance)
		);
		return static_cast<glm::vec3>(ret);
	}
}

void Camera::State::BaseCameraState::ApplyLocalSpaceGameOffsets(const glm::vec3& pos, const PlayerCharacter* player,
	const CorrectedPlayerCamera* playerCamera)
{
	auto state = reinterpret_cast<CorrectedThirdPersonState*>(playerCamera->cameraState);
	const auto rot = GetCameraRotation();
	glm::vec3 forward, right, up, coef;
	mmath::DecomposeToBasis(
		pos,
		{ rot.x, 0.0f, rot.y },
		forward, right, up, coef
	);

	auto view = glm::quat_cast(GetViewMatrix());
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