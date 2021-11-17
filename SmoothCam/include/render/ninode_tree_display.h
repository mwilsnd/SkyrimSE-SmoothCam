#pragma once
#ifdef WITH_D2D
#include "render/d3d_context.h"
#include "render/gradbox.h"
#include "string_builder.h"

namespace Render {
	class NiNodeTreeDisplay : public GradBox {
		public:
			NiNodeTreeDisplay(uint32_t width, uint32_t height, D3DContext& ctx);
			~NiNodeTreeDisplay();
			NiNodeTreeDisplay(const NiNodeTreeDisplay&) = delete;
			NiNodeTreeDisplay(NiNodeTreeDisplay&&) noexcept = delete;
			NiNodeTreeDisplay& operator=(const NiNodeTreeDisplay&) = delete;
			NiNodeTreeDisplay& operator=(NiNodeTreeDisplay&&) noexcept = delete;

			// Set the position of the graph
			void SetPosition(uint32_t x, uint32_t y) noexcept;
			// Set the size of the graph
			void SetSize(uint32_t w, uint32_t h) noexcept;
			// Draw the chart
			void Draw(D3DContext& ctx, RE::NiNode* node) noexcept;

		private:
			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t xPos = 0;
			uint32_t yPos = 0;
			
			StringBuilder<eastl::wstring> builder;
			wchar_t* buffer = nullptr;
			size_t bufSize = 0;
	};
}

#endif