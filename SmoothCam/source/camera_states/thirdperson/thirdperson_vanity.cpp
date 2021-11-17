#include "camera_states/thirdperson/thirdperson_vanity.h"
#include "thirdperson.h"

/*
* The base thirdperson camera logic - used when no weapon is drawn.
*/
Camera::State::ThirdpersonVanityState::ThirdpersonVanityState(Thirdperson* camera) noexcept : BaseThird(camera) {}

void Camera::State::ThirdpersonVanityState::OnBegin(RE::PlayerCharacter*, RE::Actor*, RE::PlayerCamera*,
	BaseThird*) noexcept
{

}

bool Camera::State::ThirdpersonVanityState::OnEnd(RE::PlayerCharacter*, RE::Actor*, RE::PlayerCamera*,
	BaseThird* nextState, bool) noexcept
{
	StateHandOff(nextState);
	return false;
}

void Camera::State::ThirdpersonVanityState::Update(RE::PlayerCharacter* player, RE::Actor* cameraRef, RE::PlayerCamera* playerCamera)
	noexcept
{
	BaseThird::Update(player, cameraRef, playerCamera);

	const auto state = reinterpret_cast<SkyrimSE::AutoVanityState*>(playerCamera->cameraStates[RE::CameraState::kAutoVanity].get());
	if (!state) return;

	// Get our computed local-space xyz offset.
	const auto cameraLocal = GetCameraOffsetStatePosition();
	// Get the base world position for the camera which we will offset with the local-space values.
	const auto worldTarget = GetCameraWorldPosition(cameraRef);
	
	// Now rotate based on vanity yaw
	auto rot = glm::rotate(glm::identity<glm::mat4>(), state->yaw * -1.0f, { 0.0f, 0.0f, 1.0f });
	auto transformedLocalPos = rot * glm::vec4(
		cameraLocal.x,
		cameraLocal.y - GetConfig()->minCameraFollowDistance + camera->GetZoomTransitionState().currentPosition,
		0.0f, 1.0f
	);
	transformedLocalPos.z += cameraLocal.z;

	// We don't do interpolation or offset clamping in the vanity state
	glm::vec3 preFinalPos = worldTarget + static_cast<glm::vec3>(transformedLocalPos);

	glm::vec3 f, s, u, coef;
	mmath::DecomposeToBasis(
		preFinalPos,
		{ GetCameraRotation().euler.x, 0.0f, GetCameraRotation().euler.y },
		f, s, u, coef
	);

	// Cast from the player out towards the X offset, rotated
	// The end position of that ray becomes the origin for our primary distance ray
	// for camera collision.
	// Doing this ensures the origin of our collision ray is never occluded - We just
	// need to make sure the hull size for our origin test ray is larger than the primary
	// collision ray.
	constexpr auto hullSize = 30.0f;

	// Ray origin from the player, plus Z offset
	const auto playerOrigin = worldTarget + glm::vec3(0.0f, 0.0f, cameraLocal.z);
	// Towards the +X offset
	auto rayXOrigin = playerOrigin + (f * cameraLocal.x);
	const auto resultPtoX = Raycast::CastRay(glm::vec4(playerOrigin, 0.0f), glm::vec4(rayXOrigin, 0.0f), hullSize);

	// And if we hit, that hit position becomes the new origin for the next ray
	if (resultPtoX.hit)
		rayXOrigin = resultPtoX.hitPos + (resultPtoX.rayNormal * glm::min(resultPtoX.rayLength, hullSize));

	// Otherwise, we just cast from +X offset like normal as nothing is overlapping
	// Cast from origin towards the camera, Get back final position
	glm::vec3 finalPos;
	if (GetConfig()->pitchZoomAfterInterp) {
		coef.y -= camera->GetPitchZoom();
		const auto pitchZoomedOffset = (f * coef.x) + (s * coef.y) + (u * coef.z);
		finalPos = ComputeRaycast(rayXOrigin, pitchZoomedOffset);
	} else {
		finalPos = ComputeRaycast(rayXOrigin, preFinalPos);
	}

	// Set the position to the ray hit position
	// This sets the rendering position of the camera, and applies game offsets
	SetCameraPosition(finalPos, player, playerCamera);
	// And store our world position BEFORE collision
	GetCameraPosition().SetWorldPosition(preFinalPos);
	// Update crosshair visibility
	UpdateCrosshair(player, playerCamera);
}