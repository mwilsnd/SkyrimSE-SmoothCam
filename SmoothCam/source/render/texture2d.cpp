#include "render/texture2d.h"

Render::Texture2D::Texture2D(Render::D3DContext& ctx, const Texture2DCreateInfo& info) noexcept {
	D3D11_TEXTURE2D_DESC desc;
	desc.Format = info.format;
	desc.ArraySize = info.arraySize;
	desc.MipLevels = info.mipLivels;
	desc.Usage = info.usage;
	desc.SampleDesc.Count = info.sampleCount;
	desc.SampleDesc.Quality = info.sampleQuality;
	desc.BindFlags = info.bindFlags;
	desc.CPUAccessFlags = info.cpuAccessFlags;
	desc.MiscFlags = info.miscFlags;
	desc.Height = info.height;
	desc.Width = info.width;

	auto code = ctx.device->CreateTexture2D(&desc, nullptr, texture.put());
	if (!SUCCEEDED(code)) {
		FatalError(L"SmoothCam: Failed to create 2D texture resource");
	}

	if (info.createSampler)
		CreateSamplerState(ctx, info);
}

Render::Texture2D::Texture2D(Render::D3DContext& ctx, winrt::com_ptr<ID3D11Texture2D> texture,
	const Texture2DCreateInfo& info) : texture(texture)
{
	CreateSamplerState(ctx, info);
}

Render::Texture2D::~Texture2D() {
	sampler = nullptr;
	texture = nullptr;
}

void Render::Texture2D::CreateSamplerState(Render::D3DContext& ctx, const Texture2DCreateInfo& info) noexcept {
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Filter = info.filterMode;
	desc.MaxAnisotropy = info.maxAnisotropy;
	desc.AddressU = info.u;
	desc.AddressV = info.v;
	desc.AddressW = info.w;
	desc.ComparisonFunc = info.comparisonFunc;
	desc.MaxLOD = info.maxLOD;
	if (!SUCCEEDED(ctx.device->CreateSamplerState(&desc, sampler.put()))) {
		FatalError(L"SmoothCam: Failed to create sampler state");
	}
	hasSampler = true;
}

void Render::Texture2D::Bind(Render::D3DContext& ctx, Render::PipelineStage stage, uint32_t location) noexcept {
	assert(hasSampler);
	auto s = sampler.get();
	switch (stage) {
		case PipelineStage::Vertex: {
			ctx.context->VSSetSamplers(location, 1, &s);
			break;
		}
		case PipelineStage::Fragment: {
			ctx.context->PSSetSamplers(location, 1, &s);
			break;
		}
	}
}

winrt::com_ptr<ID3D11Texture2D>& Render::Texture2D::GetResource() noexcept {
	return texture;
}