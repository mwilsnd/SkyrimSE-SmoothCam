#include "render/shader.h"

Render::Shader::Shader(const ShaderCreateInfo& createInfo, D3DContext& ctx) noexcept : stage(createInfo.stage), context(ctx) {
	validBinary = Compile(createInfo.source, createInfo.entryName, createInfo.version);
	if (!validBinary) return;

	if (stage == PipelineStage::Vertex) {
		auto result = context.device->CreateVertexShader(binary->GetBufferPointer(), binary->GetBufferSize(),
			nullptr, &program.vertex);
		validProgram = SUCCEEDED(result);
		if (!validProgram)
			_ERROR("SmoothCam: A shader failed to compile.");
	} else {
		auto result = context.device->CreatePixelShader(binary->GetBufferPointer(), binary->GetBufferSize(),
			nullptr, &program.fragment);
		validProgram = SUCCEEDED(result);
		if (!validProgram)
			_ERROR("SmoothCam: A shader failed to compile.");
	}
}

Render::Shader::~Shader() noexcept {
	if (validProgram) {
		switch (stage) {
			case PipelineStage::Vertex:
				program.vertex->Release();
				break;
			case PipelineStage::Fragment:
				program.fragment->Release();
				break;
		}
	}
	validProgram = false;
}

void Render::Shader::Use() noexcept {
	if (stage == PipelineStage::Vertex) {
		context.context->VSSetShader(program.vertex, nullptr, 0);
	} else {
		context.context->PSSetShader(program.fragment, nullptr, 0);
	}
}

bool Render::Shader::IsValid() const noexcept {
	return validProgram && validBinary;
}

bool Render::Shader::Compile(const std::string& source, const std::string& entryName, const std::string& version) noexcept {
	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
	winrt::com_ptr<ID3DBlob> errorBlob;

	auto versionStr = stage == PipelineStage::Vertex ?
		std::string("vs_").append(version) :
		std::string("ps_").append(version);

	auto result = D3DCompile(
		source.c_str(), source.length(), nullptr, nullptr, nullptr,
		entryName.c_str(),
		versionStr.c_str(),
		compileFlags, 0, binary.put(), errorBlob.put()
	);

	if (!SUCCEEDED(result)) {
		if (errorBlob)
			_ERROR(static_cast<LPCSTR>(errorBlob->GetBufferPointer()));
		return false;
	}

	return true;
}