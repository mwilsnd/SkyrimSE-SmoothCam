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
				BaseThird(Thirdperson* camera) noexcept;
				BaseThird(const BaseThird&) = delete;
				BaseThird(BaseThird&&) noexcept = delete;
				BaseThird& operator=(const BaseThird&) = delete;
				BaseThird& operator=(BaseThird&&) noexcept = delete;

			public:
				virtual ~BaseThird();
				virtual void OnBegin(const PlayerCharacter* player, const Actor* cameraRef, const CorrectedPlayerCamera* camera,
					BaseThird* fromState) noexcept = 0;
				virtual void OnEnd(const PlayerCharacter* player, const Actor* cameraRef, const CorrectedPlayerCamera* camera,
					BaseThird* nextState) noexcept = 0;
				virtual void Update(PlayerCharacter* player, const Actor* cameraRef, const CorrectedPlayerCamera* camera)
					noexcept;

			protected:
				// Hand of internal state to the next state
				void StateHandOff(BaseThird* nextState) const noexcept;

				// Returns the current camera state
				GameState::CameraState GetCameraState() const noexcept;
				// Returns the current camera action state
				CameraActionState GetCameraActionState() const noexcept;
				// Get the last camera position
				mmath::Position& GetLastCameraPosition() const noexcept;
				// Get the current camera position
				mmath::Position& GetCameraPosition() const noexcept;
				// Get the current frustum
				const NiFrustum& GetFrustum() const noexcept;
				// Sets the camera world position
				void SetCameraPosition(const glm::vec3& pos, const PlayerCharacter* player,
					const CorrectedPlayerCamera* playerCamera) noexcept;
				// Performs a ray cast and returns a new position based on the result
				glm::vec3 ComputeRaycast(const glm::vec3& rayStart, const glm::vec3& rayEnd);
				// Clamps the camera position based on offset clamp settings
				glm::vec3 ComputeOffsetClamping(
					const TESObjectREFR* ref,
					const glm::vec3& cameraLocalOffset, const glm::vec3& cameraWorldTarget,
					const glm::vec3& cameraPosition) const;
				glm::vec3 ComputeOffsetClamping(
					const TESObjectREFR* ref, const glm::vec3& cameraWorldTarget,
					const glm::vec3& cameraPosition) const;

				// Primary crosshair update method, enables crosshair, performs raycast, sets position
				void UpdateCrosshair(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const;
				// Updates the 3D crosshair position, performing all logic internally
				void UpdateCrosshairPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const;

				// Returns the euler rotation of the camera
				mmath::Rotation& GetCameraRotation() const noexcept;
				// Returns the local offsets to apply to the camera
				glm::vec3 GetCameraOffsetStatePosition() const noexcept;
				// Returns the world position to apply local offsets to
				glm::vec3 GetCameraWorldPosition(const TESObjectREFR* ref, const CorrectedPlayerCamera* playerCamera) const;
				// Performs all camera offset math using the view rotation matrix and local offsets, returns a local position
				glm::vec3 GetTransformedCameraLocalPosition(const CorrectedPlayerCamera* camera) const;
				// Interpolates the given position, stores last interpolated rotation
				glm::vec3 UpdateInterpolatedLocalPosition(const PlayerCharacter* player, const glm::vec3& rot);
				// Interpolates the given position, stores last interpolated position
				glm::vec3 UpdateInterpolatedWorldPosition(const PlayerCharacter* player, const glm::vec3& fromPos,
					const glm::vec3& pos, const float distance);

				// Returns true if the camera should perform separate local interpolation
				bool IsLocalInterpAllowed() const noexcept;
				// Applies a local space offset to the camera to force the interaction raycast to properly align with the crosshair
				void ApplyLocalSpaceGameOffsets(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) noexcept;
				// Returns true if the player is moving
				bool IsPlayerMoving(const PlayerCharacter* player) const noexcept;
				// Returns the user config
				const Config::UserConfig* const GetConfig() const noexcept;

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