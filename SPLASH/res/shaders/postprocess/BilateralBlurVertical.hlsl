//=============================================================================
// Highly inspired by Frank Luna
//
// Performs a separable blur with a blur radius of 5.  
//=============================================================================
#include "BilateralBlurCommon.hlsl"

Texture2D input : register(t0);
RWTexture2D<float4> output : register(u10);

[numthreads(1, N, 1)]
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
	if(groupThreadID.y < blurRadius) {
		// Clamp out of bound samples that occur at image borders.
		int y = max(dispatchThreadID.y - blurRadius, 0);
		cache[groupThreadID.y] = input[int2(dispatchThreadID.x, y)];
	}
	if(groupThreadID.y >= N-blurRadius) {
		// Clamp out of bound samples that occur at image borders.
		int y = min(dispatchThreadID.y + blurRadius, input.Length.y-1);
		cache[groupThreadID.y+2*blurRadius] = input[int2(dispatchThreadID.x, y)];
	}
	
	// Clamp out of bound samples that occur at image borders.
	cache[groupThreadID.y+blurRadius] = input[min(dispatchThreadID.xy, input.Length.xy-1)];


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
		int k = groupThreadID.y + blurRadius + i;
		
        factor = normpdf3(cache[k].rgb-cache[groupThreadID.y].rgb, BSIGMA) * bZ * weights[i+blurRadius];
        Z += factor;
        blurColor += factor*cache[k];
	}
	
	output[dispatchThreadID.xy] = float4(blurColor / Z, 1.0f);	
	// output[dispatchThreadID.xy] = cache[groupThreadID.y];
}