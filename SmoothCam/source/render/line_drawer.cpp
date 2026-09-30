#include "render/line_drawer.h"
#include "render/shaders/wide_line.h"
#include "render/shader_cache.h"

Render::LineDrawer::LineDrawer(D3DContext& c) : ctx(c) {
	CreateObjects(c);
}

Render::LineDrawer::~LineDrawer() {
	cbuf.reset();
	for (auto i = 0u; i < vbo.size(); i++) {
		vbo[i].reset();
	}
	vs.reset();
	ps.reset();
}

void Render::LineDrawer::CreateObjects(D3DContext& c) {
	Render::ShaderCreateInfo vsInfo(Render::Shaders::WideLineVS, Render::PipelineStage::Vertex);
	Render::ShaderCreateInfo psInfo(Render::Shaders::WideLinePS, Render::PipelineStage::Fragment);
	vs = ShaderCache::Get().Load(vsInfo, c);
	ps = ShaderCache::Get().Load(psInfo, c);

	Render::CBufferCreateInfo cbInfo;
	cbInfo.size = sizeof(WideLineParams);
	cbInfo.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	cbInfo.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	WideLineParams init = {};
	init.viewportSize = { 1.0f, 1.0f };
	init.lineWidthPx = 1.0f;
	cbInfo.initialData = &init;
	cbuf = std::make_shared<Render::CBuffer>(cbInfo, c);

	Render::VertexBufferCreateInfo vbInfo;
	vbInfo.elementSize = sizeof(Render::WideLineVertex);
	vbInfo.numElements = static_cast<uint32_t>(WideLineMaxVerts);
	vbInfo.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	vbInfo.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	vbInfo.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	vbInfo.vertexProgram = vs;
	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "OFF", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });

	for (auto i = 0u; i < vbo.size(); i++) {
		vbo[i] = std::make_unique<Render::VertexBuffer>(vbInfo, c);
	}
}

uint32_t Render::LineDrawer::EmitCorner(Render::WideLineVertex* buf, uint32_t vertexIndex, const JoinState& prev,
		const glm::vec2& newOffUnit) noexcept
{
	// collinear
	if (glm::dot(prev.offUnit, newOffUnit) > 0.9995f) {
		return 0;
	}

	const auto sum = prev.offUnit + newOffUnit;
	const float sumLen = glm::length(sum);
	if (sumLen < 0.00001f) {
		return 0;
	}

	const auto miter = sum / sumLen;
	const float miterLen = glm::min(1.0f / glm::max(glm::dot(miter, prev.offUnit), 0.25f), 4.0f);
	const float cross = prev.offUnit.x * newOffUnit.y - prev.offUnit.y * newOffUnit.x;
	const float side = (cross > 0.0f) ? -1.0f : 1.0f;
	
	const glm::vec2 apex{ 0.0f, 0.0f };
	const glm::vec2 a = prev.offUnit * side;
	const glm::vec2 m = miter * (miterLen * side);
	const glm::vec2 b = newOffUnit * side;

	buf[vertexIndex + 0] = { prev.clipPos, prev.col, apex };
	buf[vertexIndex + 1] = { prev.clipPos, prev.col, a };
	buf[vertexIndex + 2] = { prev.clipPos, prev.col, m };
	buf[vertexIndex + 3] = { prev.clipPos, prev.col, apex };
	buf[vertexIndex + 4] = { prev.clipPos, prev.col, m };
	buf[vertexIndex + 5] = { prev.clipPos, prev.col, b };
	return 6;
}

void Render::LineDrawer::Submit(const Polyline& line, const glm::mat4& matProjView) noexcept {
	if (line.points.size() < 2 || line.widthPx <= 0.0f) {
		return;
	}

	Render::SetRasterState(ctx, D3D11_FILL_MODE::D3D11_FILL_SOLID,
		D3D11_CULL_MODE::D3D11_CULL_NONE, true);
	vs->Use();
	ps->Use();

	WideLineParams params;
	params.viewportSize = ctx.windowSize;
	params.lineWidthPx = line.widthPx;
	cbuf->Update(&params, 0, sizeof(params), ctx);
	cbuf->Bind(Render::PipelineStage::Vertex, 0, ctx);

	JoinState join;
	join.valid = false;

	bufferIndex = ++bufferIndex % static_cast<uint32_t>(vbo.size());
	auto buf = reinterpret_cast<Render::WideLineVertex*>(
		vbo[bufferIndex]->Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD).pData);
	uint32_t numVerts = 0;

	const size_t n = line.points.size();
	for (size_t i = 0; i + 1 < n; i++) {
		const auto p0 = matProjView * glm::vec4(line.points[i].pos, 1.0f);
		const auto p1 = matProjView * glm::vec4(line.points[i + 1].pos, 1.0f);

		// behind camera
		if (p0.w <= 0.00001f || p1.w <= 0.00001f) {
			join.valid = false;
			continue;
		}

		const glm::vec2 p0w{ p0.x / p0.w, p0.y / p0.w };
		const glm::vec2 p1w{ p1.x / p1.w, p1.y / p1.w };
		const glm::vec2 dirPx = (p1w - p0w) * ctx.windowSize;
		if (glm::dot(dirPx, dirPx) < 0.00001f) {
			continue; // degenerate
		}

		if (numVerts + 12 > WideLineMaxVerts) {
			vbo[bufferIndex]->Unmap();
			vbo[bufferIndex]->Bind();
			vbo[bufferIndex]->DrawCount(numVerts);
			numVerts = 0;
			bufferIndex = ++bufferIndex % static_cast<uint32_t>(vbo.size());
			buf = reinterpret_cast<Render::WideLineVertex*>(
				vbo[bufferIndex]->Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD).pData);
		}

		glm::vec2 offUnit = glm::normalize(dirPx);
		offUnit = { -offUnit.y, offUnit.x };
		
		if (join.valid) {
			numVerts += EmitCorner(buf, numVerts, join, offUnit);
		}

		const auto& a = line.points[i].col;
		const auto& b = line.points[i + 1].col;
		buf[numVerts + 0] = { p0, a, offUnit * -1.0f };
		buf[numVerts + 1] = { p0, a, offUnit *  1.0f };
		buf[numVerts + 2] = { p1, b, offUnit * -1.0f };
		buf[numVerts + 3] = { p1, b, offUnit * -1.0f };
		buf[numVerts + 4] = { p0, a, offUnit *  1.0f };
		buf[numVerts + 5] = { p1, b, offUnit *  1.0f };
		numVerts += 6;
		join = JoinState{ true, p1, b, offUnit };
	}

	if (numVerts > 0) {
		vbo[bufferIndex]->Unmap();
		vbo[bufferIndex]->Bind();
		vbo[bufferIndex]->DrawCount(numVerts);
	}
}
