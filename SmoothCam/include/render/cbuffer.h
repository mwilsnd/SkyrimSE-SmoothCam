#pragma once
#include "render/d3d_context.h"
#include "render/shader.h"

namespace Render {
	typedef struct CBufferCreateInfo {
		size_t size;
		void* initialData = nullptr;
		D3D11_USAGE bufferUsage = D3D11_USAGE_DYNAMIC;
		uint32_t cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
	} CBufferCreateInfo;

	class CBuffer {
		public:
			explicit CBuffer(CBufferCreateInfo& info, D3DContext& ctx);
			CBuffer(const CBuffer&) = delete;
			CBuffer(CBuffer&&) noexcept = delete;
			CBuffer& operator=(const CBuffer&) = delete;
			CBuffer& operator=(CBuffer&&) noexcept = delete;

			// Bind the constant buffer to a pipeline stage at the given location
			void Bind(PipelineStage stage, uint8_t loc, D3DContext& ctx);
			// Update the contents of the buffer, starting at offset with length size
			void Update(const void* newData, size_t offset, size_t size, D3DContext& ctx);
			// Get the size of the buffer memory
			size_t Size() const noexcept;
			// Get the buffer usage it was created with
			D3D11_USAGE Usage() const noexcept;

		private:
			winrt::com_ptr<ID3D11Buffer> buffer;
			D3D11_USAGE usage;
			size_t size;
	};
};