#include "render/gradbox.h"
#include "render/vertex_buffer.h"
#include "render/shader.h"
#include "render/shaders/shader_vertex_color.h"

Render::GradBox::GradBox(Render::D3DContext& ctx, uint32_t width, uint32_t height) :
	bgSize(width, height)
{
	backgroundDirty = true;
	Render::ShaderCreateInfo vsCreateInfo(
		Render::Shaders::VertexColorPassThruVS,
		Render::PipelineStage::Vertex
	);
	vsBackground = std::make_shared<Render::Shader>(vsCreateInfo, ctx);

	Render::ShaderCreateInfo psCreateInfo(
		Render::Shaders::VertexColorPassThruPS,
		Render::PipelineStage::Fragment
	);
	psBackground = std::make_shared<Render::Shader>(psCreateInfo, ctx);
}

Render::GradBox::~GradBox() {
	vboBackground.reset();
	vsBackground.reset();
	psBackground.reset();
}

void Render::GradBox::SetBackgroundSize(const glm::vec2& size) noexcept {
	bgSize = static_cast<glm::uvec2>(size);
	backgroundDirty = true;
}

void Render::GradBox::SetBackgroundPosition(const glm::vec2& pos) noexcept {
	bgPos = static_cast<glm::uvec2>(pos);
	backgroundDirty = true;
}

void Render::GradBox::SetBackgroundColors(const glm::vec4& color1, const glm::vec4& color2) noexcept {
	backgroundDirty = true;
}

void Render::GradBox::DrawBackground(Render::D3DContext& ctx) noexcept {
	if (backgroundDirty)
		MakeBackgroundVerts(ctx);
	vsBackground->Use();
	psBackground->Use();
	vboBackground->Bind();
	vboBackground->Draw();
}

void Render::GradBox::MakeBackgroundVerts(D3DContext& ctx) noexcept {
	const auto w = static_cast<float>(bgSize.x);
	const auto h = static_cast<float>(bgSize.y);
	const auto x = static_cast<float>(bgPos.x);
	const auto y = static_cast<float>(bgPos.y);
	backgroundVerts = {
		x,   y,   0.0f, 0.0f, bgColor1.r, bgColor1.g, bgColor1.b, bgColor1.a,
		x,   y+h, 0.0f, 0.0f, bgColor2.r, bgColor2.g, bgColor2.b, bgColor2.a,
		x+w, y+h, 0.0f, 0.0f, bgColor2.r, bgColor2.g, bgColor2.b, bgColor2.a,

		x,   y,   0.0f, 0.0f, bgColor1.r, bgColor1.g, bgColor1.b, bgColor1.a,
		x+w, y+h, 0.0f, 0.0f, bgColor2.r, bgColor2.g, bgColor2.b, bgColor2.a,
		x+w, y,   0.0f, 0.0f, bgColor1.r, bgColor1.g, bgColor1.b, bgColor1.a,
	};

	if (!vboBackground) {
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = backgroundVerts.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		Render::VertexBufferCreateInfo vbInfo;
		vbInfo.elementSize = sizeof(float) * 8;
		vbInfo.numElements = 6;
		vbInfo.elementData = &data;
		vbInfo.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		vbInfo.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		vbInfo.cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vbInfo.vertexProgram = vsBackground;

		vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{
			"POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
			0, D3D11_INPUT_PER_VERTEX_DATA, 0
		});
		vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{
			"COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		});

		vboBackground = std::make_unique<Render::VertexBuffer>(vbInfo, ctx);
	} else {
		auto data = vboBackground->Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD);
		memcpy(data.pData, backgroundVerts.data(), backgroundVerts.size() * sizeof(float));
		vboBackground->Unmap();
	}

	backgroundDirty = false;
}