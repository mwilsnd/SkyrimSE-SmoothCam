#pragma once

namespace Render {
	namespace Shaders {
		typedef struct ShaderDecl {
			uint8_t uid = 0;
			const char* source = nullptr;
		} ShaderDecl;
	}
}