#pragma once
#include "render/shaders/shader_decl.h"

namespace Render {
	namespace Shaders {
		constexpr ShaderDecl VertexColorWorldVS = {
			4,
			R"(
struct VS_INPUT {
	float3 vPos     : POS;
	float2 vUV      : UV;
	float3 vNormal  : NORMAL;
	float4 vColor   : COLOR;
};

struct VS_OUTPUT {
	float4 vPos     : SV_POSITION;
	float2 vUV      : COLOR0;
	float3 vNormal  : COLOR1;
	float4 vColor   : COLOR2;
};

cbuffer PerObject : register(b0) {
	float4x4 matModel;
};

cbuffer PerFrame : register(b1) {
	float4x4 matProjView;
};

VS_OUTPUT main(VS_INPUT input) {
	float4 pos = float4(input.vPos, 1.0f);
	pos = mul(matModel, pos);
	pos = mul(matProjView, pos);

	VS_OUTPUT output;
	output.vPos = pos;
	output.vUV = input.vUV;
	// If we ever use normals later, run thru inverse transpose first
	output.vNormal = input.vNormal;
	output.vColor = input.vColor;

	return output;
}
		)" };

		constexpr ShaderDecl VertexColorWorldPS = {
			5,
			R"(
struct PS_INPUT {
	float4 pos      : SV_POSITION;
	float2 uv       : COLOR0;
	float3 normal   : COLOR1;
	float4 color    : COLOR2;
};

struct PS_OUTPUT {
	float4 color : SV_Target;
};

cbuffer PerFrame : register(b1) {
	float4x4 matProjView;
	float4 tint;
	float curTime;
};

PS_OUTPUT main(PS_INPUT input) {
	PS_OUTPUT output;
	// scale tint.rgb by color.xyz so that black colors remain unchanged
	output.color = float4(lerp(input.color.xyz, tint.xyz * input.color.xyz, 0.5f), input.color.w);
	return output;
}
		)" };
	}
}