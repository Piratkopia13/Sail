struct VSIn {
	float4 position : POSITION0;
};

struct PSIn {
	float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

cbuffer PSSystemCBuffer : register(b0) {
    matrix sys_mView;
    matrix sys_mProjection;
    float4 kernel[64];
    float4 noise[4][4];
    float2 windowSize;
}

PSIn VSMain(VSIn input) {
	PSIn output;

	input.position.w = 1.f;
	output.position = input.position;
    output.texCoord = input.position.xy * 0.5f + 0.5f;
    output.texCoord.y = 1.f - output.texCoord.y; // Flip y-coord cuz direct x

	return output;
}

Texture2D def_positions       : register(t1);
Texture2D def_worldNormals    : register(t2);
SamplerState PSssNearestClamp : register(s3) : SAIL_SAMPLER_POINT_CLAMP; // Use to sample positions (s3)

float PSMain(PSIn input) : SV_Target0 {
    float3 fragPos      = def_positions.Sample(PSssNearestClamp, input.texCoord).xyz;
    float3 worldNormal  = def_worldNormals.Sample(PSssNearestClamp, input.texCoord).xyz;
    float3 vsNormal     = mul(sys_mView, float4(normalize(worldNormal), 0.f)).xyz; // should be changed to inverse transpose to support non-uniformly scaled objects
    float3 randomVec    = noise[(input.texCoord.x * windowSize.x) % 4][(input.texCoord.y * windowSize.y) % 4].xyz; // Tile noise texture over screen based on screen dimensions modulus noise size

    // return fragPos.x;
    // return vsNormal.y;
    // return randomVec.x * 0.5f + 0.5f;

    float3 tangent   = normalize(randomVec - vsNormal * dot(randomVec, vsNormal));
    float3 bitangent = cross(vsNormal, tangent);
    float3x3 TBN     = float3x3(tangent, bitangent, vsNormal);

    static const int kernelSize = 64;
    static const float radius = 0.5f;
    static const float bias = 0.0f; //0.025

    float occlusion = 0.f;
    // [unroll]
    for(int i = 0; i < kernelSize; i++) {
        // Get sample position
        float3 smpl = mul(kernel[i].xyz, TBN); // From tangent to view-space
        smpl = fragPos + smpl * radius;
        
        float4 offset = float4(smpl, 1.0f);
        offset        = mul(sys_mProjection, offset);  // View to clip-space
        offset.xyz   /= offset.w;                      // Perspective divide
        offset.xyz    = offset.xyz * 0.5f + 0.5f;      // Transform to range 0 - 1

        offset.y = 1.f - offset.y; // Flip y-coord cuz direct x
        float sampleDepth = def_positions.Sample(PSssNearestClamp, offset.xy).z;
        
        float rangeCheck = smoothstep(0.f, 1.f, radius / abs(fragPos.z - sampleDepth));
        occlusion       += (sampleDepth <= smpl.z + bias ? 1.f : 0.f) * rangeCheck;
    }
    occlusion = 1.f - (occlusion / kernelSize);
    return occlusion;
}

