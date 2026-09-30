#include "firstperson.h"

Camera::Firstperson::Firstperson(Camera* baseCamera) : ICamera(baseCamera, CameraID::Firstperson) {}

Camera::Firstperson::~Firstperson() {}

void Camera::Firstperson::OnBegin(RE::PlayerCharacter*, RE::PlayerCamera*, ICamera*) noexcept {}

void Camera::Firstperson::OnEnd(RE::PlayerCharacter*, RE::PlayerCamera*, ICamera*) noexcept {}

bool Camera::Firstperson::OnPreGameUpdate(RE::PlayerCharacter*, RE::PlayerCamera*,
	RE::BSTSmartPointer<RE::TESCameraState>&)
{
	return false;
}

void Camera::Firstperson::OnUpdateCamera(RE::PlayerCharacter*, RE::PlayerCamera*,
	RE::BSTSmartPointer<RE::TESCameraState>&) {}

void Camera::Firstperson::Render(Render::D3DContext&) noexcept {}

void Camera::Firstperson::OnTogglePOV(RE::ButtonEvent*) noexcept {}

bool Camera::Firstperson::OnKeyPress(const RE::ButtonEvent*) noexcept {
	return false;
}

bool Camera::Firstperson::OnMenuOpenClose(MenuID, const RE::MenuOpenCloseEvent* const) noexcept {
	return false;
}

void Camera::Firstperson::OnCameraActionStateTransition(const RE::PlayerCharacter*, const CameraActionState,
	const CameraActionState) noexcept
{}

bool Camera::Firstperson::OnCameraStateTransition(RE::PlayerCharacter*, RE::PlayerCamera*,
	const GameState::CameraState, const GameState::CameraState) noexcept
{
	return false;
}
