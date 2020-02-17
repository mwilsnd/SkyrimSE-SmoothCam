#include "pch.h"
#include "camera_states/thirdperson_horse.h"
#include "camera.h"

/*
 * The thirdperson horse state is currently exactly the same as thirdperson combat.
 * I might end up doing some slightly different logic here in the future so for now this state will hang around.
 */

Camera::State::ThirdpersonHorseState::ThirdpersonHorseState(Camera::SmoothCamera* camera) noexcept : BaseCameraState(camera) {

}

void Camera::State::ThirdpersonHorseState::OnBegin(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) {

}

void Camera::State::ThirdpersonHorseState::OnEnd(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) {
	SetCrosshairPosition({ 0, 0 });
}

void Camera::State::ThirdpersonHorseState::Update(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) {
	// Get the current pitch and yaw values the game has set for the camera.
	const auto cameraAngles = GetCameraRotation(camera);
	// Get our computed local-space xyz offset.
	const auto cameraLocal = GetCameraLocalPosition(player, camera);
	// Get the base world position for the camera which we will offset with the local-space values.
	const auto worldTarget = GetCameraWorldPosition(player, camera);

	// Build a rotation matrix to rotate the camera around pitch and yaw axes.
	auto m = glm::identity<glm::mat4>();
	m = glm::rotate(m, -cameraAngles.x, Camera::UNIT_FORWARD); // pitch

	// Rotate around just x-y, if we include z then the camera will be pitching around an offset which is not what we want.
	auto translated = m * glm::vec4(
		cameraLocal.x,
		cameraLocal.y,
		0.0f,
		1.0f
	);

	// Now rotate around yaw
	m = glm::identity<glm::mat4>();
	if (IsPlayerMoving(player)) {
		// When moving the camera yaw is locked to the player yaw
		m = glm::rotate(m, -player->rot.z, Camera::UNIT_UP); // yaw
	} else {
		// Rotate around the player based on camera angles instead
		m = glm::rotate(m, -cameraAngles.y, Camera::UNIT_UP); // yaw
	}
	translated = m * translated;

	// Apply the height offset here to get proper rotation around the point we want
	translated.z += cameraLocal.z;

	// Define the starting point for our raycast
	const auto start = worldTarget + glm::vec3(0.0f, 0.0f, cameraLocal.z);
	// Add the final local space transformation to the player postion
	const auto finalPos = worldTarget + glm::vec3(translated);
	// Now lerp it based on camera distance to player position
	const auto lerped = GetInterpolatedPosition(finalPos, glm::length(finalPos - worldTarget));

	// Cast our ray and update the camera position
	UpdateCameraPosition(start, lerped);

	// Update the crosshair
	if (IsWeaponDrawn(player))
		UpdateCrosshairPosition(player, camera);
	else
		SetCrosshairPosition({ 0, 0 });
}