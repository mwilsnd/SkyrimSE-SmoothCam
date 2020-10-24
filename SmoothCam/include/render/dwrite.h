#pragma once
#include <d2d1_2.h>
#include <dwrite_2.h>
#include <winrt/base.h>
#pragma comment(lib, "D2d1.lib")
#pragma comment(lib, "Dwrite.lib")

namespace Render {
	class D2D;



	class DWrite {
		public:
			DWrite(D2D* d2d);
			~DWrite();

			// Draw text
			void Write(const std::wstring& text, float maxWidth, float maxHeight, const glm::vec2& pos,
				const glm::vec4& color) noexcept;

			// Get the width and height of a text input, were it to be drawn
			glm::vec2 GetTextSize(const std::wstring& text, float maxWidth, float maxHeight) noexcept;

		private:
			void CreateFont();

			D2D* d2d;
			winrt::com_ptr<IDWriteFactory2> factory;
			winrt::com_ptr<IDWriteTextFormat> textFormat;
			winrt::com_ptr<IDWriteTextLayout> textLayout;
	};
}