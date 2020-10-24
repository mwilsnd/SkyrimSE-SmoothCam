#pragma once

namespace Render {
	namespace Shaders {
		constexpr const auto VertexColorVS = R"(
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
		)";

		constexpr const auto VertexColorPS = R"(
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
	float curTime;
};

PS_OUTPUT main(PS_INPUT input) {
	PS_OUTPUT output;
	output.color = input.color;
	return output;
}
		)";
	}
}