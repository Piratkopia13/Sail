struct PointLightInput {
	float3 color;
    float attRadius;
	float3 position;
	float intensity;
};

#include "../PBR.hlsl"

struct VSIn {
	float4 position : POSITION0;
};

struct PSIn {
	float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

cbuffer PSSystemCBuffer : register(b0) {
    matrix sys_mViewInv;
    float3 sys_cameraPos;
    uint sys_numPLights;
    bool useSSAO;
}

cbuffer PSLights : register(b1) {
	DirectionalLight dirLight;
    PointLightInput pointLights[NUM_POINT_LIGHTS];
}

PSIn VSMain(VSIn input) {
	PSIn output;

	input.position.w = 1.f;
	output.position = input.position;
    output.texCoord = input.position.xy * 0.5f + 0.5f;
    output.texCoord.y = 1.f - output.texCoord.y;

	return output;
}


Texture2D sys_texBrdfLUT : register(t0);
TextureCube irradianceMap : register(t1);
TextureCube radianceMap : register(t2);

Texture2D def_positions     : register(t3);
Texture2D def_worldNormals  : register(t4);
Texture2D def_albedo        : register(t5);
Texture2D def_mrao          : register(t6);
Texture2D tex_ssao          : register(t7);
Texture2D tex_shadows       : register(t8);
SamplerState PSss            : SAIL_SAMPLER_ANIS_WRAP; // s0
SamplerState PSLinearSampler : SAIL_SAMPLER_LINEAR_CLAMP; // s2

float4 PSMain(PSIn input) : SV_Target0 {

	// return sys_texBrdfLUT.Sample(PSss, input.texCoord);
	// float3 viewDir = input.worldPos - sys_cameraPos;
	// return irradianceMap.SampleLevel(PSss, viewDir, 0);
	// return radianceMap.SampleLevel(PSss, viewDir, 0);

	float4 shadows = tex_shadows.Sample(PSss, input.texCoord);
	// return shadows;

    float3 worldPos = mul(sys_mViewInv, def_positions.Sample(PSss, input.texCoord)).xyz;
    // return float4(worldPos / 50.f, 1.0f);

	PBRScene scene;
	
	// Lights
	scene.dirLight = dirLight;
	scene.pointLights = pointLights;
	scene.numPLs = sys_numPLights;

	scene.brdfLUT = sys_texBrdfLUT;
	scene.prefilterMap = radianceMap;
	scene.irradianceMap = irradianceMap;
	scene.linearSampler = PSLinearSampler;
	
	PBRPixel pixel;
    pixel.worldPos = worldPos;
	pixel.camPos = sys_cameraPos;

	pixel.inShadow = 1.f - shadows.r;
	pixel.albedo = def_albedo.Sample(PSss, input.texCoord).rgb;
	pixel.worldNormal = def_worldNormals.Sample(PSss, input.texCoord).rgb;
    // return float4(pixel.worldNormal / 2.f, 1.0f);
    
    float3 mrao = def_mrao.Sample(PSss, input.texCoord).rgb;
	pixel.metalness = mrao.r;
	pixel.roughness = mrao.g;
	pixel.ao = mrao.b;
    if (useSSAO)
	    pixel.ao *= pow(tex_ssao.Sample(PSLinearSampler, input.texCoord).r, 3.f);

    // return float4(pixel.ao, pixel.ao, pixel.ao, 1.0f);

    // Shade
	float3 shadedColor = pbrShade(scene, pixel);

	// Gamma correction
    float3 output = shadedColor / (shadedColor + 1.0f);
    // Tone mapping using the Reinhard operator
    output = pow(output, 1.0f / 2.2f);
	return float4(output, 1.0);

	// return float4(shadedColor, 1.0);
	// return float4(worldPos, 1.0);
}

