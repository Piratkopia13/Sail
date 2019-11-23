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

#include "PBR.hlsl"

cbuffer PSSceneCBuffer : register(b0) {
    float3 cameraPosition;
    float padding;
    PointlightInput pointLights[NUM_POINT_LIGHTS];
    SpotlightInput spotLights[NUM_POINT_LIGHTS];
    IndexMap shadowTextureIndexMap[NUM_POINT_LIGHTS*2]; // Maps light indices to shadow texture indices
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


float4 PSMain(PSIn input) : SV_Target0 {

    PBRPixel pixelOne;
    PBRPixel pixelTwo;

    pixelOne.worldNormal = normalsBounceOne.Sample(PSss, input.texCoord).xyz * 2.f - 1.f;
    pixelTwo.worldNormal = normalsBounceTwo.Sample(PSss, input.texCoord).xyz * 2.f - 1.f;

    pixelOne.albedo = albedoBounceOne.Sample(PSss, input.texCoord).rgb;
    pixelTwo.albedo = albedoBounceTwo.Sample(PSss, input.texCoord).rgb;

    float4 metalnessRoughnessAoOne = metalnessRoughnessAoBounceOne.Sample(PSss, input.texCoord);
    float4 metalnessRoughnessAoTwo = metalnessRoughnessAoBounceTwo.Sample(PSss, input.texCoord);
    pixelOne.metalness = metalnessRoughnessAoOne.r;
    pixelTwo.metalness = metalnessRoughnessAoTwo.r;
    pixelOne.roughness = metalnessRoughnessAoOne.g;
    pixelTwo.roughness = metalnessRoughnessAoTwo.g;
    pixelOne.ao = metalnessRoughnessAoOne.b;
    pixelTwo.ao = metalnessRoughnessAoTwo.b;
    pixelOne.emissiveness = pow(1 - metalnessRoughnessAoOne.a, 2);
    pixelTwo.emissiveness = pow(1 - metalnessRoughnessAoTwo.a, 2);

    float shadowOne[NUM_SHADOW_TEXTURES];
    float shadowTwo[NUM_SHADOW_TEXTURES];
    for (uint i = 0; i < NUM_SHADOW_TEXTURES; i++) {
        float2 shadowAmount = shadows.Sample(PSss, float3(input.texCoord, i)).rg; // z parameter in texCoords is the array index
        shadowOne[i] = shadowAmount.r;
        shadowTwo[i] = shadowAmount.g;
    }

    // Calculate world position for first bounce (stored a depth)
    pixelOne.worldPosition = worldPositionsOne.Sample(PSss, input.texCoord).xyz;
    pixelTwo.worldPosition = worldPositionsTwo.Sample(PSss, input.texCoord).xyz;
    
    pixelOne.invViewDir = cameraPosition - pixelOne.worldPosition;
    pixelTwo.invViewDir = pixelOne.worldPosition - pixelTwo.worldPosition;

    PBRScene scene;
    // Shade the reflection
    scene.pointLights = pointLights;
    scene.spotLights = spotLights;
    scene.brdfLUT = brdfLUT;
    scene.sampler = PSss;
    scene.shadow = shadowTwo;
    scene.shadowTextureIndexMap = shadowTextureIndexMap;
	float4 secondBounceColor = pbrShade(scene, pixelTwo, -1.f);

    // Shade the first hit
    scene.shadow = shadowOne;
    return pbrShade(scene, pixelOne, secondBounceColor.rgb);
    
    // Debug stuff
    // return float4(albedoOne, 1.0f);
    // return float4(worldNormalOne, 1.0f);
    // return float4(shadowTwo[0], 0.f, 0.f, 1.0f);
    // return float4(shadowAmount.x, 0.f, 0.f, 1.0f) * 0.5 + pbrShade(worldPositionOne, worldNormalOne, invViewDirOne, albedoOne, metalnessOne, roughnessOne, aoOne, shadowOne, secondBounceColor) * 0.5;
}

