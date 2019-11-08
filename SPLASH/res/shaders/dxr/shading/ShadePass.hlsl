#define HLSL
#include "../Common_hlsl_cpp.hlsl"

struct VSIn {
	float4 position : POSITION0;
};

struct PSIn {
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
};

PSIn VSMain(VSIn input) {
	PSIn output;

	input.position.w = 1.f;
	// input position is already in clip space coordinates
	output.position = input.position;
	output.texCoord.x = input.position.x / 2.f + 0.5f;
	output.texCoord.y = -input.position.y / 2.f + 0.5f;

	return output;
}

cbuffer PSSceneCBuffer : register(b0) {
    float4x4 projectionToWorld;
    float3 cameraPosition;
    float padding;
    PointLightInput pointLights[NUM_POINT_LIGHTS];
}

Texture2D<float4> albedoBounceOne : register(t0);
Texture2D<float4> albedoBounceTwo : register(t1);

Texture2D<float4> normalsBounceOne : register(t2);
Texture2D<float4> normalsBounceTwo : register(t3);

Texture2D<float4> metalnessRoughnessAoBounceOne : register(t4);
Texture2D<float4> metalnessRoughnessAoBounceTwo : register(t5);

Texture2D<float2> shadows : register(t6);
// Texture2D<float4> depthAndWorldPositions : register(t6);

Texture2D<float4> brdfLUT : register(t7);

StructuredBuffer<uint> PSwaterData : register(t9);

SamplerState PSss : register(s0);

#include "PBR.hlsl"

float4 PSMain(PSIn input) : SV_Target0 {

    float3 worldNormal = normalsBounceOne.Sample(PSss, input.texCoord).xyz * 2.f - 1.f;
    float3 albedo = albedoBounceOne.Sample(PSss, input.texCoord).rgb;
    float3 metalnessRoughnessAo = metalnessRoughnessAoBounceOne.Sample(PSss, input.texCoord);
    float metalness = metalnessRoughnessAo.x;
    float roughness = metalnessRoughnessAo.y;
    float ao = metalnessRoughnessAo.z;

    // testColor += albedoBounceTwo.Sample(PSss, input.texCoord) * 0.2f;
    
    // // Just making sure the texture are bound
    // testColor += normalsBounceOne.Sample(PSss, input.texCoord) * 0.0001f;
    // testColor += normalsBounceTwo.Sample(PSss, input.texCoord) * 0.0001f;
    // testColor += metalnessRoughnessAoBounceOne.Sample(PSss, input.texCoord) * 0.0001f;
    // testColor += metalnessRoughnessAoBounceTwo.Sample(PSss, input.texCoord) * 0.0001f;
    // testColor.xy += shadows.Sample(PSss, input.texCoord) * 0.0001f;

    // float3 worldPosition = mul(float4(1.f, 1.f, 1.f, 1.f), projectionToWorld).xyz;
    float3 worldPosition = 1.f;
    
    float3 invViewDir = cameraPosition - worldPosition;

	return pbrShade(worldPosition, worldNormal, invViewDir, albedo, metalness, roughness, ao);
}

