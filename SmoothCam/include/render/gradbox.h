#pragma once
#include "render/d3d_context.h"

namespace Render {
	class VertexBuffer;

	class GradBox {
		public:
			explicit GradBox(Render::D3DContext& ctx, uint32_t width, uint32_t height);
			~GradBox();
			GradBox(const GradBox&) = delete;
			GradBox(GradBox&&) noexcept = delete;
			GradBox& operator=(const GradBox&) = delete;
			GradBox& operator=(GradBox&&) noexcept = delete;

			void SetBackgroundSize(const glm::vec2& size) noexcept;
			void SetBackgroundPosition(const glm::vec2& pos) noexcept;
			void SetBackgroundColors(const glm::vec4& color1, const glm::vec4& color2) noexcept;
			void DrawBackground(Render::D3DContext& ctx) noexcept;

		private:
			void MakeBackgroundVerts(D3DContext& ctx) noexcept;

			bool backgroundDirty = false;
			glm::vec4 bgColor1 = { 0.05f, 0.05f, 0.05f, 0.7f };
			glm::vec4 bgColor2 = { 0.1f, 0.1f, 0.1f, 0.7f };
			std::shared_ptr<Render::Shader> vsBackground;
			std::shared_ptr<Render::Shader> psBackground;

			std::vector<float> backgroundVerts;
			std::unique_ptr<Render::VertexBuffer> vboBackground;

			glm::uvec2 bgSize = { 0, 0 };
			glm::ivec2 bgPos = { 0, 0 };
	};
}