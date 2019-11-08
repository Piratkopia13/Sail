struct VSIn {
	float4 position : POSITION0;
};

struct PSIn {
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
};

PSIn VSMain(VSIn input) {
	PSIn output;

	input.position.w = 1.f;
	// input position is already in clip space coordinates
	output.position = input.position;
	output.texCoord.x = input.position.x / 2.f + 0.5f;
	output.texCoord.y = -input.position.y / 2.f + 0.5f;

	return output;

}

Texture2D<float4> albedoBounceOne : register(t0);
Texture2D<float4> albedoBounceTwo : register(t1);

Texture2D<float4> normalsBounceOne : register(t2);
Texture2D<float4> normalsBounceTwo : register(t3);

Texture2D<float4> metalnessRoughnessAoBounceOne : register(t4);
Texture2D<float4> metalnessRoughnessAoBounceTwo : register(t5);

Texture2D<float2> shadows : register(t6);

// StructuredBuffer<uint> PSwaterData : register(t7);

SamplerState PSss : register(s0);

float4 PSMain(PSIn input) : SV_Target0 {
    float4 testColor = albedoBounceOne.Sample(PSss, input.texCoord) * 0.8f;
    testColor += albedoBounceTwo.Sample(PSss, input.texCoord) * 0.2f;
    
    // Just making sure the texture are bound
    testColor += normalsBounceOne.Sample(PSss, input.texCoord) * 0.0001f;
    testColor += normalsBounceTwo.Sample(PSss, input.texCoord) * 0.0001f;
    testColor += metalnessRoughnessAoBounceOne.Sample(PSss, input.texCoord) * 0.0001f;
    testColor += metalnessRoughnessAoBounceTwo.Sample(PSss, input.texCoord) * 0.0001f;
    testColor.xy += shadows.Sample(PSss, input.texCoord) * 0.0001f;

	return testColor;
}

