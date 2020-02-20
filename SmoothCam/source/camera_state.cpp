#include "pch.h"
#include "camera_state.h"
#include "camera.h"

Camera::State::BaseCameraState::BaseCameraState(Camera::SmoothCamera* camera) noexcept : camera(camera) {}

Camera::State::BaseCameraState::~BaseCameraState() {}

Camera::CameraState Camera::State::BaseCameraState::GetCameraState() const noexcept {
	return camera->currentState;
}

Camera::CameraActionState Camera::State::BaseCameraState::GetCameraActionState() const noexcept {
	return camera->currentActionState;
}

glm::vec3 Camera::State::BaseCameraState::GetLastCameraPosition() const noexcept {
	return camera->lastPosition;
}

void Camera::State::BaseCameraState::SetCameraPosition(const glm::vec3& pos) noexcept {
	camera->currentPosition = pos;
}

void Camera::State::BaseCameraState::UpdateCameraPosition(const glm::vec3& rayStart, const glm::vec3& rayEnd) {
	constexpr float hullSize = 15.0f;
	const auto rayStart4 = glm::vec4(rayStart.x, rayStart.y, rayStart.z, 0.0f);
	const auto rayEnd4 = glm::vec4(rayEnd.x, rayEnd.y, rayEnd.z, 0.0f);
	const auto result = Raycast::CastRay(rayStart4, rayEnd4, hullSize);
	if (result.hit) {
		// We hit some geometry, back the camera position up along the ray normal to before the hit
		SetCameraPosition(result.hitPos + (result.rayNormal * glm::min(result.rayLength, hullSize)));
	} else {
		// Nothing hit
		SetCameraPosition(rayEnd);
	}
}

void Camera::State::BaseCameraState::UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const {
	if (camera->config->enable3DCrosshair)
		camera->UpdateCrosshairPosition(player, playerCamera);
	else
		camera->SetCrosshairPosition({ 0.0f, 0.0f });
}

void Camera::State::BaseCameraState::SetCrosshairPosition(const glm::vec2& pos) const {
	camera->SetCrosshairPosition(pos);
}

glm::vec2 Camera::State::BaseCameraState::GetCameraRotation(const CorrectedPlayerCamera* playerCamera) const noexcept {
	return {
		camera->GetCameraPitchRotation(playerCamera),
		camera->GetCameraYawRotation(playerCamera)
	};
}

void Camera::State::BaseCameraState::SetCrosshairEnabled(bool enabled) const {
	camera->SetCrosshairEnabled(enabled);
}

glm::vec3 Camera::State::BaseCameraState::GetCameraLocalPosition(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const noexcept {
	return camera->GetCurrentCameraOffset(player, playerCamera);
}

glm::vec3 Camera::State::BaseCameraState::GetCameraWorldPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const {
	return camera->GetCurrentCameraTargetWorldPosition(player, playerCamera);
}

glm::vec3 Camera::State::BaseCameraState::GetInterpolatedPosition(PlayerCharacter* player, const glm::vec3& pos, const float distance) const {
	if (!GetConfig()->enableInterp)
		return pos;

	if (!camera->IsInterpAllowed(player))
		return pos;

	if (GetConfig()->separateZInterp) {
		glm::vec3 curPos = GetLastCameraPosition();
		auto xy = mmath::Interpolate<glm::vec3, float>(
			curPos, pos, camera->GetCurrentSmoothingScalar(distance)
		);

		auto z = mmath::Interpolate<glm::vec3, float>(
			curPos, pos, camera->GetCurrentSmoothingScalar(distance, true)
		);

		return { xy.x, xy.y, z.z };

	} else {
		return mmath::Interpolate<glm::vec3, float>(
			GetLastCameraPosition(), pos, camera->GetCurrentSmoothingScalar(distance)
		);
	}
}

float Camera::State::BaseCameraState::GetFrameDelta() const noexcept {
	return camera->GetFrameDelta();
}

bool Camera::State::BaseCameraState::IsPlayerMoving(const PlayerCharacter* player) const noexcept {
	return (camera->IsWalking(player) || camera->IsRunning(player) || camera->IsSprinting(player));
}

bool Camera::State::BaseCameraState::IsWeaponDrawn(const PlayerCharacter* player) const noexcept {
	return camera->IsWeaponDrawn(player);
}

bool Camera::State::BaseCameraState::IsMeleeWeaponDrawn(PlayerCharacter* player) const noexcept {
	return camera->IsMeleeWeaponDrawn(player);
}

const Config::UserConfig* const Camera::State::BaseCameraState::GetConfig() const noexcept {
	return camera->config;
}