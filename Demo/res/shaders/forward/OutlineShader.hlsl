struct VSIn {
	float4 position : POSITION;
	float3 normal : NORMAL;
};

struct PSIn {
	float4 position : SV_POSITION;
    float3 color : COLOR;
};

struct OutlineMaterial {
    float3 color;
    float thickness;
};

#ifdef _SAIL_VK
// VK ONLY
[[vk::push_constant]]
struct {
	matrix sys_mWorld;
	uint sys_materialIndex;
} VSPSConsts;
#else
// NOT VK
cbuffer VSPSConsts : SAIL_CONSTANT {
	matrix sys_mWorld;
	uint sys_materialIndex;
}
#endif

cbuffer VSSystemCBuffer : register(b0) {
	matrix sys_mVP;
}
cbuffer VSMaterialsBuffer : register(b1) : SAIL_BIND_ALL_MATERIALS {
	OutlineMaterial sys_materials[1024];
}

PSIn VSMain(VSIn input) {
	PSIn output;

#ifdef _SAIL_VK
	OutlineMaterial mat = sys_materials[VSPSConsts.sys_materialIndex];
    matrix mWorld = VSPSConsts.sys_mWorld;
#else
	OutlineMaterial mat = sys_materials[sys_materialIndex];
    matrix mWorld = sys_mWorld;
#endif
	output.color = mat.color;

    // Method 1 scale model 
	mWorld[0][0] *= 1.f + mat.thickness;
    mWorld[1][1] *= 1.f + mat.thickness;
    mWorld[2][2] *= 1.f + mat.thickness;

	input.position.w = 1.f;
	output.position = mul(mWorld, input.position);

    // Method 2 - move each vertex outwards along its normal
    // output.position.xyz += input.normal * mat.thickness;

	output.position = mul(sys_mVP, output.position);
	
	return output;
}

float4 PSMain(PSIn input) : SV_TARGET {
#if GAMMA_CORRECT
	// Gamma correction
    float3 output = input.color / (input.color + 1.0f);
    // Tone mapping using the Reinhard operator
    output = pow(output, 1.0f / 2.2f);
	return float4(output, 1.0);
#else
	return float4(input.color, 1.0f);
#endif
}