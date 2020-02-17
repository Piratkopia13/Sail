//=============================================================================
// Highly inspired by Frank Luna
//
// Performs a separable blur with a blur radius defined in included file.  
//=============================================================================
#include "GaussianBlurCommon.hlsl"

RWTexture2D<float4> inoutput : register(u10) : SAIL_IGNORE;

cbuffer CSData : register(b0) {
    float textureSizeDifference;
}

[numthreads(N, 1, 1)]
void CSMain(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

	float2 inputSize;
	inoutput.GetDimensions(inputSize.x, inputSize.y);

	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if(groupThreadID.x < blurRadius) {
		// Clamp out of bound samples that occur at image borders.
		int x = max(dispatchThreadID.x - blurRadius, 0);
		cache[groupThreadID.x] = inoutput[int2(x, dispatchThreadID.y) * textureSizeDifference];
		// cache[groupThreadID.x] = float4(0, 0.5, 0, 1);
	}
	if(groupThreadID.x >= N-blurRadius) {
		// Clamp out of bound samples that occur at image borders.
		int x = min(dispatchThreadID.x + blurRadius, inputSize.x-1);
		cache[groupThreadID.x+2*blurRadius] = inoutput[int2(x, dispatchThreadID.y) * textureSizeDifference];
		// cache[groupThreadID.x] = float4(0, 0.5, 0, 1);
	}

	// Clamp out of bound samples that occur at image borders.
	cache[groupThreadID.x+blurRadius] = inoutput[min(dispatchThreadID.xy, inputSize.xy-1) * textureSizeDifference];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//
	// Now blur each pixel.
	//

	float4 blurColor = float4(0, 0, 0, 0);
	
	[unroll]
	for(int i = -blurRadius; i <= blurRadius; ++i) {
		int k = groupThreadID.x + blurRadius + i;
		
		blurColor += weights[i+blurRadius] * cache[k];
	}
	
	inoutput[dispatchThreadID.xy] = blurColor;
}