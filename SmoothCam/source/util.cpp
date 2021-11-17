#pragma once
#include "util.h"

#ifdef DEBUG
#include "render/line_drawer.h"
#include "detours.h"
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

	Detours::RegisterGameShutdownEvent([]() {
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