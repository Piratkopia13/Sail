struct VSIn {
	float4 position : POSITION;
	float2 texCoord : TEXCOORD0;
};

struct PSIn {
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
	float4 modelColor : COLOR;
	float clip : SV_ClipDistance0;
};

cbuffer ModelData : register(b0) {
	float4 modelColor;
	matrix mWorld;
	matrix mVP;
}
cbuffer ClippingPlaneBuffer : register(b1) {
	float4 clippingPlane;
}

PSIn VSMain(VSIn input) {

	PSIn output;

	input.position.w = 1;
	output.position = mul(input.position, mWorld);

	// Calculate the distance from the vertex to the clipping plane
	// This needs to be done with world coordinates
	output.clip = dot(output.position, clippingPlane);

	output.position = mul(output.position, mVP);
	output.texCoord = input.texCoord;
	output.modelColor = modelColor;

	return output;

}


Texture2D tex;
SamplerState ss;

float4 PSMain(PSIn input) : SV_TARGET {

	return tex.Sample(ss, input.texCoord) * input.modelColor;
	//return input.modelColor;

}