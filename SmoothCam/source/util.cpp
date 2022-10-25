#pragma once
#include "util.h"
#include <xinput.h>

#ifdef DEBUG
#include "render/line_drawer.h"
#include "hooks.h"
#include "camera.h"
#include "thirdperson.h"

extern eastl::unique_ptr<Camera::Camera> g_theCamera;
extern Offsets* g_Offsets;

static std::mutex renderLock;
static eastl::unique_ptr<Render::LineDrawer> lineDrawer;
static Render::LineList lines;

struct VSMatricesCBuffer {
	glm::mat4 matProjView = glm::identity<glm::mat4>();
	glm::vec4 tint = { 1.0f, 1.0f, 1.0f, 1.0f };
	float curTime = 0.0f;
	float pad[3] = { 0.0f, 0.0f, 0.0f };
};
static_assert(sizeof(VSMatricesCBuffer) % 16 == 0);

static VSMatricesCBuffer cbufPerFrameStaging = {};
static eastl::shared_ptr<Render::CBuffer> cbufPerFrame;

void Util::InitializeDebugDrawing(Render::D3DContext& context) noexcept {
	lineDrawer = eastl::make_unique<Render::LineDrawer>(context);

	Render::CBufferCreateInfo perFrane;
	perFrane.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	perFrane.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	perFrane.size = sizeof(decltype(cbufPerFrameStaging));
	perFrane.initialData = &cbufPerFrameStaging;
	cbufPerFrame = eastl::make_shared<Render::CBuffer>(perFrane, context);

	Render::OnPresent([](Render::D3DContext& ctx) {
		std::lock_guard<std::mutex> lock(renderLock);
		const auto playerCamera = RE::PlayerCamera::GetSingleton();
		if (lines.size() == 0 ||
			(playerCamera->currentState->id != RE::CameraState::kThirdPerson &&
			playerCamera->currentState->id != RE::CameraState::kMount &&
			playerCamera->currentState->id != RE::CameraState::kDragon))
		{
			lines.clear();
			return;
		}

		const auto& frustum = g_theCamera->GetFrustum();
		const auto& pos = g_theCamera->GetThirdpersonCamera()->GetPosition();
		const auto& rot = g_theCamera->GetThirdpersonCamera()->GetCameraRotation();

		const auto matProj = Render::GetProjectionMatrix(frustum);
		const auto matView = Render::BuildViewMatrix(pos.world, rot.euler);
		cbufPerFrameStaging.matProjView = matProj * matView;
		cbufPerFrame->Update(
			&cbufPerFrameStaging, 0,
			sizeof(decltype(cbufPerFrameStaging)), ctx
		);

		cbufPerFrame->Bind(Render::PipelineStage::Vertex, 1, ctx);
		cbufPerFrame->Bind(Render::PipelineStage::Fragment, 1, ctx);

		// Setup depth and blending
		Render::SetDepthState(ctx, false, false, D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL);
		Render::SetBlendState(
			ctx, true,
			D3D11_BLEND_OP::D3D11_BLEND_OP_ADD, D3D11_BLEND_OP::D3D11_BLEND_OP_ADD,
			D3D11_BLEND::D3D11_BLEND_SRC_ALPHA, D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND::D3D11_BLEND_ONE, D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA
		);

		lineDrawer->Submit(lines);
		lines.clear();
	});

	Hooks::RegisterGameShutdownEvent([]() {
		lineDrawer.reset();
		cbufPerFrame.reset();
	});
}

void Util::DrawCross3D(const glm::vec3& pos, float extents, const glm::vec4& color) noexcept {
	std::lock_guard<std::mutex> lock(renderLock);
	eastl::array<glm::vec3, 6> points = {
		pos + glm::vec3{extents, 0.0f, 0.0f},
		pos + glm::vec3{-extents, 0.0f, 0.0f},

		pos + glm::vec3{0.0f, extents, 0.0f},
		pos + glm::vec3{0.0f, -extents, 0.0f},

		pos + glm::vec3{0.0f, 0.0f, extents},
		pos + glm::vec3{0.0f, 0.0f, -extents}
	};

	for (auto& point : points) {
		lines.emplace_back(
			Render::Point(Render::ToRenderScale(pos), color),
			Render::Point(Render::ToRenderScale(point), color)
		);
	}
}

void Util::DrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color) noexcept {
	std::lock_guard<std::mutex> lock(renderLock);
	lines.emplace_back(
		Render::Point(Render::ToRenderScale(from), color),
		Render::Point(Render::ToRenderScale(to), color)
	);
}
#endif

uint32_t Util::GamepadMaskToKeycode(uint32_t keyMask)
{
	switch (keyMask)
	{
		case XINPUT_GAMEPAD_DPAD_UP:		return kGamepadButtonOffset_DPAD_UP;
		case XINPUT_GAMEPAD_DPAD_DOWN:		return kGamepadButtonOffset_DPAD_DOWN;
		case XINPUT_GAMEPAD_DPAD_LEFT:		return kGamepadButtonOffset_DPAD_LEFT;
		case XINPUT_GAMEPAD_DPAD_RIGHT:		return kGamepadButtonOffset_DPAD_RIGHT;
		case XINPUT_GAMEPAD_START:			return kGamepadButtonOffset_START;
		case XINPUT_GAMEPAD_BACK:			return kGamepadButtonOffset_BACK;
		case XINPUT_GAMEPAD_LEFT_THUMB:		return kGamepadButtonOffset_LEFT_THUMB;
		case XINPUT_GAMEPAD_RIGHT_THUMB:	return kGamepadButtonOffset_RIGHT_THUMB;
		case XINPUT_GAMEPAD_LEFT_SHOULDER:	return kGamepadButtonOffset_LEFT_SHOULDER;
		case XINPUT_GAMEPAD_RIGHT_SHOULDER: return kGamepadButtonOffset_RIGHT_SHOULDER;
		case XINPUT_GAMEPAD_A:				return kGamepadButtonOffset_A;
		case XINPUT_GAMEPAD_B:				return kGamepadButtonOffset_B;
		case XINPUT_GAMEPAD_X:				return kGamepadButtonOffset_X;
		case XINPUT_GAMEPAD_Y:				return kGamepadButtonOffset_Y;
		case 0x9:							return kGamepadButtonOffset_LT;
		case 0xA:							return kGamepadButtonOffset_RT;
		default:							return kMaxMacros; // Invalid
	}
}

uint32_t Util::GamepadKeycodeToMask(uint32_t keyCode)
{
	switch (keyCode)
	{
		case kGamepadButtonOffset_DPAD_UP:			return XINPUT_GAMEPAD_DPAD_UP;
		case kGamepadButtonOffset_DPAD_DOWN:		return XINPUT_GAMEPAD_DPAD_DOWN;
		case kGamepadButtonOffset_DPAD_LEFT:		return XINPUT_GAMEPAD_DPAD_LEFT;
		case kGamepadButtonOffset_DPAD_RIGHT:		return XINPUT_GAMEPAD_DPAD_RIGHT;
		case kGamepadButtonOffset_START:			return XINPUT_GAMEPAD_START;
		case kGamepadButtonOffset_BACK:				return XINPUT_GAMEPAD_BACK;
		case kGamepadButtonOffset_LEFT_THUMB:		return XINPUT_GAMEPAD_LEFT_THUMB;
		case kGamepadButtonOffset_RIGHT_THUMB:		return XINPUT_GAMEPAD_RIGHT_THUMB;
		case kGamepadButtonOffset_LEFT_SHOULDER:	return XINPUT_GAMEPAD_LEFT_SHOULDER;
		case kGamepadButtonOffset_RIGHT_SHOULDER:	return XINPUT_GAMEPAD_RIGHT_SHOULDER;
		case kGamepadButtonOffset_A:				return XINPUT_GAMEPAD_A;
		case kGamepadButtonOffset_B:				return XINPUT_GAMEPAD_B;
		case kGamepadButtonOffset_X:				return XINPUT_GAMEPAD_X;
		case kGamepadButtonOffset_Y:				return XINPUT_GAMEPAD_Y;
		case kGamepadButtonOffset_LT:				return 0x9;
		case kGamepadButtonOffset_RT:				return 0xA;
		default:									return 0xFF; // Invalid
	}
}