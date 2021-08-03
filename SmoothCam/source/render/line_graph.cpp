#ifdef WITH_D2D
#include "render/line_graph.h"
#include "render/vertex_buffer.h"
#include "render/d2d.h"
#include "render/dwrite.h"
#include "render/shader.h"

extern eastl::unique_ptr<Render::D2D> g_D2D;

Render::LineGraph::LineGraph(uint8_t numPlots, uint32_t maxPoints, uint32_t width, uint32_t height, D3DContext& ctx)
	: numPlots(numPlots), maxPoints(maxPoints), width(width), height(height), GradBox(ctx, width, height)
{
	plots.reserve(numPlots);
	plotRanges.reserve(numPlots);
	plotColors.reserve(numPlots);

	for (auto i = 0; i < numPlots; i++) {
		plots.emplace_back();
		plotRanges.emplace_back(0.0f, 0.0f);
		plotColors.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);
	}
}

Render::LineGraph::~LineGraph() {}

void Render::LineGraph::SetPosition(uint32_t x, uint32_t y) noexcept {
	xPos = x;
	yPos = y;
	SetBackgroundPosition({ x, y });
}

void Render::LineGraph::SetSize(uint32_t w, uint32_t h) noexcept {
	width = w;
	height = h;
	SetBackgroundSize({ w, h });
}

void Render::LineGraph::SetLineColor(uint8_t plotID, const glm::vec4& color) noexcept {
	plotColors[plotID] = color;
}

void Render::LineGraph::SetLineThickness(float amount) noexcept {
	lineThickness = amount;
}

void Render::LineGraph::SetName(const eastl::wstring& n) {
	name = n;
}

void Render::LineGraph::AddPoint(uint8_t plotID, float value) noexcept {
	auto& points = plots[plotID];

	if (points.size() >= maxPoints)
		points.erase(points.begin());
	points.push_back(value);

	const float minValue = *std::min_element(points.begin(), points.end());
	const float maxValue = *std::max_element(points.begin(), points.end());
	plotRanges[plotID] = { minValue, maxValue };
}

void Render::LineGraph::Draw(D3DContext& ctx) noexcept {
	// Draw background
	DrawBackground(ctx);

	// Draw line(s)
	eastl::vector<PlotMetrics> plotMetrics;
	const auto xAdd = static_cast<float>(width) / static_cast<float>(maxPoints);
	for (auto i = 0; i < plots.size(); i++) {
		const auto& points = plots[i];
		const auto& lineColor = plotColors[i];
		const auto& plotRange = plotRanges[i];
		float x = 0.0f;

		float minVal = points.size() == 0 ? 0.0f : eastl::numeric_limits<float>::max();
		float maxVal = points.size() == 0 ? 0.0f : eastl::numeric_limits<float>::min();
		float avg = 0.0f;

		eastl::vector<glm::vec2> plotLocations;
		uint32_t j = 0;
		for (const auto& value : points) {
			if (value > maxVal) maxVal = value;
			if (value < minVal) minVal = value;
			avg += value;

			const auto normalized = static_cast<float>(height) - (
				plotRange.x == plotRange.y ? 
				static_cast<float>(height) :
				mmath::Remap(value, plotRange.x, plotRange.y, 0.0f, static_cast<float>(height))
			);

			plotLocations.emplace_back(
				x + static_cast<float>(xPos),
				normalized + static_cast<float>(yPos)
			);

			if (j > 1) {
				g_D2D->DrawLine(
					plotLocations[j-1],
					plotLocations[j],
					lineColor,
					lineThickness
				);
			}

			j++;
			x += xAdd;
		}

		if (points.size() > 0) avg /= static_cast<float>(points.size());
		plotMetrics.emplace_back(minVal, maxVal, avg);
	}

	// Draw chart name
	const auto tpos = glm::vec2{
		static_cast<float>(xPos),
		static_cast<float>(yPos)
	};
	g_D2D->GetDWrite()->Write(
		name,
		ctx.windowSize.x, ctx.windowSize.y,
		tpos,
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	);

	// Draw text overlay(s)
	const auto nameSize = g_D2D->GetDWrite()->GetTextSize(name, ctx.windowSize.x, ctx.windowSize.y);
	float xoff = 0.0f;
	constexpr float xpad = 10.0f;

	for (auto i = 0; i < plotMetrics.size(); i++) {
		const auto& metrics = plotMetrics[i];
		const auto& lineColor = plotColors[i];
		float yoff = nameSize.y;
		float longestLine = 0.0f;

		eastl::wstring minStr = L"min: ";
		minStr.append(std::to_wstring(metrics.min).c_str());
		g_D2D->GetDWrite()->Write(
			minStr,
			ctx.windowSize.x, ctx.windowSize.y,
			tpos + glm::vec2{ xoff, yoff },
			lineColor
		);

		auto sz = g_D2D->GetDWrite()->GetTextSize(minStr, ctx.windowSize.x, ctx.windowSize.y);
		longestLine = eastl::max(sz.x, longestLine);
		eastl::wstring maxStr = L"max: ";
		maxStr.append(std::to_wstring(metrics.max).c_str());
		g_D2D->GetDWrite()->Write(
			maxStr,
			ctx.windowSize.x, ctx.windowSize.y,
			tpos + glm::vec2{ xoff, yoff + sz.y },
			lineColor
		);
		yoff += sz.y;

		sz = g_D2D->GetDWrite()->GetTextSize(maxStr, ctx.windowSize.x, ctx.windowSize.y);
		longestLine = eastl::max(sz.x, longestLine);
		eastl::wstring avgStr = L"avg: ";
		avgStr.append(std::to_wstring(metrics.avg).c_str());
		g_D2D->GetDWrite()->Write(
			avgStr,
			ctx.windowSize.x, ctx.windowSize.y,
			tpos + glm::vec2{ xoff, yoff + sz.y },
			lineColor
		);
		yoff += sz.y;

		sz = g_D2D->GetDWrite()->GetTextSize(avgStr, ctx.windowSize.x, ctx.windowSize.y);
		xoff += eastl::max(sz.x, longestLine) + xpad;
	}
}
#endif