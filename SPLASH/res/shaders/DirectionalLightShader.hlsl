struct VSIn {
	float4 position : POSITION;
	float3 normal : NORMAL0;
};


struct PSIn {
	float4 position : SV_POSITION;
	float3 normal : NORMAL0;
	float4 modelColor : COLOR1;
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

	input.position.w = 1.0f;
	output.position = mul(input.position, mWorld);

	// Calculate the distance from the vertex to the clipping plane
	// This needs to be done with world coordinates
	output.clip = dot(output.position, clippingPlane);

	output.position = mul(output.position, mVP);
	
	output.normal = mul(input.normal, (float3x3)mWorld);
    output.normal = normalize(output.normal);
	output.modelColor = modelColor;

	return output;
}


float4 PSMain(PSIn input) : SV_TARGET {

	float3 lightDir = float3(-0.44f, -0.17f, 0.176f);
	lightDir = normalize(lightDir);

	float3 diffuse = saturate(dot(input.normal, -lightDir)) * input.modelColor.xyz;
	float3 ambient = float3(.1f, .1f, .1f) * input.modelColor.xyz;

	return float4(diffuse + ambient, 1.f);
	//return float4((input.normal + 1.0f) / 2.f, 1.0f);
}