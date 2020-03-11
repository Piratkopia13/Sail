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
// Texture2D<float4> sys_texNormal[] : register(t4, space1);
// Texture2D<float4> sys_texMetalnessRoughnessAO[] : register(t4, space2);

SamplerState ss : register(s0);

// Outputs
RWTexture2D<float4> output : register(u0);

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
    
	TraceRay(rtScene, 0, 0xFF, 0, 0, 0, ray, payload);

    // Output returned color
    output[launchIndex].rgb = payload.color;
	output[launchIndex].a = 1.0f;
}

[shader("miss")]
void miss(inout RayPayload payload) {
    payload.color = 0.f;
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
	float3 positions1 = positions[i1]; float2 texCoord1 = texCoords[i1]; float3 normals1 = normals[i1];
	float3 positions2 = positions[i2]; float2 texCoord2 = texCoords[i2]; float3 normals2 = normals[i2];
	float3 positions3 = positions[i3]; float2 texCoord3 = texCoords[i3]; float3 normals3 = normals[i3];

    float2 texCoords = Utils::barrypolation(barycentrics, texCoord1, texCoord2, texCoord3);
    float3 position = Utils::barrypolation(barycentrics, positions1, positions2, positions3);
    
    // float3 normalInLocalSpace = cross(positions2 - positions1, vertex3.position - vertex1.position);
    float3 normalInLocalSpace = Utils::barrypolation(barycentrics, normals1, normals2, normals3);
    normalInLocalSpace = normalize(normalInLocalSpace);
    float3 normalInWorldSpace = normalize(mul(ObjectToWorld3x4(), float4(normalInLocalSpace, 0.f)));

    // payload.color = position;
    // payload.color *= normalInWorldSpace;
    // payload.color *= float3(texCoords, 0.f);

    float4 color = float4(data.color.rgb, 0.f);
	if (data.flags & MESH_HAS_ALBEDO_TEX)
		color = sys_texAlbedo[instanceIndex].SampleLevel(ss, texCoords, 0);

    payload.color = color.rgb;
    // if (meshData[blasIndex].flags & MESH_USE_INDICES)
        // payload.color = 1.f;
    // payload.color = float3(attribs.barycentrics, 0.f);
}