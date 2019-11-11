Texture2D sourceTexture : register(t0);
Texture2D blendTexture : register(t1);
RWTexture2D<float4> output : register(u10);

[numthreads(1, 1, 1)]
void CSMain(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
    static const float blendFactor = 1.0f;
	output[dispatchThreadID.xy] = sourceTexture[dispatchThreadID.xy] + blendTexture[dispatchThreadID.xy] * blendFactor;
}