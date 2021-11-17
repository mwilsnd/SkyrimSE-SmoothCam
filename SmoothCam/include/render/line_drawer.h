#pragma once
#include "render/vertex_buffer.h"
#include "render/shader.h"

namespace Render {
	typedef struct Point {
		glm::vec4 pos;
		glm::vec4 col;

		Point(glm::vec3 position, glm::vec4 color) : col(color) {
			pos = { position.x, position.y, position.z, 1.0f };
		}
	} Point;

	typedef struct Line {
		Point start;
		Point end;
		Line(Point&& start, Point&& end) : start(start), end(end) {};
	} Line;

	using LineList = eastl::vector<Line>;

	// Number of points we can submit in a single draw call
	constexpr size_t LineDrawPointBatchSize = 256;
	// Number of buffers to use
	constexpr size_t NumBuffers = 2;

	class LineDrawer {
		public:
			explicit LineDrawer(D3DContext& ctx);
			~LineDrawer();
			LineDrawer(const LineDrawer&) = delete;
			LineDrawer(LineDrawer&&) noexcept = delete;
			LineDrawer& operator=(const LineDrawer&) = delete;
			LineDrawer& operator=(LineDrawer&&) noexcept = delete;

			// Submit a list of lines for drawing
			void Submit(const LineList& lines) noexcept;

		protected:
			eastl::shared_ptr<Render::Shader> vs;
			eastl::shared_ptr<Render::Shader> ps;

		private:
			eastl::array<eastl::unique_ptr<Render::VertexBuffer>, NumBuffers> vbo;

			void CreateObjects(D3DContext& ctx);
			void DrawBatch(uint32_t bufferIndex, LineList::const_iterator& begin, LineList::const_iterator& end);
	};
}