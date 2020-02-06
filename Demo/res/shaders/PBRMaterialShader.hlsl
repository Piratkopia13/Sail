#include "PBR.hlsl"

struct VSIn {
	float4 position : POSITION0;
	float2 texCoords : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	float3 bitangent : BINORMAL0;
};

struct PSIn {
	float4 position : SV_Position;
	float3 normal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	float clip : SV_ClipDistance0;
	float3 worldPos : worldPos;
	float3x3 tbn : TBN;
};

cbuffer VSPSSystemCBuffer : register(b0) {
	matrix sys_mWorld;
    matrix sys_mVP;
    PBRMaterial sys_material;
    //float padding;
    float4 sys_clippingPlane;
    float3 sys_cameraPos;
}

struct PointLightInput {
	float3 color;
	float padding1;
	float3 position;
    float attConstant;
    float attLinear;
    float attQuadratic;
	float2 padding2;
};
cbuffer PSLights : register(b1) {
	DirectionalLight dirLight;
    PointLightInput pointLights[NUM_POINT_LIGHTS];
}

PSIn VSMain(VSIn input) {
	PSIn output;

	input.position.w = 1.f;
	output.position = mul(sys_mWorld, input.position);

	// Calculate the distance from the vertex to the clipping plane
	// This needs to be done with world coordinates
    output.clip = dot(output.position, sys_clippingPlane);

	// World space vector pointing from the vertex position to the camera
    output.worldPos = output.position.xyz;

	output.position = mul(sys_mVP, output.position);
	output.tbn = 0.f;

	if (sys_material.hasNormalTexture) {
	    // Convert to tangent space
		float3x3 tbn = {
			mul((float3x3) sys_mWorld, normalize(input.tangent)),
			mul((float3x3) sys_mWorld, normalize(input.bitangent)),
			mul((float3x3) sys_mWorld, normalize(input.normal))
		};
		// tbn = transpose(tbn);

		output.tbn = tbn;
    }

	output.normal = mul((float3x3) sys_mWorld, normalize(input.normal));
	output.texCoords = input.texCoords;

	return output;
}


Texture2D sys_texBrdfLUT : register(t0);
TextureCube irradianceMap : register(t1);
TextureCube radianceMap : register(t2);

Texture2D sys_texAlbedo : register(t3);
Texture2D sys_texNormal : register(t4);
Texture2D sys_texMRAO : register(t5);
SamplerState PSss : register(s0);
SamplerState PSLinearSampler : register(s2);

float4 PSMain(PSIn input) : SV_Target0 {

	// return sys_texBrdfLUT.Sample(PSss, input.texCoords);
	// float3 viewDir = input.worldPos - sys_cameraPos;
	// return irradianceMap.SampleLevel(PSss, viewDir, 0);
	// return radianceMap.SampleLevel(PSss, viewDir, 0);

	PBRScene scene;
	
	// Lights
	scene.lights.dirLight = dirLight;
	[unroll]
	for (uint i = 0; i < NUM_POINT_LIGHTS; i++) {
		scene.lights.pointLights[i].color = pointLights[i].color;
		scene.lights.pointLights[i].attConstant = pointLights[i].attConstant;
		scene.lights.pointLights[i].attLinear = pointLights[i].attLinear;
		scene.lights.pointLights[i].attQuadratic = pointLights[i].attQuadratic;
		// World space vector poiting from the vertex position to the point light
		scene.lights.pointLights[i].fragToLight = pointLights[i].position - input.worldPos;
		// The world space distance from the vertex to the light
		scene.lights.pointLights[i].distanceToLight = length(scene.lights.pointLights[i].fragToLight);
	}

	scene.brdfLUT = sys_texBrdfLUT;
	scene.prefilterMap = radianceMap;
	scene.irradianceMap = irradianceMap;
	scene.linearSampler = PSLinearSampler;
	
	PBRPixel pixel;
    pixel.invViewDir = sys_cameraPos - input.worldPos;

	pixel.albedo = sys_material.modelColor.rgb;
	if (sys_material.hasAlbedoTexture)
		pixel.albedo *= sys_texAlbedo.Sample(PSss, input.texCoords).rgb;

	pixel.worldNormal = input.normal;
	if (sys_material.hasNormalTexture) {
		float3 normalSample = sys_texNormal.Sample(PSss, input.texCoords).rgb;
        normalSample.y = 1.f - normalSample.y;
        normalSample.x = 1.f - normalSample.x;
		
        pixel.worldNormal = mul(normalize(normalSample * 2.f - 1.f), input.tbn);
	}

	pixel.metalness = sys_material.metalnessScale;
	pixel.roughness = sys_material.roughnessScale;
	pixel.ao = sys_material.aoScale;
	if (sys_material.hasMRAOTexture) {
		float3 mrao = sys_texMRAO.Sample(PSss, input.texCoords).rgb;
		pixel.metalness *= mrao.r;
		pixel.roughness *= mrao.g;
		pixel.ao *= mrao.b;
	}

	float3 reflectionColor = -1.f;
	float3 shadedColor = pbrShade(scene, pixel, reflectionColor);

	// Gamma correction
    float3 output = shadedColor / (shadedColor + 1.0f);
    // Tone mapping using the Reinhard operator
    output = pow(output, 1.0f / 2.2f);
	return float4(output, 1.0);

	// return float4(shadedColor, 1.0);
	// return float4(input.worldPos, 1.0);
}

