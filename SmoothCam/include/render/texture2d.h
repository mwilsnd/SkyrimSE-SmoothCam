#pragma once
#include "render/d3d_context.h"
#include "render/shader.h"

namespace Render {
	class RenderTarget;
	class SRV;

	typedef struct Texture2DCreateInfo {
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mipLivels = 1;
		uint32_t arraySize = 1;
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		uint32_t bindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		uint32_t cpuAccessFlags = 0;
		uint32_t miscFlags = 0;
		uint32_t sampleCount = 1;
		uint32_t sampleQuality = 0;
		bool createSampler = false;

		D3D11_TEXTURE_ADDRESS_MODE u = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
		D3D11_TEXTURE_ADDRESS_MODE v = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
		D3D11_TEXTURE_ADDRESS_MODE w = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
		D3D11_FILTER filterMode = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
		D3D11_COMPARISON_FUNC comparisonFunc = D3D11_COMPARISON_ALWAYS;
		uint32_t maxAnisotropy = 0;
		float maxLOD = 0.0f;
	} Texture2DCreateInfo;
	
	class Texture2D {
		public:
			Texture2D(Render::D3DContext& ctx, const Texture2DCreateInfo& info) noexcept;
			Texture2D(Render::D3DContext& ctx, winrt::com_ptr<ID3D11Texture2D> texture,
				const Texture2DCreateInfo& info) noexcept;
			~Texture2D() noexcept;
			Texture2D(const Texture2D&) = delete;
			Texture2D(Texture2D&&) noexcept = delete;
			Texture2D& operator=(const Texture2D&) = delete;
			Texture2D& operator=(Texture2D&&) noexcept = delete;

			// Bind the texture to a pipeline stage at the given location
			// Requires that the texture was created with a sampler state
			void Bind(Render::D3DContext& ctx, PipelineStage stage, uint32_t location) noexcept;
			// Get the underlying D3D resource
			winrt::com_ptr<ID3D11Texture2D>& GetResource() noexcept;

		private:
			void CreateSamplerState(Render::D3DContext& ctx, const Texture2DCreateInfo& info) noexcept;

			winrt::com_ptr<ID3D11Texture2D> texture;
			winrt::com_ptr<ID3D11SamplerState> sampler;
			bool hasSampler = false;

			friend class Render::RenderTarget;
			friend class Render::SRV;
	};
}