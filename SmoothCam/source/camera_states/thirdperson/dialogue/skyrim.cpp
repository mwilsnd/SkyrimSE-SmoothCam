#include "thirdperson.h"
#include "camera_states/thirdperson/dialogue/skyrim.h"

using namespace Camera;

State::SkyrimDialogue::SkyrimDialogue(ThirdpersonDialogueState* parentState) noexcept
	: IThirdPersonDialogue(parentState)
{}

State::SkyrimDialogue::~SkyrimDialogue() noexcept {}

void State::SkyrimDialogue::OnStart(RE::PlayerCharacter*, RE::Actor*,
	RE::PlayerCamera*) noexcept
{}

void State::SkyrimDialogue::OnEnd(RE::PlayerCharacter*, RE::Actor*,
	RE::PlayerCamera*) noexcept
{}

// @Note: For 'Skyrim' style, we just run the thirdperson camera like normal - it works really well
void State::SkyrimDialogue::Update(RE::PlayerCharacter* player, RE::Actor* cameraRef,
	RE::PlayerCamera* playerCamera) noexcept
{
	// Get our computed local-space xyz offset.
	const auto cameraLocal = parentState->GetCameraOffsetStatePosition();
	// Get the base world position for the camera which we will offset with the local-space values.
	const auto worldTarget = parentState->GetCameraWorldPosition(cameraRef);
	// Transform the camera offsets based on the computed view matrix
	const auto transformedLocalPos = parentState->GetTransformedCameraLocalPosition();

	glm::vec3 preFinalPos{};
	if (parentState->IsLocalInterpAllowed()) {
		// Handle separate local-space interpolation
		const auto loc = parentState->UpdateInterpolatedLocalPosition(transformedLocalPos);

		const auto& last = parentState->GetLastCameraPosition();
		// Last offset position from ref
		const auto lastWorld = last.world - last.local;

		// And the world target
		const auto lerpedWorldPos = parentState->UpdateInterpolatedWorldPosition(
			player, lastWorld, worldTarget,
			glm::length(lastWorld - worldTarget)
		);

		// Compute offset clamping if enabled
		preFinalPos = parentState->ComputeOffsetClamping(cameraRef, worldTarget, lerpedWorldPos) + loc;

	} else {
		// Combined case

		// Add the final local space transformation to the player postion
		const auto targetWorldPos = worldTarget + transformedLocalPos;

		// Now lerp it based on camera distance to player position
		const auto lerpedWorldPos = parentState->UpdateInterpolatedWorldPosition(
			player, parentState->GetLastCameraPosition().world, targetWorldPos,
			glm::length(targetWorldPos - worldTarget)
		);

		// Compute offset clamping if enabled
		preFinalPos = parentState->ComputeOffsetClamping(cameraRef, transformedLocalPos, worldTarget, lerpedWorldPos);
	}

	glm::vec3 f, s, u, coef;
	mmath::DecomposeToBasis(
		preFinalPos,
		{ parentState->GetCameraRotation().euler.x, 0.0f, parentState->GetCameraRotation().euler.y },
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
	if (parentState->GetConfig()->pitchZoomAfterInterp) {
		coef.y -= parentState->GetThirdpersonCamera()->GetPitchZoom();
		const auto pitchZoomedOffset = (f * coef.x) + (s * coef.y) + (u * coef.z);
		finalPos = parentState->ComputeRaycast(rayXOrigin, pitchZoomedOffset);
	} else {
		finalPos = parentState->ComputeRaycast(rayXOrigin, preFinalPos);
	}

	// Set the position to the ray hit position
	// This sets the rendering position of the camera, and applies game offsets
	parentState->SetCameraPosition(finalPos, player, playerCamera);
	// And store our world position BEFORE collision
	parentState->GetCameraPosition().SetWorldPosition(preFinalPos);
	// Update crosshair visibility
	parentState->UpdateCrosshair(player, playerCamera);
}

void State::SkyrimDialogue::ExitPoll(RE::PlayerCharacter*, RE::Actor*,
	RE::PlayerCamera*) noexcept
{}

bool State::SkyrimDialogue::CanExit(RE::PlayerCharacter*, RE::Actor*,
	RE::PlayerCamera*) noexcept
{
	return true;
}