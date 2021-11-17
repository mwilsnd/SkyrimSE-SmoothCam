#include "crosshair/dot.h"
#include "render/models/dot.h"
#include "render/shaders/vertex_color_world.h"
#include "render/shader_cache.h"
#include "camera.h"

void Crosshair::Dot::Create3D(Render::D3DContext& ctx, eastl::shared_ptr<Render::CBuffer>& perObjectBuf) noexcept {
	Render::Model::Model mdl;
	if (!Render::Model::Load(dotMesh, mdl))
		FatalError(L"SmoothCam: Failed to load 3D asset");

	// Per-object CBuffer for passing world transforms
	perObjectBuffer = perObjectBuf;

	// Vertex and fragment programs
	Render::ShaderCreateInfo vsCreateInfo(Render::Shaders::VertexColorWorldVS, Render::PipelineStage::Vertex);
	Render::ShaderCreateInfo psCreateInfo(Render::Shaders::VertexColorWorldPS, Render::PipelineStage::Fragment);

	// Upload mesh 0 to the GPU
	Render::MeshCreateInfo meshInfo;
	meshInfo.mesh = &mdl.meshes[0];
	meshInfo.vs = Render::ShaderCache::Get().Load(vsCreateInfo, ctx);
	meshInfo.ps = Render::ShaderCache::Get().Load(psCreateInfo, ctx);

	// Assert our shaders are good
	if (!meshInfo.vs->IsValid() || !meshInfo.ps->IsValid()) {
		FatalError(L"SmoothCam: A shader failed to compile.");
		return;
	}

	meshDrawer = eastl::make_unique<Render::MeshDrawer>(meshInfo, perObjectBuffer, ctx);

	const glm::vec3 ourSize{ 10.0f, 10.0f, 1.0f };
	SetScale(ourSize);
}

void Crosshair::Dot::Render(Render::D3DContext& ctx, bool allowDepthTesting) noexcept {
	// Compute our transform
	UpdateTransform();

	// Set raster state with some depth bias and submit using our transform
	Render::SetRasterState(
		ctx, D3D11_FILL_MODE::D3D11_FILL_SOLID, D3D11_CULL_MODE::D3D11_CULL_NONE,
		true, -25000
	);
	
	// Depth testing
	if (allowDepthTesting) {
		Render::SetDepthState(ctx, true, true, D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL);
	} else {
		Render::SetDepthState(ctx, false, false, D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER);
	}

	meshDrawer->Submit(transform);
}