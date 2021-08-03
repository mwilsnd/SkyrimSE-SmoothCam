#pragma once
#include "render/shaders/shader_decl.h"

namespace Render {
	namespace Shaders {
		constexpr ShaderDecl DrawFullscreenTextureVS = {
			2,
			R"(
struct VS_INPUT {
	float3 vPos : POS;
	float2 vUV : UV;
};

struct VS_OUTPUT {
	float4 vPos : SV_POSITION;
	float2 vUV : COLOR0;
};

VS_OUTPUT main(VS_INPUT input) {
	VS_OUTPUT output;
	output.vPos = float4(input.vPos.xyz, 1.0f);
	output.vUV = input.vUV;
	return output;
}
		)" };

		constexpr ShaderDecl DrawFullscreenTexturePS = {
			3,
			R"(
struct PS_INPUT {
	float4 vPos : SV_POSITION;
	float2 uv : COLOR0;
};

struct PS_OUTPUT {
	float4 color : SV_Target;
};

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

PS_OUTPUT main(PS_INPUT input) {
	PS_OUTPUT output;
	output.color = tex.Sample(texSampler, input.uv);
	return output;
}
		)" };
	}
}