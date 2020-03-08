#define HLSL
#include "HardShadows.shared"

// Inputs
RaytracingAccelerationStructure rtScene : register(t0);
ConstantBuffer<SceneCBuffer> systemSceneBuffer : register(b0, space0);
Texture2D<float4> gbuffer_positions : register(t1);
Texture2D<float4> gbuffer_normals : register(t2);

// Outputs
RWTexture2D<float4> output : register(u0);

[shader("raygeneration")]
void rayGen() {
	uint2 launchIndex = DispatchRaysIndex().xy;

	float3 pixelWorldPos = mul(systemSceneBuffer.viewToWorld, gbuffer_positions[launchIndex]).xyz;
	float3 pixelWorldNormal = gbuffer_normals[launchIndex].xyz;
    
	// Cast a shadow ray from the directional light
	RayDesc ray;
	ray.Direction = -systemSceneBuffer.dirLightDirection;
	ray.Origin = pixelWorldPos;
	ray.Origin += pixelWorldNormal * 0.1f; // Offset slightly to avoid self-shadowing
										   // This should preferably be done with the vertex normal and not a normal-mapped normal

	// Set TMin to a non-zero small value to avoid aliasing issues due to floating point errors
	// TMin should be kept small to prevent missing geometry at close contact areas
	ray.TMin = 0.00001;
	ray.TMax = 10000.0;

	ShadowRayPayload payload;
    payload.isHit = true; // Assume hit, a missed ray will change this to false
    
	uint missShaderIndex = 1; // Shadow miss group
	uint hitGroup = 1; // Null hit group
	TraceRay(rtScene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, hitGroup, 0, missShaderIndex, ray, payload);

    // Output black on hit pixels
    output[launchIndex].rgb = 1.f - (float)payload.isHit;
	output[launchIndex].a = 1.0f;
}

[shader("miss")]
void miss(inout RayPayload payload) {

}

[shader("closesthit")]
void closestHitTriangle(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs) {
	payload.recursionDepth++;

}