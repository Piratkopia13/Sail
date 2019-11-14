#include "Common.hlsl"

struct VSIn {
	float4 position : POSITION0;
};

struct PSIn {
	float4 position : SV_Position;
};

cbuffer VSPSSystemCBuffer : register(b0) {
	matrix sys_mWorld;
	matrix sys_mView;
	matrix sys_mProj;
    Material sys_material;
}

PSIn VSMain(VSIn input) {
	PSIn output;

	input.position.w = 1.f;
	output.position = mul(sys_mWorld, input.position);
	output.position = mul(sys_mView, output.position);
	output.position = mul(sys_mProj, output.position);
	
	return output;
}

float4 PSMain(PSIn input) : SV_Target0 {
	return float4(sys_material.modelColor.rgb, 1.0f);
}

