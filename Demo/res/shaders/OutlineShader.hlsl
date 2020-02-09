struct VSIn {
	float4 position : POSITION;
	float3 normal : NORMAL;
};

struct PSIn {
	float4 position : SV_POSITION;
    float3 color : COLOR;
};

cbuffer VSSystemCBuffer : register(b0) {
	matrix sys_mWorld;
	matrix sys_mVP;
    float3 mat_color;
    float mat_thickness;
}

PSIn VSMain(VSIn input) {
	PSIn output;

	output.color = mat_color;

    // Method 1 scale model 
    matrix mWorld = sys_mWorld;
    mWorld[0][0] *= 1.f + mat_thickness;
    mWorld[1][1] *= 1.f + mat_thickness;
    mWorld[2][2] *= 1.f + mat_thickness;

	input.position.w = 1.f;
	output.position = mul(mWorld, input.position);

    // Method 2 - move each vertex outwards along its normal
    // output.position.xyz += input.normal * mat_thickness;

	output.position = mul(sys_mVP, output.position);
	
	return output;
}

TextureCube sys_tex0 : register(t0);
SamplerState PSss : register(s2); // Linear sampler

float4 PSMain(PSIn input) : SV_TARGET {
	return float4(input.color, 1.f);
}