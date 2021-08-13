#include "camera_states/base_third.h"
#include "camera.h"
#include "thirdperson.h"
#include "raycast.h"
#include "crosshair.h"

Camera::State::BaseThird::BaseThird(Thirdperson* camera) noexcept : camera(camera) {}

Camera::State::BaseThird::~BaseThird() {}

void Camera::State::BaseThird::Update(PlayerCharacter* player, const Actor* cameraRef,
	const CorrectedPlayerCamera* cameraState) noexcept
{}

void Camera::State::BaseThird::StateHandOff(BaseThird* nextState) const noexcept {
	if (!nextState) return;
	nextState->interpSmoother = interpSmoother;
	nextState->wasInterpLastFrame = wasInterpLastFrame;
}

// Returns the current camera state
GameState::CameraState Camera::State::BaseThird::GetCameraState() const noexcept {
	return camera->m_camera->GetCurrentCameraState();
}

// Returns the current camera action state
Camera::CameraActionState Camera::State::BaseThird::GetCameraActionState() const noexcept {
	return camera->m_camera->GetCurrentCameraActionState();
}

// Get the last camera position
mmath::Position& Camera::State::BaseThird::GetLastCameraPosition() const noexcept {
	return camera->lastPosition;
}

// Get the current camera position
mmath::Position& Camera::State::BaseThird::GetCameraPosition() const noexcept {
	return camera->currentPosition;
}

const NiFrustum& Camera::State::BaseThird::GetFrustum() const noexcept {
	return camera->m_camera->GetFrustum();
}

// Sets the camera position
void Camera::State::BaseThird::SetCameraPosition(const glm::vec3& pos, const PlayerCharacter* player,
	const CorrectedPlayerCamera* playerCamera) noexcept
{
	camera->SetPosition(pos, playerCamera);
	ApplyLocalSpaceGameOffsets(player, playerCamera);
}

// Performs a ray cast and returns a new position based on the result
glm::vec3 Camera::State::BaseThird::ComputeRaycast(const glm::vec3& rayStart, const glm::vec3& rayEnd) {
	constexpr float hullSize = 15.0f;
	const auto rayStart4 = glm::vec4(rayStart.x, rayStart.y, rayStart.z, 0.0f);
	const auto rayEnd4 = glm::vec4(rayEnd.x, rayEnd.y, rayEnd.z, 0.0f);
	const auto result = Raycast::CastRay(rayStart4, rayEnd4, hullSize);
	return result.hit ?
		result.hitPos + (result.rayNormal * glm::min(result.rayLength, hullSize)) :
		rayEnd;
}

// Clamps the camera position based on offset clamp settings
glm::vec3 Camera::State::BaseThird::ComputeOffsetClamping(const TESObjectREFR* ref,
	const glm::vec3& cameraLocalOffset, const glm::vec3& cameraWorldTarget, const glm::vec3& cameraPosition) const
{
	// This is the position we ideally want to be in, before interpolation
	const auto expectedPosition = cameraWorldTarget + cameraLocalOffset;
	// This extracts the interpolation vector from the camera position
	const auto interpVector = cameraPosition - expectedPosition;

	// Decompose the interpVector to player-local axes
	glm::vec3 forward, right, up, coef;
	mmath::DecomposeToBasis(
		interpVector,
		{ camera->GetCameraRotation().euler.x, ref->rot.y, camera->GetCameraRotation().euler.y },
		forward, right, up, coef
	);

	auto& [mins, maxs] = camera->GetDistanceClamping();

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

glm::vec3 Camera::State::BaseThird::ComputeOffsetClamping(const TESObjectREFR* ref, const glm::vec3& cameraWorldTarget,
	const glm::vec3& cameraPosition) const
{
	glm::vec3 forward, right, up, coef;
	mmath::DecomposeToBasis(
		cameraPosition - cameraWorldTarget,
		{ camera->GetCameraRotation().euler.x, ref->rot.y, camera->GetCameraRotation().euler.y },
		forward, right, up, coef
	);

	const auto& [mins, maxs] = camera->GetDistanceClamping();

	// Now we can do whatever we want to the interp vector in axis aligned space
	if (camera->config->cameraDistanceClampXEnable)
		coef.x = glm::clamp(coef.x, mins.x, maxs.x);
	if (camera->config->cameraDistanceClampYEnable)
		coef.y = glm::clamp(coef.y, mins.y, maxs.y);
	if (camera->config->cameraDistanceClampZEnable)
		coef.z = glm::clamp(coef.z, mins.z, maxs.z);

	// Now recompose with the new coefficients and add back the world position
	auto mod = (forward * coef.x) + (right * coef.y) + (up * coef.z) + cameraWorldTarget;
	auto d = glm::distance(mod, cameraPosition);
	return mod;
}

void Camera::State::BaseThird::UpdateCrosshair(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const {
	if (camera->m_camera->InLoadingScreen()) return;

	auto use3D = false;
	if (GameState::IsRangedWeaponDrawn(player)) {
		use3D = GameState::IsBowDrawn(player) && GetConfig()->use3DBowAimCrosshair;
	} else if (GameState::IsMagicDrawn(player)) {
		use3D = GetConfig()->use3DMagicCrosshair;
	}

	if (GameState::IsWeaponDrawn(player)) {
		if (GetConfig()->hideCrosshairMeleeCombat && GameState::IsMeleeWeaponDrawn(player) && !GameState::IsMagicDrawn(player)) {
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
void Camera::State::BaseThird::UpdateCrosshairPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const {
	if (camera->m_camera->InLoadingScreen()) return;
	camera->crosshair->UpdateCrosshairPosition(
		player, playerCamera,
		camera->GetAimRotation(camera->currentFocusObject, playerCamera),
		camera->m_camera->worldToScaleform
	);
}

// Returns the euler rotation of the camera
mmath::Rotation& Camera::State::BaseThird::GetCameraRotation() const noexcept {
	return camera->rotation;
}

// Returns the local offsets to apply to the camera
glm::vec3 Camera::State::BaseThird::GetCameraOffsetStatePosition() const noexcept {
	return camera->offsetState.position;
}

// Returns the world position to apply local offsets to
glm::vec3 Camera::State::BaseThird::GetCameraWorldPosition(const TESObjectREFR* ref, const CorrectedPlayerCamera* playerCamera) const {
	return camera->GetCurrentCameraTargetWorldPosition(ref, playerCamera);
}

// Performs all camera offset math using the view rotation matrix and local offsets, returns a local position
glm::vec3 Camera::State::BaseThird::GetTransformedCameraLocalPosition(const CorrectedPlayerCamera* playerCamera) const {
	const auto cameraLocal = GetCameraOffsetStatePosition();
	auto translated = camera->rotation.ToRotationMatrix() * glm::vec4(
		cameraLocal.x,
		cameraLocal.y - camera->config->minCameraFollowDistance + camera->zoomTransitionState.currentPosition,
		0.0f,
		1.0f
	);
	translated.z += cameraLocal.z;
	return static_cast<glm::vec3>(translated);
}

// Interpolates the given position
glm::vec3 Camera::State::BaseThird::UpdateInterpolatedLocalPosition(const PlayerCharacter* player, const glm::vec3& rot) {
	const auto pos = mmath::Interpolate<glm::dvec3, double>(
		camera->lastPosition.local, rot,
		static_cast<double>(camera->localSmoother.BlendResult<float, mmath::Local::Yes>(
			glm::length(rot - camera->lastPosition.local),
			!GetConfig()->disableDeltaTime
		))
	);
	camera->currentPosition.local = static_cast<glm::vec3>(pos);
	return pos;
}

// Interpolates the given position, stores last interpolated position
glm::vec3 Camera::State::BaseThird::UpdateInterpolatedWorldPosition(const PlayerCharacter* player, const glm::vec3& fromPos,
	const glm::vec3& pos, const float distance)
{
	if (!GetConfig()->enableInterp) {
		// If interp is off, smooth the transition to a fixed position state
		if (wasInterpLastFrame) {
			wasInterpLastFrame = false;
			interpSmoother.startTime = GameTime::CurTime();
			interpSmoother.running = true;
			interpSmoother.lastPosition = fromPos;
		}

		if (interpSmoother.running) {
			return mmath::UpdateFixedTransitionGoal<glm::vec3, InterpSmoother>(
				GameTime::CurTime(), GetConfig()->globalInterpDisableSmoothing,
				GetConfig()->globalInterpDisableMehtod, interpSmoother, pos
			);
		} else {
			return pos;
		}
	}

	// And here
	if (!camera->IsInterpAllowed(player)) {
		if (wasInterpLastFrame) {
			wasInterpLastFrame = false;
			interpSmoother.startTime = GameTime::CurTime();
			interpSmoother.running = true;
			interpSmoother.lastPosition = fromPos;
		}

		if (interpSmoother.running) {
			const auto itp = camera->globalSmoother.BlendResult(distance, !GetConfig()->disableDeltaTime);
			const auto blend = mmath::UpdateFixedTransitionGoal<glm::vec3, InterpSmoother>(
				GameTime::CurTime(), GetConfig()->globalInterpDisableSmoothing,
				GetConfig()->globalInterpDisableMehtod, interpSmoother, pos
			);
			
			glm::dvec3 fromLerped = {};
			if (GetConfig()->separateZInterp) {
				const auto xy = mmath::Interpolate<glm::dvec3, double>(fromPos, pos, itp);
				const auto z = mmath::Interpolate<glm::dvec3, double>(
					fromPos, pos, camera->GetCurrentSmoothingScalar(player, distance, ScalarSelector::SepZ)
				);
				fromLerped = { xy.x, xy.y, z.z };

			} else {
				const auto ret = mmath::Interpolate<glm::dvec3, double>(fromPos, pos, itp);
				fromLerped = static_cast<glm::vec3>(ret);
			}

			const auto scalar = glm::clamp(
				static_cast<float>(GameTime::CurTime() - interpSmoother.startTime) / glm::max(GetConfig()->globalInterpDisableSmoothing, 0.01f),
				0.0f, 1.0f
			);

			return static_cast<glm::vec3>(mmath::Interpolate<glm::dvec3, double>(fromLerped, pos, scalar));
		} else {
			return pos;
		}
	}

	// Otherwise, we lerp
	wasInterpLastFrame = true;
	const auto itp = camera->globalSmoother.BlendResult(distance, !GetConfig()->disableDeltaTime);

	if (GetConfig()->separateZInterp) {
		const auto xy = mmath::Interpolate<glm::dvec3, double>(fromPos, pos, itp);
		const auto z = mmath::Interpolate<glm::dvec3, double>(
			fromPos, pos, camera->GetCurrentSmoothingScalar(player, distance, ScalarSelector::SepZ)
		);
		return { xy.x, xy.y, z.z };

	} else {
		const auto ret = mmath::Interpolate<glm::dvec3, double>(fromPos, pos, itp);
		return static_cast<glm::vec3>(ret);
	}
}

bool Camera::State::BaseThird::IsLocalInterpAllowed() const noexcept {
	return GetConfig()->separateLocalInterp;
}

void Camera::State::BaseThird::ApplyLocalSpaceGameOffsets(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera)
	noexcept
{
	auto state = reinterpret_cast<CorrectedThirdPersonState*>(playerCamera->cameraState);
	const auto& euler = camera->rotation.euler;

	glm::vec3 f, s, u, coef;
	mmath::DecomposeToBasis(
		camera->currentPosition.world - GetCameraWorldPosition(camera->currentFocusObject, playerCamera),
		{ euler.x, 0.0f, euler.y },
		f, s, u, coef
	);

	state->rotation = camera->rotation.ToNiQuat();
	state->yaw1 = euler.y;
	state->yaw2 = euler.y;

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

// Returns true if the player is moving
bool Camera::State::BaseThird::IsPlayerMoving(const PlayerCharacter* player) const noexcept {
	return (GameState::IsWalking(player) || GameState::IsRunning(player) || GameState::IsSprinting(player));
}

// Returns the user config
const Config::UserConfig* const Camera::State::BaseThird::GetConfig() const noexcept {
	return camera->config;
}