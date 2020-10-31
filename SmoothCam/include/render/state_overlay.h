#pragma once
#ifdef WITH_D2D
#include "render/d3d_context.h"
#include "render/gradbox.h"

namespace Camera {
	class SmoothCamera;
}

namespace Render {
	class StateOverlay : public GradBox {
		public:
			explicit StateOverlay(uint32_t width, uint32_t height, Camera::SmoothCamera* camera, D3DContext& ctx);
			~StateOverlay();
			StateOverlay(const StateOverlay&) = delete;
			StateOverlay(StateOverlay&&) noexcept = delete;
			StateOverlay& operator=(const StateOverlay&) = delete;
			StateOverlay& operator=(StateOverlay&&) noexcept = delete;

			// Set the position of the graph
			void SetPosition(uint32_t x, uint32_t y) noexcept;
			// Set the size of the graph
			void SetSize(uint32_t w, uint32_t h) noexcept;
			// Draw the chart
			void Draw(const Config::OffsetGroup* curGroup, D3DContext& ctx) noexcept;

		private:
			void DrawBitset32(const std::wstring& name, const std::bitset<32>& bits,
				const glm::vec2& pos, D3DContext& ctx) noexcept;
			void DrawBool(const std::wstring& name, bool value, const glm::vec2& pos,
				D3DContext& ctx) noexcept;

			Camera::SmoothCamera* camera = nullptr;
			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t xPos = 0;
			uint32_t yPos = 0;
	};
}
#endif