#define HLSL
#include "dxr.shared"

[shader("miss")]
void ShadowMissMain(inout ShadowRayPayload payload) {
    payload.isHit = false;
}