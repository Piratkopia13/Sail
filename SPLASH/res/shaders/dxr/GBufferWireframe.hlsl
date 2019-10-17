#include "../Common.hlsl"


struct VSIn {
	float4 position : POSITION0;
	float3 normal : NORMAL0;
};

struct PSIn {
	float4 position : SV_Position;
	float4 posVS : POSVS;
	float3 normal : NORMAL0;
};

cbuffer VSPSSystemCBuffer : register(b0) {
	matrix sys_mWorld;
	matrix sys_mView;
	matrix sys_mProj;
    Material sys_material_pbr;
}

PSIn VSMain(VSIn input) {
	PSIn output;

	matrix wv = mul(sys_mView, sys_mWorld);

	input.position.w = 1.f;
	output.position = mul(wv, input.position);
	output.posVS = output.position;
	// Convert position into projection space
	output.position = mul(sys_mProj, output.position);

	// Convert normal into world space and normalize
	output.normal = mul((float3x3) sys_mWorld, input.normal);
	output.normal = normalize(output.normal);
	
	return output;
}

struct GBuffers {
	float4 normal  : SV_Target0;
	float4 albedo : SV_Target1;
	float4 metalnessRoughnessAO : SV_Target2;
};

GBuffers PSMain(PSIn input) {
	GBuffers gbuffers;

	gbuffers.normal = float4(0,0,0, 1.f);
	gbuffers.albedo = sys_material_pbr.modelColor;
	gbuffers.metalnessRoughnessAO = float4(sys_material_pbr.metalnessScale, sys_material_pbr.roughnessScale, sys_material_pbr.aoScale, 1.0f);

	return gbuffers;
}

