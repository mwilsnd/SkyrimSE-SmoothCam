#include "render/srv.h"
#include "render/texture2d.h"

Render::SRV::SRV(Render::D3DContext& ctx, const SRVCreateInfo& info) :
	texture(info.texture)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = info.format;
	desc.ViewDimension = info.dimensions;
	desc.Texture2D = info.texture2D;

	if (!SUCCEEDED(ctx.device->CreateShaderResourceView(texture->texture.get(), &desc, srv.put()))) {
		FatalError(L"SmoothCam: Failed to create shader resource view");
	}
}

Render::SRV::~SRV() {
	srv = nullptr;
	texture.reset();
}

void Render::SRV::Bind(Render::D3DContext& ctx, PipelineStage stage, uint32_t location) noexcept {
	auto resource = srv.get();
	switch (stage) {
		case PipelineStage::Vertex: {
			ctx.context->VSSetShaderResources(location, 1, &resource);
			break;
		}
		case PipelineStage::Fragment: {
			ctx.context->PSSetShaderResources(location, 1, &resource);
			break;
		}
	}
	
}