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
    float4x4 clipToView;
    float4x4 viewToWorld;
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

Texture2D<float4> depthAndWorldPositions : register(t7);

Texture2D<float4> brdfLUT : register(t8);

StructuredBuffer<uint> PSwaterData : register(t9);

SamplerState PSss : register(s0);

#include "PBR.hlsl"

float4 PSMain(PSIn input) : SV_Target0 {

    float3 worldNormalOne = normalsBounceOne.Sample(PSss, input.texCoord).xyz * 2.f - 1.f;
    float3 worldNormalTwo = normalsBounceTwo.Sample(PSss, input.texCoord).xyz * 2.f - 1.f;

    float3 albedoOne = albedoBounceOne.Sample(PSss, input.texCoord).rgb;
    float3 albedoTwo = albedoBounceTwo.Sample(PSss, input.texCoord).rgb;

    float3 metalnessRoughnessAoOne = metalnessRoughnessAoBounceOne.Sample(PSss, input.texCoord).xyz;
    float3 metalnessRoughnessAoTwo = metalnessRoughnessAoBounceTwo.Sample(PSss, input.texCoord).xyz;
    float metalnessOne = metalnessRoughnessAoOne.x;
    float metalnessTwo = metalnessRoughnessAoTwo.x;
    float roughnessOne = metalnessRoughnessAoOne.y;
    float roughnessTwo = metalnessRoughnessAoTwo.y;
    float aoOne = metalnessRoughnessAoOne.z;
    float aoTwo = metalnessRoughnessAoTwo.z;

    float2 shadowAmount = shadows.Sample(PSss, input.texCoord).rg;
    float shadowOne = shadowAmount.r;
    float shadowTwo = shadowAmount.g;

    // Calculate world position for first bounce (stored a depth)
    float4 depthPosition = depthAndWorldPositions.Sample(PSss, input.texCoord);
    float linearDepth = depthPosition.x;

    float2 screenPos = input.texCoord * 2.0f - 1.0f;
	screenPos.y = -screenPos.y; // Invert Y for DirectX-style coordinates.
    float3 screenVS = mul(clipToView, float4(screenPos, 0.f, 1.0f)).xyz;
	float3 viewRay = float3(screenVS.xy / screenVS.z, 1.f);
	float4 vsPosition = float4(viewRay * linearDepth, 1.0f);
    float3 worldPositionOne = mul(viewToWorld, vsPosition).xyz;
    float3 worldPositionTwo = depthPosition.yzw;
    
    float3 invViewDirOne = cameraPosition - worldPositionOne;
    float3 invViewDirTwo = worldPositionOne - worldPositionTwo;

	float4 secondBounceColor = pbrShade(worldPositionTwo, worldNormalTwo, invViewDirTwo, albedoTwo, metalnessTwo, roughnessTwo, aoTwo, shadowTwo, -1.f);
    return pbrShade(worldPositionOne, worldNormalOne, invViewDirOne, albedoOne, metalnessOne, roughnessOne, aoOne, shadowOne, secondBounceColor);
    // return float4(shadowAmount, 0.f, 1.0f);
}

