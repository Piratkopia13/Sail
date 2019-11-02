#include "../Common.hlsl"

struct VSIn {
	float4 position : POSITION0;
	float2 texCoords : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	float3 bitangent : BINORMAL0;
};

struct PSIn {
    float4 position : SV_Position;
    float4 screenPosition : SCREENPOS;
    float3 normal : NORMAL0;
    float2 texCoords : TEXCOORD0;
    float3x3 tbn : TBN;
};

cbuffer VSSystemCBuffer : register(b0) {
    matrix sys_mWorld;
    matrix sys_mView;
    matrix sys_mProj;
}

cbuffer PSSystemCBuffer : register(b1) {
    Material sys_material_pbr;
}

PSIn VSMain(VSIn input) {
    PSIn output;

    matrix wv = mul(sys_mView, sys_mWorld);

	output.texCoords = input.texCoords;
	input.position.w = 1.f;
	output.position = mul(wv, input.position);
	// Convert position into projection space
    output.position = mul(sys_mProj, output.position);
    output.screenPosition = output.position;
    // output.position.xyz /= output.position.w;
    
	// Convert normal into world space and normalize
	output.normal = mul((float3x3) sys_mWorld, input.normal);
	output.normal = normalize(output.normal);

	// Create TBN matrix to go from tangent space to world space
	output.tbn = float3x3(
	  normalize(mul((float3x3) sys_mWorld, input.tangent)),
	  normalize(mul((float3x3) sys_mWorld, input.bitangent)),
	  output.normal
	);

	return output;
}

// TODO: check if it is worth the extra VRAM to write diffuse and specular gbuffers instead of sampling them in the raytracing shaders
//       The advantage is that mip map level can be calculated automatically here
//      
Texture2D sys_texLastScreenPositions    : register(t0);
Texture2D sys_texAlbedo                 : register(t1);
Texture2D sys_texNormal                 : register(t2);
Texture2D sys_texMetalnessRoughnessAO   : register(t3);
SamplerState PSss;

struct GBuffers {
	float4 normal               : SV_Target0;
	float4 albedo               : SV_Target1;
	float4 metalnessRoughnessAO : SV_Target2;
    float4 motionVector         : SV_Target3;
    float4 screenSpacePosition  : SV_Target4;
};

GBuffers PSMain(PSIn input) {
	GBuffers gbuffers;

	gbuffers.normal = float4(normalize(input.normal) / 2.f + .5f, 1.f);
    if (sys_material_pbr.hasNormalTexture) {
        float3 normalSample = sys_texNormal.Sample(PSss, input.texCoords).rgb;
        normalSample.y = 1.0f - normalSample.y;
        gbuffers.normal = float4(mul(normalize(normalSample * 2.f - 1.f), input.tbn) / 2.f + .5f, 1.0f);
    }

    gbuffers.albedo = sys_material_pbr.modelColor;
	if (sys_material_pbr.hasAlbedoTexture)
		gbuffers.albedo *= sys_texAlbedo.Sample(PSss, input.texCoords);

    gbuffers.metalnessRoughnessAO = float4(sys_material_pbr.metalnessScale, sys_material_pbr.roughnessScale, sys_material_pbr.aoScale, 1.0f);
	if (sys_material_pbr.hasMetalnessRoughnessAOTexture)
		gbuffers.metalnessRoughnessAO *= sys_texMetalnessRoughnessAO.Sample(PSss, input.texCoords);

    float2 screenPos = input.screenPosition.xy / input.screenPosition.w * 0.5f + 0.5f;
    screenPos.y = 1.f - screenPos.y; // Flip y cuz directX
    float2 position = input.screenPosition.xy * 0.5f + 0.5f;
    float2 positionLastFrame = sys_texLastScreenPositions.Sample(PSss, screenPos).xy;
    // gbuffers.motionVector = float4(position, 0.0f, 1.0f);
    gbuffers.motionVector = float4( (position - positionLastFrame) * 0.5f + 0.5f, 0.f, 1.0f);
    gbuffers.screenSpacePosition = float4(position, 0.f, 1.0f);

    return gbuffers;
}
