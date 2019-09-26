struct VSIn
{
	float4 position : POSITION;
	float4 color : COLOR;
};


struct PSIn
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float4 modelColor : COLOR1;
};

cbuffer ModelData
{
	float4 modelColor;
	matrix mMVP;
}

PSIn VSMain(VSIn input)
{
	PSIn output;

	input.position.w = 1.0f;

	//output.position = input.position;
	output.position = mul(input.position, mMVP);
	
	output.color = input.color;
	output.modelColor = modelColor;

	return output;
}


float4 PSMain(PSIn input) : SV_TARGET
{
	return input.color * input.modelColor;
}