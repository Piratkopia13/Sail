#define HLSL
#include "dxr.shared"

// Inputs
RaytracingAccelerationStructure rtScene : register(t0);
ConstantBuffer<SceneCBuffer> systemSceneBuffer : register(b0, space0);
Texture2D<float4> gbuffer_positions : register(t1);
Texture2D<float4> gbuffer_normals : register(t2);

// Outputs
RWTexture2D<float4> output : register(u0);

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid
inline void generateCameraRay(uint2 index, out float3 origin, out float3 direction) {
	float2 xy = index + 0.5f; // center in the middle of the pixel.
	float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

	// Invert Y for DirectX-style coordinates.
	screenPos.y = -screenPos.y;

	// Unproject the pixel coordinate into a ray.
	float4 world = mul(systemSceneBuffer.projectionToWorld, float4(screenPos, 0, 1));

	world.xyz /= world.w;
	origin = systemSceneBuffer.cameraPosition;
	direction = normalize(world.xyz - origin);
}

[shader("raygeneration")]
void rayGen() {
	uint2 launchIndex = DispatchRaysIndex().xy;

	// Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
	// generateCameraRay(launchIndex, ray.Origin, ray.Direction);

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