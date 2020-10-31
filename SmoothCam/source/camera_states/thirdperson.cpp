#include "camera_states/thirdperson.h"
#include "camera.h"

/*
 * The base thirdperson camera logic - used when no weapon is drawn.
 */

Camera::State::ThirdpersonState::ThirdpersonState(Camera::SmoothCamera* camera) noexcept : BaseCameraState(camera) {

}

void Camera::State::ThirdpersonState::OnBegin(const PlayerCharacter* player, const TESObjectREFR* cameraRef, const CorrectedPlayerCamera* camera,
	BaseCameraState* fromState)
{

}

void Camera::State::ThirdpersonState::OnEnd(const PlayerCharacter* player, const TESObjectREFR* cameraRef, const CorrectedPlayerCamera* camera,
	BaseCameraState* nextState)
{
	StateHandOff(nextState);
}

void Camera::State::ThirdpersonState::Update(PlayerCharacter* player, const TESObjectREFR* cameraRef, const CorrectedPlayerCamera* camera) {
	// Get our computed local-space xyz offset.
	const auto cameraLocal = GetCameraOffsetStatePosition();
	// Get the base world position for the camera which we will offset with the local-space values.
	const auto worldTarget = GetCameraWorldPosition(cameraRef, camera);
	// Transform the camera offsets based on the computed view matrix
	const auto transformedLocalPos = GetTransformedCameraLocalPosition();
	// Define the starting point for our raycast
	const auto start = worldTarget + glm::vec3(0.0f, 0.0f, cameraLocal.z);

	glm::vec3 preFinalPos;
	if (GetConfig()->separateLocalInterp) {
		// Handle separate local-space interpolation
		const auto loc = UpdateInterpolatedLocalPosition(transformedLocalPos);

		const auto& last = GetLastCameraPosition();
		// Last offset position from ref
		const auto lastWorld = last.world - last.local;

		// And the world target
		const auto lerpedWorldPos = UpdateInterpolatedWorldPosition(
			player, lastWorld, worldTarget,
			glm::length(lastWorld - worldTarget)
		);
		
		// Compute offset clamping if enabled
		preFinalPos = ComputeOffsetClamping(cameraRef, worldTarget, lerpedWorldPos) + loc;

	} else {
		// Combined case

		// Add the final local space transformation to the player postion
		const auto targetWorldPos = worldTarget + transformedLocalPos;

		// Now lerp it based on camera distance to player position
		const auto lerpedWorldPos = UpdateInterpolatedWorldPosition(
			player, GetLastCameraPosition().world, targetWorldPos,
			glm::length(targetWorldPos - worldTarget)
		);

		// Compute offset clamping if enabled
		preFinalPos = ComputeOffsetClamping(cameraRef, transformedLocalPos, worldTarget, lerpedWorldPos);
	}

	// Cast our ray and update the camera position
	const auto finalPos = ComputeRaycast(start, preFinalPos);
	// Set the position
	SetCameraPosition(finalPos, player, camera);
	// Update crosshair visibility
	UpdateCrosshair(player, camera);
}