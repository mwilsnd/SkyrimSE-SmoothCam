#include "render/vertex_buffer.h"

Render::VertexBuffer::VertexBuffer(const VertexBufferCreateInfo& createInfo, D3DContext& ctx) noexcept
	: context(ctx), stride(createInfo.elementSize), topology(createInfo.topology), vertexCount(createInfo.numElements)
{
	CreateBuffer(
		createInfo.elementSize * createInfo.numElements,
		createInfo.bufferUsage,
		createInfo.cpuAccessFlags,
		createInfo.elementData
	);
	CreateIALayout(createInfo.iaLayout, createInfo.vertexProgram.get());
}

Render::VertexBuffer::~VertexBuffer() noexcept {
	if (buffer)
		buffer = nullptr;

	if (inputLayout)
		inputLayout = nullptr;
}

void Render::VertexBuffer::CreateIALayout(const IALayout& layout, const Shader* vertexProgram) noexcept {
	if (inputLayout)
		inputLayout = nullptr;

	const auto layoutCode = context.device->CreateInputLayout(
		layout.data(), layout.size(),
		vertexProgram->binary->GetBufferPointer(), vertexProgram->binary->GetBufferSize(),
		inputLayout.put()
	);
	if (!SUCCEEDED(layoutCode))
		FatalError(L"SmoothCam: Failed to create input assembler layout.");
}

void Render::VertexBuffer::CreateBuffer(size_t size, D3D11_USAGE usage, uint32_t cpuAccessFlags,
	const D3D11_SUBRESOURCE_DATA* initialData) noexcept 
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = static_cast<UINT>(size);
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	desc.Usage = usage;
	desc.CPUAccessFlags = cpuAccessFlags;

	const auto code = context.device->CreateBuffer(&desc, initialData, buffer.put());
	if (!SUCCEEDED(code))
		FatalError(L"SmoothCam: Failed to create D3D vertex buffer.");
}

void Render::VertexBuffer::Bind(uint32_t offset) noexcept {
	const auto buf = buffer.get();
	context.context->IASetInputLayout(inputLayout.get());
	context.context->IASetVertexBuffers(0, 1, &buf, &stride, &offset);
	context.context->IASetPrimitiveTopology(topology);
}

void Render::VertexBuffer::Draw() noexcept {
	context.context->Draw(vertexCount, 0);
}

void Render::VertexBuffer::DrawCount(uint32_t num) noexcept {
	assert(num <= vertexCount);
	context.context->Draw(num, 0);
}

D3D11_MAPPED_SUBRESOURCE& Render::VertexBuffer::Map(D3D11_MAP mode) noexcept {
	const auto code = context.context->Map(buffer.get(), 0, mode, 0, &mappedBuffer);
	if (!SUCCEEDED(code))
		FatalError(L"SmoothCam: Failed to map a D3D vertex buffer.");
	return mappedBuffer;
}

void Render::VertexBuffer::Unmap() noexcept {
	context.context->Unmap(buffer.get(), 0);
}