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

[numthreads(1, 1, 1)]
void CSMain(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
    static const float blendFactor = 10.0f;

	float2 texCoordScale = 0.25f; // TODO: read this from a cbuffer

    float4 finalColor = sourceTexture[dispatchThreadID.xy] + blendTexture[dispatchThreadID.xy * texCoordScale] * blendFactor;
	output[dispatchThreadID.xy] = float4(tonemap(finalColor.rgb), 1.0f);
}