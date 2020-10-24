#pragma once
#include "render/d3d_context.h"
#include "render/texture2d.h"

namespace Render {
	class SRV;

	typedef struct RenderTargetCreateInfo {
		DXGI_FORMAT format;
		D3D11_RTV_DIMENSION dimensions = D3D11_RTV_DIMENSION_TEXTURE2D;
		D3D11_TEX2D_RTV texture2D;
		std::shared_ptr<Texture2D> texture;
	} RenderTargetCreateInfo;

	class RenderTarget {
		public:
			explicit RenderTarget(Render::D3DContext& ctx, const RenderTargetCreateInfo& info);
			~RenderTarget();
			RenderTarget(const RenderTarget&) = delete;
			RenderTarget(RenderTarget&&) noexcept = delete;
			RenderTarget& operator=(const RenderTarget&) = delete;
			RenderTarget& operator=(RenderTarget&&) noexcept = delete;

			// Push the render target for drawing
			void Push(Render::D3DContext& ctx) noexcept;
			// Clear the target
			void Clear(Render::D3DContext& ctx, glm::vec4& color) noexcept;
			// Get an SRV for reading the target in shaders
			std::unique_ptr<SRV>& GetColorSRV() noexcept;

		private:
			winrt::com_ptr<ID3D11RenderTargetView> rtv;
			std::shared_ptr<Texture2D> texture;
			std::unique_ptr<SRV> srvColor;
	};
}