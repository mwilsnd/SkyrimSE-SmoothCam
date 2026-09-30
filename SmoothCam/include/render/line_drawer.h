#pragma once
#include "render/vertex_buffer.h"
#include "render/cbuffer.h"
#include "render/shader.h"

namespace Render {
	struct PolyPoint {
		glm::vec3 pos;
		glm::vec4 col;
		PolyPoint(glm::vec3 position, glm::vec4 color) : pos(position), col(color) {}
	};

	struct Polyline {
		float widthPx = 2.0f;
		std::vector<PolyPoint> points;
	};

	struct WideLineVertex {
		glm::vec4 clipPos;
		glm::vec4 col;
		glm::vec2 off;
	};

	struct WideLineParams {
		glm::vec2 viewportSize;
		float lineWidthPx;
		float pad0;
	};
	static_assert(sizeof(WideLineParams) % 16 == 0);

	class LineDrawer {
		public:
			explicit LineDrawer(D3DContext& ctx);
			~LineDrawer();
			LineDrawer(const LineDrawer&) = delete;
			LineDrawer(LineDrawer&&) noexcept = delete;
			LineDrawer& operator=(const LineDrawer&) = delete;
			LineDrawer& operator=(LineDrawer&&) noexcept = delete;

			// Submit a list of lines for drawing
			void Submit(const Polyline& line, const glm::mat4& matProjView) noexcept;

		protected:
			std::shared_ptr<Render::Shader> vs;
			std::shared_ptr<Render::Shader> ps;

		private:
			static constexpr size_t WideLineMaxVerts = 8192;
			struct JoinState {
				bool valid = false;
				glm::vec4 clipPos;
				glm::vec4 col;
				glm::vec2 offUnit;
			};
		
			uint32_t EmitCorner(WideLineVertex* buf, uint32_t vertexIndex, const JoinState& prev,
				const glm::vec2& newOffUnit) noexcept;

			D3DContext& ctx;
			std::array<std::unique_ptr<Render::VertexBuffer>, 2> vbo;
			std::shared_ptr<Render::CBuffer> cbuf;
			size_t bufferIndex = 0;

			void CreateObjects(D3DContext& ctx);
	};
}
