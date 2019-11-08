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

    float3 worldNormal[2];
    worldNormal[0] = normalsBounceOne.Sample(PSss, input.texCoord).xyz * 2.f - 1.f;
    worldNormal[1] = normalsBounceTwo.Sample(PSss, input.texCoord).xyz * 2.f - 1.f;

    float3 albedo[2];
    albedo[0] = albedoBounceOne.Sample(PSss, input.texCoord).rgb;
    albedo[1] = albedoBounceTwo.Sample(PSss, input.texCoord).rgb;

    float3 metalnessRoughnessAoOne = metalnessRoughnessAoBounceOne.Sample(PSss, input.texCoord).xyz;
    float3 metalnessRoughnessAoTwo = metalnessRoughnessAoBounceTwo.Sample(PSss, input.texCoord).xyz;
    float metalness[2];
    metalness[0] = metalnessRoughnessAoOne.x;
    metalness[1] = metalnessRoughnessAoTwo.x;
    float roughness[2];
    roughness[0] = metalnessRoughnessAoOne.y;
    roughness[1] = metalnessRoughnessAoTwo.y;
    float ao[2];
    ao[0] = metalnessRoughnessAoOne.z;
    ao[1] = metalnessRoughnessAoTwo.z;

    // Calculate world position for first bounce (stored a depth)
    float4 depthPosition = depthAndWorldPositions.Sample(PSss, input.texCoord);
    float linearDepth = depthPosition.x;

    float2 screenPos = input.texCoord * 2.0f - 1.0f;
	screenPos.y = -screenPos.y; // Invert Y for DirectX-style coordinates.
    float3 screenVS = mul(clipToView, float4(screenPos, 0.f, 1.0f)).xyz;
	float3 viewRay = float3(screenVS.xy / screenVS.z, 1.f);
	float4 vsPosition = float4(viewRay * linearDepth, 1.0f);
	float3 worldPosition[2];
    worldPosition[0] = mul(viewToWorld, vsPosition).xyz;
    worldPosition[1] = depthPosition.yzw;
    
    float3 invViewDir[2];
    invViewDir[0] = cameraPosition - worldPosition[0];
    invViewDir[1] = worldPosition[0] - worldPosition[1];

	float4 secondBounceColor = pbrShade(worldPosition[1], worldNormal[1], invViewDir[1], albedo[1], metalness[1], roughness[1], ao[1], -1.f);
    return pbrShade(worldPosition[0], worldNormal[0], invViewDir[0], albedo[0], metalness[0], roughness[0], ao[0], secondBounceColor);
    // return 1.f;
}

