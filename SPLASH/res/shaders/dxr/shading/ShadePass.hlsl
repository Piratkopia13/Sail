#define HLSL
#include "../Common_hlsl_cpp.hlsl"
#include "../Utils.hlsl"

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

Texture2DArray<float2> shadows : register(t6);

Texture2D<float4> worldPositionsOne : register(t7);
Texture2D<float4> worldPositionsTwo : register(t8);

Texture2D<float4> brdfLUT : register(t9);

SamplerState PSss : register(s0);

#include "PBR.hlsl"

float4 PSMain(PSIn input) : SV_Target0 {

    float3 worldNormalOne = normalsBounceOne.Sample(PSss, input.texCoord).xyz * 2.f - 1.f;
    float3 worldNormalTwo = normalsBounceTwo.Sample(PSss, input.texCoord).xyz * 2.f - 1.f;

    float3 albedoOne = albedoBounceOne.Sample(PSss, input.texCoord).rgb;
    float3 albedoTwo = albedoBounceTwo.Sample(PSss, input.texCoord).rgb;

    float4 metalnessRoughnessAoOne = metalnessRoughnessAoBounceOne.Sample(PSss, input.texCoord);
    float4 metalnessRoughnessAoTwo = metalnessRoughnessAoBounceTwo.Sample(PSss, input.texCoord);
    float metalnessOne = metalnessRoughnessAoOne.r;
    float metalnessTwo = metalnessRoughnessAoTwo.r;
    float roughnessOne = metalnessRoughnessAoOne.g;
    float roughnessTwo = metalnessRoughnessAoTwo.g;
    float aoOne = metalnessRoughnessAoOne.b;
    float aoTwo = metalnessRoughnessAoTwo.b;
    float emissivenessOne = pow(1 - metalnessRoughnessAoOne.a, 2);
    float emissivenessTwo = pow(1 - metalnessRoughnessAoTwo.a, 2);

	float shadowOne[NUM_SHADOW_TEXTURES];
    float shadowTwo[NUM_SHADOW_TEXTURES];
    for (uint i = 0; i < NUM_SHADOW_TEXTURES; i++) {
        float2 shadowAmount = shadows.Sample(PSss, float3(input.texCoord, i)).rg; // z parameter in texCoords is the array index
        shadowOne[i] = shadowAmount.r;
        shadowTwo[i] = shadowAmount.g;
    }
    // float shadowOne = shadowAmount.r;
    // float shadowTwo = shadowAmount.g;

    // Calculate world position for first bounce (stored a depth)
    float3 worldPositionOne = worldPositionsOne.Sample(PSss, input.texCoord).xyz;
    float3 worldPositionTwo = worldPositionsTwo.Sample(PSss, input.texCoord).xyz;
    
    float3 invViewDirOne = cameraPosition - worldPositionOne;
    float3 invViewDirTwo = worldPositionOne - worldPositionTwo;

	float4 secondBounceColor = pbrShade(worldPositionTwo, worldNormalTwo, invViewDirTwo, albedoTwo, emissivenessTwo, metalnessTwo, roughnessTwo, aoTwo, shadowTwo, -1.f);
    return pbrShade(worldPositionOne, worldNormalOne, invViewDirOne, albedoOne, emissivenessOne, metalnessOne, roughnessOne, aoOne, shadowOne, secondBounceColor.rgb);
    
    // Debug stuff
    // return float4(metalnessRoughnessAoOne, 1.0f);
    // return float4(worldNormalOne, 1.0f);
    // return float4(shadowOne[0], 0.f, 0.f, 1.0f);
    // return float4(shadowAmount.x, 0.f, 0.f, 1.0f) * 0.5 + pbrShade(worldPositionOne, worldNormalOne, invViewDirOne, albedoOne, metalnessOne, roughnessOne, aoOne, shadowOne, secondBounceColor) * 0.5;
}

