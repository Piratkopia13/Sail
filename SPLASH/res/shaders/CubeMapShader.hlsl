struct VSIn {
	float4 position : POSITION;
};

struct PSIn {
	float4 position : SV_POSITION;
	float clip : SV_ClipDistance0;
	float3 camToFrag : CTF;
};

cbuffer ModelData : register(b0) {
	matrix mWorld;
	matrix mVP;
}
cbuffer WorldData : register(b1) {
	float4 clippingPlane;
	float3 cameraPos;
}

PSIn VSMain(VSIn input) {
	PSIn output;

	input.position.w = 1.0f;
	output.position = mul(input.position, mWorld);

	// Calculate the distance from the vertex to the clipping plane
	// This needs to be done with world coordinates
	output.clip = dot(output.position, clippingPlane);

	// Vector going from the camera to the fragment
	// Used as 3D texture coordinate
	output.camToFrag = output.position.xyz - cameraPos;

	output.position = mul(output.position, mVP);
	
	return output;
}

TextureCube cubeMap;
SamplerState ss;

float4 PSMain(PSIn input) : SV_TARGET {

	input.camToFrag = normalize(input.camToFrag);
	float4 color = cubeMap.Sample(ss, input.camToFrag);

	return color;
	//return float4(1.f, 0.f, 0.f, 1.f);

}