#define HLSL
#include "dxr.shared"

[shader("miss")]
void shadowMiss(inout ShadowRayPayload payload) {
    payload.isHit = false;
}