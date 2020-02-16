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
}

PSIn VSMain(VSIn input) {
	PSIn output;

	output.texCoord = input.position.xyz;

	input.position.w = 0.f; // Remove translation from the view matrix
	output.position = mul(sys_mView, input.position);
	output.position.w = 1.f;
	output.position = mul(sys_mProjection, output.position);
	output.position.z = 1.0f; // Draw at the back of the depth range
	
	return output;
}

TextureCube sys_tex0 : register(t0);
SamplerState PSss : register(s2); // Linear sampler

float4 PSMain(PSIn input) : SV_TARGET {
	float3 color = sys_tex0.SampleLevel(PSss, input.texCoord, 0).rgb;

	// Gamma correction
    float3 output = color / (color + 1.0f);
    // Tone mapping using the Reinhard operator
    output = pow(output, 1.0f / 2.2f);
	return float4(output, 1.0);

}