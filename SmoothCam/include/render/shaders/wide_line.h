#pragma once
#include "render/shaders/shader_decl.h"

namespace Render {
	namespace Shaders {
		constexpr ShaderDecl WideLineVS = {
			6,
			R"(
struct VS_INPUT {
	float4 vPos : POS;
	float4 vColor : COL;
	float2 vOff : OFF;
};

struct VS_OUTPUT {
	float4 vPos : SV_POSITION;
	float4 vColor : COLOR0;
};

cbuffer WideLineCBuffer : register(b0) {
	float2 viewportSize;
	float lineWidthPx;
	float pad0;
};

VS_OUTPUT main(VS_INPUT input) {
	VS_OUTPUT output;
	output.vPos = input.vPos;

	float2 px = lineWidthPx / viewportSize;
	output.vPos.xy += input.vOff.xy * px * output.vPos.w;
	output.vColor = input.vColor;
	return output;
}
		)" };

		constexpr ShaderDecl WideLinePS = {
			7,
			R"(
struct PS_INPUT {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

struct PS_OUTPUT {
	float4 color : SV_Target;
};

PS_OUTPUT main(PS_INPUT input) {
	PS_OUTPUT output;
	output.color = input.color;
	return output;
}
		)" };
	}
}
