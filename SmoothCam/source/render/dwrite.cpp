#ifdef WITH_D2D
#include "render/dwrite.h"
#include "render/d2d.h"

Render::DWrite::DWrite(Render::D2D* d2d) : d2d(d2d) {
	if (!SUCCEEDED(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE::DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(factory.put())
	)))
	{
		FatalError(L"SmoothCam: Failed to initialize direct write");
	}

	CreateFont();
}

Render::DWrite::~DWrite() {
	textLayout = nullptr;
	textFormat = nullptr;
	factory = nullptr;
}

void Render::DWrite::CreateFont() {
	if (!SUCCEEDED(factory->CreateTextFormat(
		L"Lucida Console",
		nullptr,
		DWRITE_FONT_WEIGHT_LIGHT,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		14.0f,
		L"en-US",
		textFormat.put()
	)))
	{
		FatalError(L"SmoothCam: Failed to create text format");
	}

	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}

// Generate and return a text layout
winrt::com_ptr<IDWriteTextLayout>& Render::DWrite::GetLayout(const std::wstring_view& text, float maxWidth, float maxHeight) {
	textLayout = nullptr;
	if (!SUCCEEDED(factory->CreateTextLayout(
		text.data(),
		text.length(),
		textFormat.get(),
		maxWidth,
		maxHeight,
		textLayout.put()
	)))
	{
		FatalError(L"SmoothCam: Failed to create text layout");
	}
	return textLayout;
}

void Render::DWrite::Write(const std::wstring_view& text, float maxWidth, float maxHeight, const glm::vec2& pos,
	const glm::vec4& color) noexcept
{
	textLayout = nullptr;
	if (!SUCCEEDED(factory->CreateTextLayout(
		text.data(),
		text.length(),
		textFormat.get(),
		maxWidth,
		maxHeight,
		textLayout.put()
	)))
	{
		FatalError(L"SmoothCam: Failed to create text layout");
	}

	d2d->context->DrawTextLayout(
		{ pos.x, pos.y },
		GetLayout(text, maxWidth, maxHeight).get(),
		d2d->GetColorBrush(color).get(),
		D2D1_DRAW_TEXT_OPTIONS_NONE
	);
}

// Draw using a layout
void Render::DWrite::Write(winrt::com_ptr<IDWriteTextLayout>& layout, const glm::vec2& pos,
	const glm::vec4& color) noexcept
{
	d2d->context->DrawTextLayout(
		{ pos.x, pos.y },
		layout.get(),
		d2d->GetColorBrush(color).get(),
		D2D1_DRAW_TEXT_OPTIONS_NONE
	);	
}

glm::vec2 Render::DWrite::GetTextSize(const std::wstring_view& text, float maxWidth, float maxHeight) noexcept {
	DWRITE_TEXT_METRICS metrics;
	if (!SUCCEEDED(GetLayout(text, maxWidth, maxHeight)->GetMetrics(&metrics))) {
		FatalError(L"SmoothCam: Failed to get text metrics");
	}

	return { metrics.width, metrics.height };
}

// Get the size from a layout
glm::vec2 Render::DWrite::GetTextSize(winrt::com_ptr<IDWriteTextLayout>& layout) noexcept {
	DWRITE_TEXT_METRICS metrics;
	if (!SUCCEEDED(layout->GetMetrics(&metrics))) {
		FatalError(L"SmoothCam: Failed to get text metrics");
	}

	return { metrics.width, metrics.height };
}
#endif