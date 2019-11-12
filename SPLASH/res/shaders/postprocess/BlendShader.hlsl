Texture2D sourceTexture : register(t0);
Texture2D blendTexture : register(t1);
RWTexture2D<float4> output : register(u10);

float3 tonemap(float3 color) {
	// Gamma correction
    float3 output = color / (color + 1.0f);
    // Tone mapping using the Reinhard operator
    output = pow(output, 1.0f / 2.2f);
	return output;
}

cbuffer CSData : register(b0) {
    float textureSizeDifference;
    uint2 textureSize;
}

#define BLOCK_SIZE 256
[numthreads(BLOCK_SIZE, 1, 1)]
void CSMain(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x > textureSize.x) {
        return;
    }

    static const float blendFactor = 0.2f;

    float3 finalColor = sourceTexture[dispatchThreadID.xy].rgb + blendTexture[dispatchThreadID.xy * textureSizeDifference].rgb * blendFactor;
	output[dispatchThreadID.xy] = float4(tonemap(finalColor), 1.0f);
}