#include "render/mesh_drawer.h"

Render::MeshDrawer::MeshDrawer(MeshCreateInfo& info, eastl::shared_ptr<Render::CBuffer>& perObjectBuffer, D3DContext& ctx) :
	vs(info.vs), ps(info.ps)
{
	assert(perObjectBuffer->Size() == sizeof(glm::mat4));
	assert(perObjectBuffer->Usage() == D3D11_USAGE_DYNAMIC);
	cbufPerObject = perObjectBuffer;

	CreateObjects(info.mesh->vertices, ctx);
}

Render::MeshDrawer::~MeshDrawer() {
	vbo.reset();
	vs.reset();
	ps.reset();
	cbufPerObject.reset();
}

void Render::MeshDrawer::CreateObjects(eastl::vector<Model::Vertex>& vertices, D3DContext& ctx) {
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertices.data();
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	Render::VertexBufferCreateInfo vbInfo;
	vbInfo.elementSize = sizeof(Model::Vertex);
	vbInfo.numElements = vertices.size();
	vbInfo.elementData = &data;
	vbInfo.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	vbInfo.bufferUsage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	vbInfo.cpuAccessFlags = 0;
	vbInfo.vertexProgram = vs;

	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });

	vbo = eastl::make_unique<Render::VertexBuffer>(vbInfo, ctx);
	context = ctx;
}

void Render::MeshDrawer::Submit(glm::mat4& modelMatrix) noexcept {
	cbufPerObject->Update(&modelMatrix, 0, sizeof(modelMatrix), context);
	cbufPerObject->Bind(PipelineStage::Vertex, 0, context);
	vs->Use();
	ps->Use();
	vbo->Bind();
	vbo->Draw();
}

void Render::MeshDrawer::SetShaders(eastl::shared_ptr<Shader>& nvs, eastl::shared_ptr<Shader>& nps) {
	IALayout iaLayout;
	iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	vbo->CreateIALayout(iaLayout, nvs);
	vs = nvs;
	ps = nps;
}