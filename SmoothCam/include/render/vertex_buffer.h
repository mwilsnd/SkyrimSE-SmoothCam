#pragma once
#include "render/d3d_context.h"
#include "render/shader.h"

namespace Render {
	using IALayout = std::vector<D3D11_INPUT_ELEMENT_DESC>;
	struct VertexBufferCreateInfo {
		size_t elementSize = 0;
		size_t numElements = 0;
		D3D11_SUBRESOURCE_DATA* elementData = nullptr;
		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		D3D11_USAGE bufferUsage = D3D11_USAGE_IMMUTABLE;
		uint32_t cpuAccessFlags = 0;
		std::shared_ptr<Shader> vertexProgram;
		IALayout iaLayout = {};
	};

	class VertexBuffer {
		public:
			explicit VertexBuffer(const VertexBufferCreateInfo& createInfo, D3DContext& ctx) noexcept;
			~VertexBuffer() noexcept;
			VertexBuffer(const VertexBuffer&) = delete;
			VertexBuffer(VertexBuffer&&) noexcept = delete;
			VertexBuffer& operator=(const VertexBuffer&) = delete;
			VertexBuffer& operator=(VertexBuffer&&) noexcept = delete;

			// Bind the vertex buffer for drawing
			void Bind(uint32_t offset = 0) noexcept;
			// Draw the full contents of the buffer
			void Draw() noexcept;
			// Draw the given number of elements from the buffer
			void DrawCount(uint32_t num) noexcept;
			// Map the buffer to CPU memory
			D3D11_MAPPED_SUBRESOURCE& Map(D3D11_MAP mode) noexcept;
			// Unmap the buffer
			void Unmap() noexcept;
			// Create the input assembler layout
			void CreateIALayout(const IALayout& layout, const std::shared_ptr<Shader>& vertexProgram) noexcept;

		private:
			uint32_t stride;
			uint32_t vertexCount;
			D3D11_PRIMITIVE_TOPOLOGY topology;
			D3DContext context;
			winrt::com_ptr<ID3D11Buffer> buffer;
			winrt::com_ptr<ID3D11InputLayout> inputLayout;
			D3D11_MAPPED_SUBRESOURCE mappedBuffer;

			void CreateBuffer(size_t size, D3D11_USAGE usage, uint32_t cpuAccessFlags,
				const D3D11_SUBRESOURCE_DATA* initialData) noexcept;
	};
}