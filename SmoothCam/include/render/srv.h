#pragma once
#include "render/d3d_context.h"
#include "render/shader.h"

namespace Render {
	class Texture2D;

	typedef struct SRVCreateInfo {
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		D3D11_SRV_DIMENSION dimensions = D3D11_SRV_DIMENSION::D3D10_1_SRV_DIMENSION_TEXTURE2D;
		D3D11_TEX2D_SRV texture2D = {};
		std::shared_ptr<Texture2D> texture;
	} SRVCreateInfo;

	class SRV {
		public:
			explicit SRV(Render::D3DContext& ctx, const SRVCreateInfo& info);
			~SRV();
			SRV(const SRV&) = delete;
			SRV(SRV&&) noexcept = delete;
			SRV& operator=(const SRV&) = delete;
			SRV& operator=(SRV&&) noexcept = delete;

			// Bind the srv to a pipeline stage at the given location
			void Bind(Render::D3DContext& ctx, PipelineStage stage, uint32_t location) noexcept;

		private:
			winrt::com_ptr<ID3D11ShaderResourceView> srv;
			std::shared_ptr<Texture2D> texture;
	};
}