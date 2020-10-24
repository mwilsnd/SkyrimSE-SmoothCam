#include "render/render_target.h"
#include "render/srv.h"

Render::RenderTarget::RenderTarget(Render::D3DContext& ctx, const RenderTargetCreateInfo& info) :
	texture(info.texture)
{
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Format = info.format;
	desc.ViewDimension = info.dimensions;
	desc.Texture2D = info.texture2D;

	if (!SUCCEEDED(ctx.device->CreateRenderTargetView(info.texture->texture.get(), &desc, rtv.put()))) {
		FatalError(L"SmoothCam: Failed to create render target view");
	}

	SRVCreateInfo srvInfo;
	srvInfo.dimensions = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvInfo.format = info.format;
	srvInfo.texture2D.MipLevels = 1;
	srvInfo.texture2D.MostDetailedMip = 0;
	srvInfo.texture = texture;
	srvColor = std::make_unique<Render::SRV>(ctx, srvInfo);
}

Render::RenderTarget::~RenderTarget() {
	srvColor.reset();
	rtv = nullptr;
	texture.reset();
}

void Render::RenderTarget::Push(Render::D3DContext& ctx) noexcept {
	auto rtvA = rtv.get();
	ctx.context->OMSetRenderTargets(1, &rtvA, Render::GetDepthStencilView().get());
}

void Render::RenderTarget::Clear(Render::D3DContext& ctx, glm::vec4& color) noexcept {
	float col[4] = {
		color.r,
		color.g,
		color.b,
		color.a
	};
	ctx.context->ClearRenderTargetView(rtv.get(), col);
}

std::unique_ptr<Render::SRV>& Render::RenderTarget::GetColorSRV() noexcept {
	return srvColor;
}