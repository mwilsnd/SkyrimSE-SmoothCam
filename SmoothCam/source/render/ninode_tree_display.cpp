#ifdef WITH_D2D
#include "render/ninode_tree_display.h"
#include "render/d2d.h"
#include "render/dwrite.h"
#include <iomanip>

extern std::unique_ptr<Render::D2D> g_D2D;

template< typename T >
std::wstring int_to_hex(T i) {
	std::wstringstream stream;
	stream << L"0x"
		<< std::setfill(L'0') << std::setw(sizeof(T) * 2)
		<< std::hex << i;
	return stream.str();
}

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

void Render::NiNodeTreeDisplay::Draw(D3DContext& ctx, RE::NiNode* node) noexcept {
	DrawBackground(ctx);

	builder.clear();
	std::wstring str;
	
	const auto maxSize = glm::vec2{ width, height };
	std::function<void(RE::NiAVObject*, float&, float&, uint32_t)> walkFun;
	walkFun = [&ctx, &maxSize, &walkFun, &str, this](RE::NiAVObject* n, float& x, float& y, uint32_t level) {
		constexpr auto lineHeight = 10.0f;

		if (!n || n->name.empty()) return;

		// uuuggggggghhhhhhhhhh
		auto len = MultiByteToWideChar(CP_UTF8, 0, n->name.c_str(), -1, nullptr, 0);
		if (len > bufSize || buffer == nullptr) {
			if (buffer) delete[] buffer;
			buffer = new wchar_t[len];
			bufSize = len;
		}
		MultiByteToWideChar(CP_UTF8, 0, n->name.c_str(), -1, buffer , len);

		str.assign(buffer, static_cast<size_t>(len));

		if (y + lineHeight >= maxSize.y) {
			auto& layout = g_D2D->GetDWrite()->GetLayout(builder, maxSize.x, maxSize.y);
			auto size = g_D2D->GetDWrite()->GetTextSize(layout);
			g_D2D->GetDWrite()->Write(layout, { x, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });

			x += size.x;
			y = 0.0f;
			builder.clear();
		}

		for (uint32_t i = 0; i < level; i++) builder += L"-";
		builder.append(std::move(str));
		builder += L"::";
		builder += int_to_hex(n->flags.underlying());
		builder += L"\n";
		y += lineHeight;

		auto no = skyrim_cast<RE::NiNode*>(n);
		if (no) {
			for (auto& child : no->children)
				walkFun(child.get(), x, y, level + 1);
		}
	};

	float x = 0.0f;
	float y = 0.0f;
	walkFun(node, x, y, 0);

	if (!builder.empty()) {
		auto& layout = g_D2D->GetDWrite()->GetLayout(builder, maxSize.x, maxSize.y);
		auto size = g_D2D->GetDWrite()->GetTextSize(layout);
		g_D2D->GetDWrite()->Write(layout, { x, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
	}
}
#endif