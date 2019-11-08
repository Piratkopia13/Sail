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

Texture2D<float4> metalnesRoughnessAoBounceOne : register(t4);
Texture2D<float4> metalnesRoughnessAoBounceTwo : register(t5);

Texture2D<float2> shadows : register(t6);

StructuredBuffer<uint> waterData : register(t6, space0);

SamplerState ss : register(s0);

float4 PSMain(PSIn input) : SV_Target0 {
	return colorTex.Sample(ss, input.texCoord) + blendTex.Sample(ss, input.texCoord) * blendFactor;
}

