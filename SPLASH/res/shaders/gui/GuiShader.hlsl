
struct VSIn {
	float2 position : POSITION0;
	float2 texCoords : TEXCOORD0;
};

struct PSIn {
	float4 position : SV_Position;
	float2 texCoords : TEXCOORD0;
};

Texture2D sys_texDiffuse : register(t0);
SamplerState PSss : register(s0);

PSIn VSMain(VSIn input) {
	PSIn output;

	output.position.xy = input.position.xy;
	output.position.zw = 0;
	output.texCoords = input.texCoords;

	return output;
}

float4 PSMain(PSIn input) : SV_Target0{
	return sys_texDiffuse.Sample(PSss, input.texCoords);
}

