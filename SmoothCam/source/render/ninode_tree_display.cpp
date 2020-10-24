#ifdef WITH_D2D
#include "render/ninode_tree_display.h"
#include "render/d2d.h"
#include "render/dwrite.h"

extern std::unique_ptr<Render::D2D> g_D2D;

Render::NiNodeTreeDisplay::NiNodeTreeDisplay(uint32_t width, uint32_t height, D3DContext& ctx) :
	width(width), height(height), GradBox(ctx, width, height)
{}

Render::NiNodeTreeDisplay::~NiNodeTreeDisplay() {}

void Render::NiNodeTreeDisplay::SetPosition(uint32_t x, uint32_t y) noexcept {
	xPos = x;
	yPos = y;
	SetBackgroundPosition({ x, y });
}

void Render::NiNodeTreeDisplay::SetSize(uint32_t w, uint32_t h) noexcept {
	width = w;
	height = h;
	SetBackgroundSize({ w, h });
}

void Render::NiNodeTreeDisplay::Draw(D3DContext& ctx, NiNode* node) noexcept {
	DrawBackground(ctx);

	const auto maxSize = glm::vec2{ width, height };
	std::function<void(NiNode*, float&, float&, uint32_t)> walkFun;
	walkFun = [&ctx, &walkFun, this, &maxSize](NiNode* n, float& indent, float& line, uint32_t level) {
		constexpr auto lineHeight = 14.0f;

		DrawNode(ctx, n, indent, line, level);
		line = line + lineHeight;

		if (line + lineHeight >= maxSize.y) {
			line = 0.0f;
			indent += 300.0f;
		}

		for (auto i = 0; i < n->m_children.m_size; i++) {
			auto no = DYNAMIC_CAST(n->m_children.m_data[i], NiAVObject, NiNode);
			if (!no) continue;

			DrawNode(ctx, no, indent, line, level);
			line += lineHeight;

			if (line + lineHeight >= maxSize.y) {
				line = 0.0f;
				indent += 300.0f;
			}

			if (no->m_children.m_size > 0) {
				walkFun(no, indent, line, level + 1);
			}
		}
	};

	float xof = 0.0f;
	float yof = 0.0f;
	walkFun(node, xof, yof, 0);
}

void Render::NiNodeTreeDisplay::DrawNode(D3DContext& ctx, NiNode* no, float indent, float line, uint32_t level) {
	const auto pos = glm::vec2{ xPos, yPos };

	if (no->m_name) {
		// uuuggggggghhhhhhhhhh
		auto len = MultiByteToWideChar(CP_UTF8, 0, no->m_name, -1, nullptr, 0);
		wchar_t* wstr = new wchar_t[len];
		MultiByteToWideChar(CP_UTF8, 0, no->m_name, -1, wstr , len);

		std::wstring str;
		str.reserve(len+1);
		for (auto i = 0; i < len; i++)
			str.push_back(wstr[i]);
		str[len] = 0;
		delete[] wstr;

		std::wstring pre;
		for (auto i = 0; i < level; i++) pre.append(L"-");
		pre.append(str);

		g_D2D->GetDWrite()->Write(
			pre,
			ctx.windowSize.x, ctx.windowSize.y,
			pos + glm::vec2{ indent, line },
			{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	} else {
		std::wstring pre;
		for (auto i = 0; i < level; i++) pre.append(L"-");
		pre.append(L"No name");

		g_D2D->GetDWrite()->Write(
			pre,
			ctx.windowSize.x, ctx.windowSize.y,
			pos + glm::vec2{ indent, line },
			{ 1.0f, 0.0f, 0.0f, 1.0f }
		);
	}
}
#endif