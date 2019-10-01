#define HLSL
#include "Common_hlsl_cpp.hlsl"

[shader("miss")]
void shadowMiss(inout ShadowRayPayload payload) {
    payload.isHit = false;
}