#include "render/line_drawer.h"
#include "render/shaders/vertex_color_screen.h"
#include "render/shader_cache.h"

Render::LineDrawer::LineDrawer(D3DContext& ctx) {
	CreateObjects(ctx);
}

Render::LineDrawer::~LineDrawer() {
	for (auto i = 0; i < NumBuffers; i++)
		vbo[i].reset();

	vs.reset();
	ps.reset();
}

void Render::LineDrawer::CreateObjects(D3DContext& ctx) {
	Render::ShaderCreateInfo vsCreateInfo(
		Render::Shaders::VertexColorScreenVS,
		Render::PipelineStage::Vertex
	);
	vs = ShaderCache::Get().Load(vsCreateInfo, ctx);

	Render::ShaderCreateInfo psCreateInfo(
		Render::Shaders::VertexColorScreenPS,
		Render::PipelineStage::Fragment
	);
	ps = ShaderCache::Get().Load(psCreateInfo, ctx);

	Render::VertexBufferCreateInfo vbInfo;
	vbInfo.elementSize = sizeof(Point);
	vbInfo.numElements = LineDrawPointBatchSize * 2;
	vbInfo.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	vbInfo.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	vbInfo.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	vbInfo.vertexProgram = vs;
	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	
	for (auto i = 0; i < NumBuffers; i++)
		vbo[i] = eastl::move(eastl::make_unique<Render::VertexBuffer>(vbInfo, ctx));
}

void Render::LineDrawer::Submit(const LineList& lines) noexcept {
	vs->Use();
	ps->Use();

	auto begin = lines.cbegin();
	auto end = lines.cend();
	uint32_t batchCount = 0;

	while (begin != end) {
		DrawBatch(
			// Flip flop buffers to avoid pipeline stalls, if possible
			batchCount % static_cast<uint32_t>(NumBuffers),
			begin,
			end
		);
		batchCount++;
	}
}

void Render::LineDrawer::DrawBatch(uint32_t bufferIndex, LineList::const_iterator& begin, LineList::const_iterator& end) {
	uint32_t batchSize = 0;
	uint32_t index = 0;
	auto buf = reinterpret_cast<glm::vec4*>(vbo[bufferIndex]->Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD).pData);

	while (begin != end) {
		buf[index] = begin->start.pos;
		buf[index+1] = begin->start.col;
		buf[index+2] = begin->end.pos;
		buf[index+3] = begin->end.col;

		begin++;
		batchSize++;
		index += 4;

		if (batchSize >= LineDrawPointBatchSize) break;
	}

	vbo[bufferIndex]->Unmap();
	vbo[bufferIndex]->Bind();
	vbo[bufferIndex]->DrawCount(batchSize * 2);
}