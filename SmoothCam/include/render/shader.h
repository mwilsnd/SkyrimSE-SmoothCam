#pragma once
#include "render/d3d_context.h"
#include <d3dcompiler.h>
#include "render/shaders/shader_decl.h"

namespace Render {
	class VertexBuffer;

	enum class PipelineStage {
		Vertex,
		Fragment,
	};

	struct ShaderCreateInfo {
		Shaders::ShaderDecl source;
		eastl::string entryName = "main";
		eastl::string version;
		PipelineStage stage;

		ShaderCreateInfo(const Shaders::ShaderDecl& source, PipelineStage stage,
			eastl::string&& entryName = "main", eastl::string&& version = "5_0")
			: source(source), entryName(entryName), version(version), stage(stage)
		{}

		size_t Hash() const {
			size_t seed = source.uid;
			Util::HashCombine(seed, entryName);
			Util::HashCombine(seed, version);
			Util::HashCombine(seed, stage);
			return seed;
		}

		bool operator==(const ShaderCreateInfo& other) const {
			return
				stage == other.stage && version == other.version &&
				entryName == other.entryName && source.uid == other.source.uid;
		}
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

			bool Compile(const eastl::string& source, const eastl::string& entryName, const eastl::string& version) noexcept;

			friend class VertexBuffer;
	};
}