#pragma once
#ifdef WITH_D2D
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

	enum class StrokeStyle : uint8_t {
		Solid,						// Solid
		RoundedSolid,				// Solid, Rounded ends
		RoundedSolidRounded,		// Solid, Rounded ends, rounded joins
		Dashed,						// Dashed
		RoundedDashed,				// Dashed, Rounded ends
		RoundedDashedRounded,		// Dashed, Rounded ends, rounded joins
		RoundedDashedRoundedSmooth, // Dashed, Rounded ends, rounded joins, rounded dashes
		STYLE_MAX,
	};

	typedef struct ColorBrushKey {
		glm::vec4 color;

		size_t Hash() const {
			return std::hash<glm::vec4>()(color);
		}

		bool operator==(const ColorBrushKey& other) const {
			return color.x == other.color.x &&
				color.y == other.color.y &&
				color.z == other.color.z &&
				color.w == other.color.w;
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
			explicit D2D(D3DContext& ctx);
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
			eastl::unique_ptr<DWrite>& GetDWrite() noexcept;
			// Get a color brush for use with D2D/DWrite
			winrt::com_ptr<ID2D1SolidColorBrush> GetColorBrush(const glm::vec4& color) noexcept;
			// Draw a line
			void DrawLine(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color, float thickness = 1.0f) noexcept;
			// Draw an ellipse
			void DrawEllipse(const glm::vec2& center, const glm::vec2& extents, const glm::vec4& color,
				StrokeStyle style = StrokeStyle::Solid, const float stroke = 1.0f) noexcept;

		private:
			void CreateRenderTarget(D3DContext& ctx, D3DContext& renderingCtx);

			// Get a stroke style
			template<uint32_t count>
			winrt::com_ptr<ID2D1StrokeStyle1> GetStrokeStyle(
				const float* dashes,
				D2D1_CAP_STYLE endStyle = D2D1_CAP_STYLE::D2D1_CAP_STYLE_FLAT,
				D2D1_CAP_STYLE dashEndStyle = D2D1_CAP_STYLE::D2D1_CAP_STYLE_FLAT,
				D2D1_LINE_JOIN segmentStyle = D2D1_LINE_JOIN::D2D1_LINE_JOIN_MITER,
				D2D1_DASH_STYLE dashStyle = D2D1_DASH_STYLE::D2D1_DASH_STYLE_SOLID) noexcept
			{
				D2D1_STROKE_STYLE_PROPERTIES1 props;
				props.dashCap = dashEndStyle;
				props.dashOffset = 0.0f;
				props.dashStyle = dashStyle;
				props.endCap = endStyle;
				props.lineJoin = segmentStyle;
				props.miterLimit = 1.0f;
				props.startCap = endStyle;
				props.transformType = D2D1_STROKE_TRANSFORM_TYPE::D2D1_STROKE_TRANSFORM_TYPE_NORMAL;
				winrt::com_ptr<ID2D1StrokeStyle1> style;

				if constexpr (count == 0) {
					if (!SUCCEEDED(factory->CreateStrokeStyle(&props, nullptr, 0, style.put())))
						FatalError(L"SmoothCam: Failed to create brush stroke style");
				} else {
					if (!SUCCEEDED(factory->CreateStrokeStyle(&props, dashes, count, style.put())))
						FatalError(L"SmoothCam: Failed to create brush stroke style");
				}

				return style;
			}
			
			D3DContext d2dContext;

			winrt::com_ptr<ID2D1Factory2> factory;
			winrt::com_ptr<IDXGIDevice> dxgiDevice;
			winrt::com_ptr<ID2D1Device1> device;
			winrt::com_ptr<ID2D1DeviceContext1> context;
			winrt::com_ptr<ID2D1Bitmap1> bitmap;

			eastl::unique_ptr<DWrite> dwrite;
			eastl::shared_ptr<Texture2D> colorBuffer;
			eastl::shared_ptr<Texture2D> sharedColorBuffer;
			winrt::com_ptr<IDXGISurface> dxgiBackBuffer;
			
			eastl::unique_ptr<SRV> colorSRV;
			eastl::unique_ptr<VertexBuffer> vboFullscreen;
			eastl::shared_ptr<Shader> fullScreenVS;
			eastl::shared_ptr<Shader> fullScreenPS;

			winrt::com_ptr<ID3D11Query> workQuery;

			eastl::unordered_map<
				ColorBrushKey,
				winrt::com_ptr<ID2D1SolidColorBrush>,
				ColorBrushHasher,
				ColorBrushCompare
			> colorBrushes;

			winrt::com_ptr<ID2D1StrokeStyle1> strokeStyles[static_cast<size_t>(StrokeStyle::STYLE_MAX)];

			friend class DWrite;
	};
}
#endif