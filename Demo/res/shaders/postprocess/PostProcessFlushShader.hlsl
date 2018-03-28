struct VSIn
{
	float4 position : POSITION0;
};

struct PSIn
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
};

PSIn VSMain(VSIn input)
{
	PSIn output;

	input.position.w = 1.f;
	// input position is already in clip space coordinates
	output.position = input.position;
	output.texCoord.x = input.position.x / 2.f + 0.5f;
	output.texCoord.y = -input.position.y / 2.f + 0.5f;

	return output;

}

Texture2D tex;
SamplerState ss : register(s0);

float4 PSMain(PSIn input) : SV_Target0
{
	//float2 texCoords = input.texCoord / 2.f + 0.5f;
	return tex.Sample(ss, input.texCoord).rgba;
}

