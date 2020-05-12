#pragma once
#include "game_state.h"

namespace Camera {
	class SmoothCamera;
	enum class CameraState;
	enum class CameraActionState;

	namespace State {
		/* The base camera state - exposes higher level methods for operating on the camera */
		class BaseCameraState {
			public:
				BaseCameraState(Camera::SmoothCamera* camera) noexcept;
				BaseCameraState(const BaseCameraState&) = delete;
				BaseCameraState(BaseCameraState&&) noexcept = delete;
				BaseCameraState& operator=(const BaseCameraState&) = delete;
				BaseCameraState& operator=(BaseCameraState&&) noexcept = delete;

			public:
				virtual ~BaseCameraState();
				virtual void OnBegin(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) = 0;
				virtual void OnEnd(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) = 0;
				virtual void Update(PlayerCharacter* player, const CorrectedPlayerCamera* camera) = 0;

			protected:
				// Returns the current camera state
				GameState::CameraState GetCameraState() const noexcept;
				// Returns the current camera action state
				Camera::CameraActionState GetCameraActionState() const noexcept;

				// Returns the position of the camera during the last frame
				glm::vec3 GetLastCameraPosition() const noexcept;
				// Sets the camera position
				void SetCameraPosition(const glm::vec3& pos, const CorrectedPlayerCamera* playerCamera) noexcept;
				// Performs a ray cast and returns a new position based on the result
				glm::vec3 ComputeRaycast(const glm::vec3& rayStart, const glm::vec3& rayEnd);
				// Clamps the camera position based on offset clamp settings
				glm::vec3 ComputeOffsetClamping(
					PlayerCharacter* player, const CorrectedPlayerCamera* camera,
					const glm::vec3& cameraLocalOffset, const glm::vec3& cameraWorldTarget,
					const glm::vec3& cameraPosition) const;
				glm::vec3 ComputeOffsetClamping(
					PlayerCharacter* player, const glm::vec3& cameraWorldTarget,
					const glm::vec3& cameraPosition) const;

				// Updates the 3D crosshair position, performing all logic internally
				void UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const;
				// Directly sets the crosshair position
				void SetCrosshairPosition(const glm::vec2& pos) const;
				// Toggles visibility of the crosshair
				void SetCrosshairEnabled(bool enabled) const;

				// Returns a rotation matrix to use with rotating the camera
				glm::mat4 GetViewMatrix(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const noexcept;
				// Returns the euler rotation of the camera
				glm::vec2 GetCameraRotation(const CorrectedPlayerCamera* playerCamera) const noexcept;
				// Returns the local offsets to apply to the camera
				glm::vec3 GetCameraLocalPosition(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const noexcept;
				// Returns the world position to apply local offsets to
				glm::vec3 GetCameraWorldPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const;
				// Performs all camera offset math using the view rotation matrix and local offsets, returns a local position
				glm::vec3 GetTransformedCameraLocalPosition(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const;
				// Interpolates the given position, stores last interpolated rotation
				glm::vec3 UpdateInterpolatedLocalPosition(PlayerCharacter* player, const glm::vec3& rot);
				// Interpolates the given position, stores last interpolated position
				glm::vec3 UpdateInterpolatedWorldPosition(PlayerCharacter* player, const glm::vec3& pos, const float distance);

				void StoreLastLocalPosition(const glm::vec3& pos);
				void StoreLastWorldPosition(const glm::vec3& pos);
				glm::vec3 GetLastLocalPosition();
				glm::vec3 GetLastWorldPosition();

				// Returns true if the player is moving
				bool IsPlayerMoving(const PlayerCharacter* player) const noexcept;
				// Returns true if any kind of weapon is drawn
				bool IsWeaponDrawn(const PlayerCharacter* player) const noexcept;
				// Returns true if a melee weapon is drawn
				bool IsMeleeWeaponDrawn(PlayerCharacter* player) const noexcept;
				// Returns the user config
				const Config::UserConfig* const GetConfig() const noexcept;

			protected:
				Camera::SmoothCamera* const camera = nullptr;
		};
	}
}