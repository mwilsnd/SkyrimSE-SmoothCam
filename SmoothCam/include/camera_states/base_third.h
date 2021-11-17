#pragma once
#include "game_state.h"

#ifdef WITH_D2D
namespace Render {
	class StateOverlay;
}
#endif

namespace Camera {
	class Thirdperson;
	enum class CameraState : uint8_t;
	enum class CameraActionState : uint8_t;

	namespace State {
		/* The base camera state - exposes higher level methods for operating on the camera */
		class BaseThird {
			public:
				explicit BaseThird(Thirdperson* camera) noexcept;
				BaseThird(const BaseThird&) = delete;
				BaseThird(BaseThird&&) noexcept = delete;
				BaseThird& operator=(const BaseThird&) = delete;
				BaseThird& operator=(BaseThird&&) noexcept = delete;

			public:
				virtual ~BaseThird();
				virtual void OnBegin(RE::PlayerCharacter* player, RE::Actor* cameraRef, RE::PlayerCamera* camera,
					BaseThird* fromState) noexcept = 0;
				// If you return true from this callback, you will maintain control of the state and continue to receive update calls
				// until such time you return false - OnEnd will be called each frame until a false return value is seen.
				virtual bool OnEnd(RE::PlayerCharacter* player, RE::Actor* cameraRef, RE::PlayerCamera* camera,
					BaseThird* nextState, bool forced = false) noexcept = 0;
				virtual void Update(RE::PlayerCharacter* player, RE::Actor* cameraRef, RE::PlayerCamera* camera)
					noexcept;

			public:
				// Returns the current camera state
				GameState::CameraState GetCameraState() const noexcept;
				// Returns the current camera action state
				CameraActionState GetCameraActionState() const noexcept;

				// Get the current frustum
				const RE::NiFrustum& GetFrustum() const noexcept;

				// Returns the local offsets to apply to the camera
				glm::vec3 GetCameraOffsetStatePosition() const noexcept;
				// Returns the world position to apply local offsets to
				glm::vec3 GetCameraWorldPosition(const RE::TESObjectREFR* ref) const;
				// Performs all camera offset math using the view rotation matrix and local offsets, returns a local position
				glm::vec3 GetTransformedCameraLocalPosition() const;
				// Get the last camera position
				mmath::Position& GetLastCameraPosition() const noexcept;
				// Get the current camera position
				mmath::Position& GetCameraPosition() const noexcept;
				// Sets the camera world position
				void SetCameraPosition(const glm::vec3& pos, const RE::Actor* player,
					const RE::PlayerCamera* playerCamera) noexcept;
				// Set the camera to the goal position and invalidate interp state
				void MoveToGoalPosition(const RE::PlayerCharacter* player, const RE::Actor* forRef,
					const RE::PlayerCamera* playerCamera) noexcept;
				// True if shoulder swap is active
				bool IsShoulderSwapped() const noexcept;

				// Get the camera rotation as defined by the base game logic
				mmath::Rotation GetBaseGameRotation(const RE::PlayerCamera* playerCamera) const noexcept;
				// Returns the euler rotation of the camera
				mmath::Rotation& GetCameraRotation() const noexcept;
				// Set the rotation of the camera
				void SetCameraRotation(mmath::Rotation& rot, RE::PlayerCamera* playerCamera) noexcept;

				// Find a node to use as the world position for following
				RE::NiAVObject* FindFollowBone(const RE::TESObjectREFR* ref, const eastl::string_view& filterBone = "") const noexcept;
				// Offset the gmae FOV by the given amount
				void SetFOVOffset(float fov, bool force = false) noexcept;
				// Return the current offset transition state, for mutation
				mmath::OffsetTransition& GetOffsetTransitionState() noexcept;
				// Return the current FOV transition state, for mutation
				mmath::FloatTransition& GetFOVTransitionState() noexcept;

				// Performs a ray cast and returns a new position based on the result
				glm::vec3 ComputeRaycast(const glm::vec3& rayStart, const glm::vec3& rayEnd);
				// Clamps the camera position based on offset clamp settings
				glm::vec3 ComputeOffsetClamping(
					const RE::TESObjectREFR* ref,
					const glm::vec3& cameraLocalOffset, const glm::vec3& cameraWorldTarget,
					const glm::vec3& cameraPosition) const;
				glm::vec3 ComputeOffsetClamping(
					const RE::TESObjectREFR* ref, const glm::vec3& cameraWorldTarget,
					const glm::vec3& cameraPosition) const;

				// Primary crosshair update method, enables crosshair, performs raycast, sets position
				void UpdateCrosshair(const RE::Actor* player, const RE::PlayerCamera* playerCamera) const;
				// Updates the 3D crosshair position, performing all logic internally
				void UpdateCrosshairPosition(const RE::Actor* player, const RE::PlayerCamera* playerCamera) const;

				// Interpolates the given position, stores last interpolated rotation
				glm::vec3 UpdateInterpolatedLocalPosition(const glm::vec3& rot);
				// Interpolates the given position, stores last interpolated position
				glm::vec3 UpdateInterpolatedWorldPosition(const RE::Actor* player, const glm::vec3& fromPos,
					const glm::vec3& pos, const float distance);

				// Returns true if the camera should perform separate local interpolation
				bool IsLocalInterpAllowed() const noexcept;
				// Applies a local space offset to the camera to force the interaction raycast to properly align with the crosshair
				void ApplyLocalSpaceGameOffsets(const RE::Actor* player, const RE::PlayerCamera* playerCamera) noexcept;
				// Returns true if the player is moving
				bool IsPlayerMoving(const RE::Actor* player) const noexcept;
				// Returns the user config
				const Config::UserConfig* const GetConfig() const noexcept;
				// Flag input for the given camera state as locked or unlocked
				void LockInputState(uint8_t stateID, bool locked) noexcept;

			protected:
				// Hand of internal state to the next state
				void StateHandOff(BaseThird* nextState) const noexcept;

			protected:
				Thirdperson* const camera = nullptr;

			private:
				// Smooth world position when moving from an interp state to a disabled one
				using InterpSmoother = mmath::FixedTransitionGoal<glm::vec3>;
				InterpSmoother interpSmoother;
				bool wasInterpLastFrame = false;

#ifdef WITH_D2D
				friend class Render::StateOverlay;
#endif
		};
	}
}