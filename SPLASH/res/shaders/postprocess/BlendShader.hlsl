Texture2D sourceTexture : register(t0);
Texture2D blendTexture : register(t1);
RWTexture2D<float4> output : register(u10);

SamplerState CSss : register(s2);

float3 tonemap(float3 color) {
	// Gamma correction
    float3 output = color / (color + 1.0f);
    // Tone mapping using the Reinhard operator
    output = pow(output, 1.0f / 2.2f);
	return output;
}

cbuffer CSData : register(b0) {
    float textureSizeDifference;
}

[numthreads(1, 1, 1)]
void CSMain(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
    static const float blendFactor = 0.2f;

    float2 blendTextureSize;
    blendTexture.GetDimensions(blendTextureSize.x, blendTextureSize.y);

    float4 finalColor = sourceTexture[dispatchThreadID.xy] + blendTexture.SampleLevel(CSss, (dispatchThreadID.xy / blendTextureSize) * textureSizeDifference, 0) * blendFactor;
	output[dispatchThreadID.xy] = float4(tonemap(finalColor.rgb), 1.0f);
}