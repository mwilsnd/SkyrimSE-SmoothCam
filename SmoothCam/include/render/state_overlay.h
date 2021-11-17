#pragma once
#ifdef WITH_D2D
#include "render/d3d_context.h"
#include "render/gradbox.h"

namespace Camera {
	class Thirdperson;

	namespace State {
		class BaseThird;
	}
}

namespace Render {
	class StateOverlay : public GradBox {
		public:
			StateOverlay(uint32_t width, uint32_t height, Camera::Thirdperson* camera, D3DContext& ctx);
			~StateOverlay();
			StateOverlay(const StateOverlay&) = delete;
			StateOverlay(StateOverlay&&) noexcept = delete;
			StateOverlay& operator=(const StateOverlay&) = delete;
			StateOverlay& operator=(StateOverlay&&) noexcept = delete;

			// Set the position of the graph
			void SetPosition(uint32_t x, uint32_t y) noexcept;
			// Set the size of the graph
			void SetSize(uint32_t w, uint32_t h) noexcept;
			// Set the running third-person state
			void SetThirdPersonState(Camera::State::BaseThird* third) noexcept;
			// Draw the chart
			void Draw(const RE::Actor* focus, const Config::OffsetGroup* curGroup, D3DContext& ctx) noexcept;

		private:
			void DrawBitset32(const eastl::wstring& name, const eastl::bitset<32>& bits,
				const glm::vec2& pos, D3DContext& ctx) noexcept;
			void DrawBool(const eastl::wstring& name, bool value, const glm::vec2& pos,
				D3DContext& ctx) noexcept;

			Camera::Thirdperson* camera = nullptr;
			Camera::State::BaseThird* thirdState = nullptr;

			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t xPos = 0;
			uint32_t yPos = 0;
	};
}
#endif