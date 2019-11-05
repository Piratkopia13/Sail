//=============================================================================
// Highly inspired by Frank Luna
//
// Performs a separable blur with a blur radius of 5.  
//=============================================================================
#include "BilateralBlurCommon.hlsl"

Texture2D input : register(t0);
RWTexture2D<float4> output : register(u10);

[numthreads(N, 1, 1)]
void CSMain(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//
	
	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if(groupThreadID.x < blurRadius) {
		// Clamp out of bound samples that occur at image borders.
		int x = max(dispatchThreadID.x - blurRadius, 0);
		cache[groupThreadID.x] = input[int2(x, dispatchThreadID.y)];
		// cache[groupThreadID.x] = float4(0, 0.5, 0, 1);
	}
	if(groupThreadID.x >= N-blurRadius) {
		// Clamp out of bound samples that occur at image borders.
		int x = min(dispatchThreadID.x + blurRadius, input.Length.x-1);
		cache[groupThreadID.x+2*blurRadius] = input[int2(x, dispatchThreadID.y)];
		// cache[groupThreadID.x] = float4(0, 0.5, 0, 1);
	}

	// Clamp out of bound samples that occur at image borders.
	cache[groupThreadID.x+blurRadius] = input[min(dispatchThreadID.xy, input.Length.xy-1)];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//
	// Now blur each pixel.
	//

    float factor;
    float bZ = 1.0f / normpdf(0.0f, BSIGMA);
	float Z = 0.f;
    float3 blurColor = 0.f;
	
	[unroll]
	for(int i = -blurRadius; i <= blurRadius; ++i) {
		int k = groupThreadID.x + blurRadius + i;
		
        factor = normpdf(cache[k].r-cache[groupThreadID.x].r, BSIGMA) * bZ * weights[i+blurRadius];
        Z += factor;
        blurColor += factor*cache[k];
	}
	
	output[dispatchThreadID.xy] = float4(blurColor.r / Z, 0.f, 0.f, 1.0f);
    // output[dispatchThreadID.xy] = cache[groupThreadID.x];
}