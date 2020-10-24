#pragma once
#include <d2d1_2.h>
#include <winrt/base.h>
#include "render/d3d_context.h"
#include "render/dwrite.h"
#pragma comment(lib, "D2d1.lib")

namespace Render {
	class DWrite;
	class RenderTarget;
	class Texture2D;
	class SRV;
	class VertexBuffer;
	class Shader;

	typedef struct ColorBrushKey {
		glm::vec4 color;

		size_t Hash() const {
			return std::hash<float>()(color.x) ^ std::hash<float>()(color.y) ^
				std::hash<float>()(color.z) ^ std::hash<float>()(color.w);
		}

		bool operator==(const ColorBrushKey& other) const {
			return color == other.color;
		}
	} ColorBrushKey;

	struct ColorBrushHasher {
		size_t operator()(const ColorBrushKey& key) const {
			return key.Hash();
		}
	};

	struct ColorBrushCompare {
		size_t operator()(const ColorBrushKey& k1, const ColorBrushKey& k2) const {
			return k1 == k2;
		}
	};

	class D2D {
		public:
			D2D(D3DContext& ctx);
			~D2D();

			// Start rendering a frame
			void BeginFrame() noexcept;
			// End rendering a frame
			void EndFrame() noexcept;
			// Flush the pipeline and spin until the device is done
			void FlushAndSpin() noexcept;
			// Copy the D2D render target to the game backbuffer
			void WriteToBackbuffer(D3DContext& ctx) noexcept;

			// Get the direct write instance
			std::unique_ptr<DWrite>& GetDWrite() noexcept;
			// Get a color brush for use with D2D/DWrite
			winrt::com_ptr<ID2D1SolidColorBrush> GetColorBrush(const glm::vec4& color) noexcept;
			// Draw a line
			void DrawLine(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color, float thickness = 1.0f) noexcept;

		private:
			void CreateRenderTarget(D3DContext& ctx, D3DContext& renderingCtx);

			D3DContext d2dContext;

			winrt::com_ptr<ID2D1Factory2> factory;
			winrt::com_ptr<IDXGIDevice> dxgiDevice;
			winrt::com_ptr<ID2D1Device1> device;
			winrt::com_ptr<ID2D1DeviceContext1> context;
			winrt::com_ptr<ID2D1Bitmap1> bitmap;

			std::unique_ptr<DWrite> dwrite;
			std::shared_ptr<Texture2D> colorBuffer;
			std::shared_ptr<Texture2D> sharedColorBuffer;
			winrt::com_ptr<IDXGISurface> dxgiBackBuffer;
			
			std::unique_ptr<SRV> colorSRV;
			std::unique_ptr<VertexBuffer> vboFullscreen;
			std::shared_ptr<Shader> fullScreenVS;
			std::shared_ptr<Shader> fullScreenPS;

			winrt::com_ptr<ID3D11Query> workQuery;

			std::unordered_map<
				ColorBrushKey,
				winrt::com_ptr<ID2D1SolidColorBrush>,
				ColorBrushHasher,
				ColorBrushCompare
			> colorBrushes;

			friend class DWrite;
	};
}