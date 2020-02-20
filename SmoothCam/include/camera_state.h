#pragma once
#include "pch.h"

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
				Camera::CameraState GetCameraState() const noexcept;
				Camera::CameraActionState GetCameraActionState() const noexcept;

				glm::vec3 GetLastCameraPosition() const noexcept;
				void SetCameraPosition(const glm::vec3& pos) noexcept;
				void UpdateCameraPosition(const glm::vec3& rayStart, const glm::vec3& rayEnd);

				void UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const;
				void SetCrosshairPosition(const glm::vec2& pos) const;
				void SetCrosshairEnabled(bool enabled) const;

				glm::vec2 GetCameraRotation(const CorrectedPlayerCamera* playerCamera) const noexcept;
				glm::vec3 GetCameraLocalPosition(PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const noexcept;
				glm::vec3 GetCameraWorldPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* playerCamera) const;
				glm::vec3 GetInterpolatedPosition(PlayerCharacter* player, const glm::vec3& pos, const float distance) const;

				float GetFrameDelta() const noexcept;

				bool IsPlayerMoving(const PlayerCharacter* player) const noexcept;
				bool IsWeaponDrawn(const PlayerCharacter* player) const noexcept;
				bool IsMeleeWeaponDrawn(PlayerCharacter* player) const noexcept;

				const Config::UserConfig* const GetConfig() const noexcept;

			protected:
				Camera::SmoothCamera* const camera = nullptr;
		};
	}
}