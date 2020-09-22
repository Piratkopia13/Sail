struct VSIn {
	float4 position : POSITION;
};

struct PSIn {
	float4 position : SV_POSITION;
	float3 texCoord : TEXCOORD;
};

cbuffer VSSystemCBuffer : register(b0) {
	matrix sys_mView;
	matrix sys_mProjection;
	matrix sys_mVP;
}

PSIn VSMain(VSIn input) {
	PSIn output;

	output.texCoord = input.position.xyz;

	input.position.w = 0.f; // Remove translation from the view matrix
	output.position = mul(sys_mView, input.position);
	output.position.w = 1.f;
	output.position = mul(sys_mProjection, output.position);
	output.position = output.position.xyzz; // Draw at the back of the depth range

	return output;
}

TextureCube sys_tex0[] : register(t1) : SAIL_BIND_ALL_TEXTURECUBES;
SamplerState PSss : register(s2) : SAIL_SAMPLER_ANIS_WRAP;

float4 PSMain(PSIn input) : SV_TARGET {
	float3 color = sys_tex0[0].SampleLevel(PSss, input.texCoord, 0).rgb;

	return float4(color, 1.0f);

	// // Gamma correction
    // float3 output = color / (color + 1.0f);
    // // Tone mapping using the Reinhard operator
    // output = pow(output, 1.0f / 2.2f);
	// return float4(output, 1.0);

}