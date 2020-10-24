// Compile to spirv:
// dxc -T lib_6_4 HardShadows.hlsl -spirv -Fo HardShadows-cs.spv -fvk-use-scalar-layout -fspv-target-env=vulkan1.2

#define HLSL
#include "dxr.shared"

// Inputs
[[vk::binding(0)]] RaytracingAccelerationStructure rtScene : register(t0);
[[vk::binding(1)]] ConstantBuffer<SceneCBuffer> RGSystemSceneBuffer : register(b0, space0);
[[vk::binding(2)]] Texture2D<float4> RGGbuffer_positions : register(t1);
[[vk::binding(3)]] Texture2D<float4> RGGbuffer_normals : register(t2);

// Outputs
[[vk::binding(4)]] RWTexture2D<float4> output : register(u0) : SAIL_NO_RESOURCE;

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid
inline void generateCameraRay(uint2 index, out float3 origin, out float3 direction) {
	float2 xy = index + 0.5f; // center in the middle of the pixel.
	float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

	// Invert Y for DirectX-style coordinates.
	screenPos.y = -screenPos.y;

	// Unproject the pixel coordinate into a ray.
	float4 world = mul(RGSystemSceneBuffer.projectionToWorld, float4(screenPos, 0, 1));

	world.xyz /= world.w;
	origin = RGSystemSceneBuffer.cameraPosition;
	direction = normalize(world.xyz - origin);
}

[shader("raygeneration")]
void RayGenMain() {
	uint2 launchIndex = DispatchRaysIndex().xy;

	// // Test, remove this
	// output[launchIndex] = 1.f;
	// output[launchIndex].rgb = RGSystemSceneBuffer.cameraPosition / 10.f;
	// // if ((launchIndex.x + launchIndex.y) % 2 == 0)
	// // 	output[launchIndex].r = 1.f;
	// return;

	// Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
	// generateCameraRay(launchIndex, ray.Origin, ray.Direction);

	float3 pixelWorldPos = mul(RGSystemSceneBuffer.viewToWorld, RGGbuffer_positions[launchIndex]).xyz;
	float3 pixelWorldNormal = RGGbuffer_normals[launchIndex].xyz;
    
	// Cast a shadow ray from the directional light
	RayDesc ray;
	ray.Direction = -RGSystemSceneBuffer.dirLightDirection;
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
void MissMain(inout ShadowRayPayload payload) {
    payload.isHit = false;
}

[shader("closesthit")]
void ClosestHitTriangleMain(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs) {
	payload.recursionDepth++;
	

}