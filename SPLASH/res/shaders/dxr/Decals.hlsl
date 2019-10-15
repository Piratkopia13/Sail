#define HLSL
#include "Common_hlsl_cpp.hlsl"

float4 renderDecal(uint index, float3 vsPosition, float3 wPos, float3 wNorm, float4 payloadColour) {
    DecalData currDecal = CB_DecalData.data[index];

    
    // Init temp test variables
    float3 decalSize = { 0.3f, 0.3f, 0.005f };
    float3x3 decalRot = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
    float3 decalPos = { 13.0f, 0.0f, 13.0f };

    // Test rotation
    float angle = 0.f;//PI / 2.f;
    float3 axis = { 1.0f, 0.0f, 0.0f };
    //decalRot = currDecal.rot;

    float3 pos = currDecal.position;
    float3x3 rot = decalRot;    
    float3 size = decalSize;

    //float3 ddx = ddx_coarse(vsPosition).xyz;
    float3 posNeighborX = vsPosition;

    //float3 ddy = ddy_coarse(vsPosition).xyz;
    float3 posNeighborY = vsPosition;

    // Apply projection, discard if outside its bb
    float3 localPos = wPos - pos;
    localPos = mul(localPos, rot);
    float3 decalUVW = localPos / size;
    decalUVW *= -1;

    float4 colourToReturn = 0.f;

    if (decalUVW.x >= -1.0f && decalUVW.x <= 1.0f &&
        decalUVW.y >= -1.0f && decalUVW.y <= 1.0f &&
        decalUVW.z >= -1.0f && decalUVW.z <= 1.0f)
    {

        // Get current decal textures
        float2 decalUV = saturate(decalUVW.xy * 0.5f + 0.5f);

        // Calculate gradient
        float3 decalPosNeighborX = posNeighborX - pos;
        decalPosNeighborX = mul(decalPosNeighborX, rot);
        decalPosNeighborX = decalPosNeighborX / size;
        decalPosNeighborX.y *= -1;
        float2 uvDX = saturate(decalPosNeighborX.xy * 0.5f + 0.5f) - decalUV;

        float3 decalPosNeighborY = posNeighborY - pos;
        decalPosNeighborY = mul(decalPosNeighborY, rot);
        decalPosNeighborY = decalPosNeighborY / size;
        decalPosNeighborY.y *= -1;
        float2 uvDY = saturate(decalPosNeighborY.xy * 0.5f + 0.5f) - decalUV;

        float4 albedoColour = decal_texAlbedo.SampleGrad(ss, decalUV, uvDX, uvDY);
        float3 decalNormalTS = decal_texNormal.SampleGrad(ss, decalUV, uvDX, uvDY);
        decalNormalTS = decalNormalTS * 2.0f - 1.0f;
        decalNormalTS.z *= -1.0f;
        float3 decalNormalWS = mul(decalNormalTS, currDecal.rot);
        float3 mra = decal_texMetalnessRoughnessAO.SampleGrad(ss, decalUV, uvDX, uvDY);
        float metalness = mra.r;
        float roughness = mra.g;
        float ao = mra.b;


        // Blend the decal properties with the material properties
        RayPayload decalPayload;
        decalPayload.color = 0.f;
        decalPayload.recursionDepth = 0;

        float3 norm = lerp(wNorm, decalNormalWS, albedoColour.w);
        shade(wPos, wNorm, payloadColour.rgb, 1.0f, 0.01f, 1.f, decalPayload);
        //colourToReturn = lerp(float4(albedo, 1.0f), decalPayload.color, albedoColour.w);
        colourToReturn = decalPayload.color;
        //colourToReturn = albedoColour;
    }

    return colourToReturn;
}