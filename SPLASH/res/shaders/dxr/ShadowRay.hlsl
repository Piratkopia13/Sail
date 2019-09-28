#define HLSL
#include "Common_hlsl_cpp.hlsl"

[shader("closesthit")]
void shadowClosestHit(inout ShadowRayPayload payload, in BuiltInTriangleIntersectionAttributes attribs) {
    payload.isHit = true;
}

[shader("miss")]
void shadowMiss(inout ShadowRayPayload payload : SV_RayPayload) {
    payload.isHit = false;
}