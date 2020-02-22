#define HLSL
#include "dxr.shared"

RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> output : register(u0);
ConstantBuffer<SceneCBuffer> SystemSceneBuffer : register(b0, space0);

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid
inline void generateCameraRay(uint2 index, out float3 origin, out float3 direction) {
	float2 xy = index + 0.5f; // center in the middle of the pixel.
	float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

	// Invert Y for DirectX-style coordinates.
	screenPos.y = -screenPos.y;

	// Unproject the pixel coordinate into a ray.
	float4 world = mul(SystemSceneBuffer.projectionToWorld, float4(screenPos, 0, 1));

	world.xyz /= world.w;
	origin = SystemSceneBuffer.cameraPosition;
	direction = normalize(world.xyz - origin);
}

[shader("raygeneration")]
void rayGen() {
	uint2 launchIndex = DispatchRaysIndex().xy;

    RayDesc ray;
	// Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
	generateCameraRay(launchIndex, ray.Origin, ray.Direction);

	// Set TMin to a non-zero small value to avoid aliasing issues due to floating point errors
	// TMin should be kept small to prevent missing geometry at close contact areas
	ray.TMin = 0.00001;
	ray.TMax = 10000.0;

	ShadowRayPayload payload;
    payload.isHit = true; // Assume hit, a missed ray will change this to false
    uint missShaderIndex = 1; // Shadow miss group
	TraceRay(gRtScene, 0, 0xFF, 0, 0, missShaderIndex, ray, payload);

    // Output black on hit pixels
    output[launchIndex] = (float)payload.isHit;
}

[shader("miss")]
void miss(inout RayPayload payload) {

}

[shader("closesthit")]
void closestHitTriangle(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs) {
	payload.recursionDepth++;
	

}