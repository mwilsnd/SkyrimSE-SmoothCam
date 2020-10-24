#pragma once

namespace Render {
	namespace Shaders {
		constexpr const auto VertexColorPassThruVS = R"(
struct VS_INPUT {
	float4 vPos : POS;
	float4 vColor : COL;
};

struct VS_OUTPUT {
	float4 vPos : SV_POSITION;
	float4 vColor : COLOR0;
};

cbuffer PerFrame : register(b1) {
	float4x4 matProjView;
};

VS_OUTPUT main(VS_INPUT input) {
	float4 pos = float4(input.vPos.xyz, 1.0f);
	pos = mul(matProjView, pos);

	VS_OUTPUT output;
	output.vPos = pos;
	output.vColor = input.vColor;
	return output;
}
		)";

		constexpr const auto VertexColorPassThruPS = R"(
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
		)";
	}
}