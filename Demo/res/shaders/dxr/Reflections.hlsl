#define HLSL
#include "Reflections.shared"
#include "Utils.hlsl"

// Inputs
RaytracingAccelerationStructure rtScene : register(t0, space0);
ConstantBuffer<SceneCBuffer> systemSceneBuffer : register(b0, space0);
Texture2D<float4> gbuffer_positions : register(t1);
Texture2D<float4> gbuffer_normals : register(t2);

// BLAS specific
StructuredBuffer<InstanceData> instanceData : register(t0, space1);

StructuredBuffer<uint> indices          : register(t3, space0);
StructuredBuffer<float3> positions      : register(t3, space1);
StructuredBuffer<float2> texCoords      : register(t3, space2);
StructuredBuffer<float3> normals        : register(t3, space3);
StructuredBuffer<float3> tangents       : register(t3, space4);
StructuredBuffer<float3> bitangents     : register(t3, space5);

Texture2D<float4> sys_texAlbedo[] : register(t4, space0);
Texture2D<float4> sys_texNormal[] : register(t4, space1);
Texture2D<float4> sys_texMetalnessRoughnessAO[] : register(t4, space2);

SamplerState ss : register(s0);

// Outputs
RWTexture2D<float4> outputPositions : register(u0, space0);
RWTexture2D<float4> outputNormals 	: register(u1, space0);
RWTexture2D<float4> outputAlbedo 	: register(u2, space0);
RWTexture2D<float4> outputMRAO 	 	: register(u3, space0);

[shader("raygeneration")]
void rayGen() {
	uint2 launchIndex = DispatchRaysIndex().xy;

	float3 pixelWorldPos = mul(systemSceneBuffer.viewToWorld, gbuffer_positions[launchIndex]).xyz;
	float3 pixelWorldNormal = gbuffer_normals[launchIndex].xyz;
    
	// Cast a reflection ray
    // TODO: use roughness to vary reflection direction in a cone
	RayDesc ray;
	ray.Direction = reflect(pixelWorldPos - systemSceneBuffer.cameraPosition, pixelWorldNormal);
	ray.Origin = pixelWorldPos;
	ray.Origin += pixelWorldNormal * 0.1f; // Offset slightly to avoid self-hit
										   // This should preferably be done with the vertex normal and not a normal-mapped normal

	// Set TMin to a non-zero small value to avoid aliasing issues due to floating point errors
	// TMin should be kept small to prevent missing geometry at close contact areas
	ray.TMin = 0.00001;
	ray.TMax = 10000.0;

	RayPayload payload;
    payload.recursionDepth = 0;
	payload.albedo = float3(1.f, 0.f, 0.f);
	payload.normal = 0.f;
	payload.mrao = 0.f;
	payload.worldPos = 0.f;
    
	TraceRay(rtScene, 0, 0xFF, 0, 0, 0, ray, payload);

    // Output returned colors
    outputPositions[launchIndex] = float4(payload.worldPos, 1.0f);
    outputAlbedo[launchIndex] = float4(payload.albedo, 1.0f);
    outputNormals[launchIndex] = float4(payload.normal, 1.0f);
    outputMRAO[launchIndex] = float4(payload.mrao, 1.0f);
}

[shader("miss")]
void miss(inout RayPayload payload) {
    payload.albedo = 0.f;
}

[shader("closesthit")]
void closestHitTriangle(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs) {
	payload.recursionDepth++;
	
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    uint blasIndex = InstanceID() & ((1 << 16) - 1);
    uint instanceIndex = InstanceID() >> 16;
	uint primitiveID = PrimitiveIndex();
    InstanceData data = instanceData[instanceIndex];

    int verticesPerPrimitive = 3;
	uint i1 = primitiveID * verticesPerPrimitive;
	uint i2 = primitiveID * verticesPerPrimitive + 1;
	uint i3 = primitiveID * verticesPerPrimitive + 2;
	// Use indices if available
	if (data.flags & MESH_USE_INDICES) {
		i1 = indices[i1];
		i2 = indices[i2];
		i3 = indices[i3];
	}
	float3 positions1 = positions[i1]; float2 texCoord1 = texCoords[i1]; float3 normals1 = normals[i1]; float3 tangents1 = tangents[i1]; float3 bitangents1 = bitangents[i1];
	float3 positions2 = positions[i2]; float2 texCoord2 = texCoords[i2]; float3 normals2 = normals[i2]; float3 tangents2 = tangents[i2]; float3 bitangents2 = bitangents[i2];
	float3 positions3 = positions[i3]; float2 texCoord3 = texCoords[i3]; float3 normals3 = normals[i3]; float3 tangents3 = tangents[i3]; float3 bitangents3 = bitangents[i3];

    float2 texCoords = Utils::barrypolation(barycentrics, texCoord1, texCoord2, texCoord3);
    float3 position = Utils::barrypolation(barycentrics, positions1, positions2, positions3);
    
    // float3 normalInLocalSpace = cross(positions2 - positions1, vertex3.position - vertex1.position);
    float3 normalInLocalSpace = Utils::barrypolation(barycentrics, normals1, normals2, normals3);
    float3 normalInWorldSpace = normalize(mul(ObjectToWorld3x4(), float4(normalInLocalSpace, 0.f)));

	float3 tangentInLocalSpace = Utils::barrypolation(barycentrics, tangents1, tangents2, tangents3);
	float3 tangentInWorldSpace = normalize(mul(ObjectToWorld3x4(), float4(tangentInLocalSpace, 0.f)));

	float3 bitangentInLocalSpace = Utils::barrypolation(barycentrics, bitangents1, bitangents2, bitangents3);
	float3 bitangentInWorldSpace = normalize(mul(ObjectToWorld3x4(), float4(bitangentInLocalSpace, 0.f)));

	// Create TBN matrix to go from tangent space to world space
	float3x3 tbn = float3x3(
	  tangentInWorldSpace,
	  bitangentInWorldSpace,
	  normalInWorldSpace
	);
	payload.normal = normalInWorldSpace;
	if (data.flags & MESH_HAS_NORMAL_TEX) {
		float3 normalSample = sys_texNormal[instanceIndex].SampleLevel(ss, texCoords, 0).rgb;
        normalSample.y = 1.f - normalSample.y;
        normalSample.x = 1.f - normalSample.x;

        payload.normal = mul(normalize(normalSample * 2.f - 1.f), tbn);
	}

    payload.albedo = data.color.rgb;
	if (data.flags & MESH_HAS_ALBEDO_TEX)
		payload.albedo = sys_texAlbedo[instanceIndex].SampleLevel(ss, texCoords, 0).rgb;
	
	payload.mrao = float3(data.metalnessScale, data.roughnessScale, data.aoIntensity);
	if (data.flags & MESH_HAS_MRAO_TEX) {
		float3 mraoSample = sys_texMetalnessRoughnessAO[instanceIndex].SampleLevel(ss, texCoords, 0).rgb;
		payload.mrao.r *= mraoSample.r;
		payload.mrao.g *= 1.f - mraoSample.g; // Invert roughness from texture to make it correct
		payload.mrao.b += mraoSample.b;
	}
	payload.worldPos = Utils::HitWorldPosition();
}