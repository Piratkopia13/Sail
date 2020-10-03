//=============================================================================
// Highly inspired by Frank Luna
//
// Performs a separable blur with a blur radius defined in included file.  
//=============================================================================
#include "GaussianBlurCommon.hlsl"

Texture2D input : register(t0);
RWTexture2D<unorm float4> output : register(u10) : SAIL_R8_UNORM;

[[vk::push_constant]]
struct {
	float textureSizeDifference;
} CSData;
// cbuffer CSData : register(b1) {
//     float textureSizeDifference;
// }

[numthreads(1, N, 1)]
void CSMain(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

	float2 inputSize;
	input.GetDimensions(inputSize.x, inputSize.y);
	
	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if(groupThreadID.y < blurRadius) {
		// Clamp out of bound samples that occur at image borders.
		int y = max(dispatchThreadID.y - blurRadius, 0);
		cache[groupThreadID.y] = input[int2(dispatchThreadID.x, y) * CSData.textureSizeDifference];
	}
	if(groupThreadID.y >= N-blurRadius) {
		// Clamp out of bound samples that occur at image borders.
		int y = min(dispatchThreadID.y + blurRadius, inputSize.y-1);
		cache[groupThreadID.y+2*blurRadius] = input[int2(dispatchThreadID.x, y) * CSData.textureSizeDifference];
	}
	
	// Clamp out of bound samples that occur at image borders.
	cache[groupThreadID.y+blurRadius] = input[min(dispatchThreadID.xy, inputSize.xy-1) * CSData.textureSizeDifference];


	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//
	// Now blur each pixel.
	//

	float4 blurColor = float4(0, 0, 0, 0);
	
	[unroll]
	for(int i = -blurRadius; i <= blurRadius; ++i) {
		int k = groupThreadID.y + blurRadius + i;
		
		blurColor += weights[i+blurRadius]*cache[k];
	}
	
	output[dispatchThreadID.xy] = blurColor;
}