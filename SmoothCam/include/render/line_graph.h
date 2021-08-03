#pragma once
#ifdef WITH_D2D
#include "render/d3d_context.h"
#include "render/gradbox.h"

namespace Render {
	class LineGraph : public GradBox {
		public:
			explicit LineGraph(uint8_t numPlots, uint32_t maxPoints, uint32_t width, uint32_t height, D3DContext& ctx);
			~LineGraph();
			LineGraph(const LineGraph&) = delete;
			LineGraph(LineGraph&&) noexcept = delete;
			LineGraph& operator=(const LineGraph&) = delete;
			LineGraph& operator=(LineGraph&&) noexcept = delete;

			// Set the position of the graph
			void SetPosition(uint32_t x, uint32_t y) noexcept;
			// Set the size of the graph
			void SetSize(uint32_t w, uint32_t h) noexcept;
			// Set the line color of the given plot id
			void SetLineColor(uint8_t plotID, const glm::vec4& color) noexcept;
			// Set the line thickness for the plots
			void SetLineThickness(float amount) noexcept;

			// Set the name of the chart
			void SetName(const eastl::wstring& n);

			// Add a point to the given plot id
			void AddPoint(uint8_t plotID, float value) noexcept;
			// Draw the chart
			void Draw(D3DContext& ctx) noexcept;
			
		private:
			using PlotList = eastl::vector<float>;
			eastl::vector<PlotList> plots;
			eastl::vector<glm::vec2> plotRanges;
			eastl::vector<glm::vec4> plotColors;

			eastl::wstring name;

			uint8_t numPlots;
			uint32_t maxPoints;
			uint32_t width;
			uint32_t height;
			uint32_t xPos = 0;
			uint32_t yPos = 0;
			float lineThickness = 2.0f;

			typedef struct PlotMetrics {
				float min;
				float max;
				float avg;

				PlotMetrics(float min, float max, float avg) :
					min(min), max(max), avg(avg)
				{}
			} PlotMetrics;
	};
}
#endif