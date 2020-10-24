#pragma once
#include "render/d3d_context.h"
#include <d3dcompiler.h>

namespace Render {
	class VertexBuffer;

	enum class PipelineStage {
		Vertex,
		Fragment,
	};

	struct ShaderCreateInfo {
		std::string source;
		std::string entryName = "main";
		std::string version;
		PipelineStage stage;

		ShaderCreateInfo(std::string&& source, PipelineStage stage,
			std::string&& entryName = "main", std::string&& version = "5_0")
			: source(source), entryName(entryName), version(version), stage(stage)
		{}
	};

	class Shader {
		public:
			explicit Shader(const ShaderCreateInfo& createInfo, D3DContext& ctx) noexcept;
			~Shader() noexcept;
			Shader(const Shader&) = delete;
			Shader(Shader&&) noexcept = delete;
			Shader& operator=(const Shader&) = delete;
			Shader& operator=(Shader&&) noexcept = delete;

			// Use the shader for draw operations
			void Use() noexcept;
			// Returns true if the shader is valid
			bool IsValid() const noexcept;

		private:
			D3DContext context;
			winrt::com_ptr<ID3DBlob> binary;
			PipelineStage stage;
			bool validBinary = false;
			bool validProgram = false;

			// no com_ptr here - we is being bad
			union {
				ID3D11VertexShader* vertex;
				ID3D11PixelShader* fragment;
			} program;

			bool Compile(const std::string& source, const std::string& entryName, const std::string& version) noexcept;

			friend class VertexBuffer;
	};
}