#include "../PBR.hlsl"

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

cbuffer VSPSSystemCBuffer : register(b0) {
    matrix sys_mVP;
    float3 sys_cameraPos;
	float padding;
    float4 sys_clippingPlane;
	DirectionalLight dirLight;
	PointLight pointLights[8];
}

cbuffer VSPSMaterials : register(b1) : SAIL_BIND_ALL_MATERIALS {
	PBRMaterial sys_materials[1024];
}


PSIn VSMain(VSIn input) {
	PSIn output;

#ifdef _SAIL_VK
	PBRMaterial mat = sys_materials[VSPSConsts.sys_materialIndex];
	matrix mWorld = VSPSConsts.sys_mWorld;
#else
	PBRMaterial mat = sys_materials[sys_materialIndex];
	matrix mWorld = sys_mWorld;
#endif

	input.position.w = 1.f;
	output.position = mul(mWorld, input.position);

	// Calculate the distance from the vertex to the clipping plane
	// This needs to be done with world coordinates
    output.clip = dot(output.position, sys_clippingPlane);

	// World space vector pointing from the vertex position to the camera
    output.worldPos = output.position.xyz;

	output.position = mul(sys_mVP, output.position);
	output.tbn = 0.f;

	if (mat.normalTexIndex != -1) {
	    // Convert to tangent space
		float3x3 tbn = {
			mul((float3x3) mWorld, normalize(input.tangent)),
			mul((float3x3) mWorld, normalize(input.bitangent)),
			mul((float3x3) mWorld, normalize(input.normal))
		};
		// tbn = transpose(tbn);

		output.tbn = tbn;
    }

	output.normal = mul((float3x3) mWorld, normalize(input.normal));
	output.texCoords = input.texCoords;

	return output;
}

SamplerState PSssPoint : SAIL_SAMPLER_POINT_CLAMP : register(s5);
SamplerState PSss 	   : SAIL_SAMPLER_ANIS_WRAP	  : register(s6);

Texture2D texArr[]		 : SAIL_BIND_ALL_TEXTURES 	  : register(t7);
TextureCube texCubeArr[] : SAIL_BIND_ALL_TEXTURECUBES : register(t8);

float4 sampleTexture(uint index, float2 texCoords) {
	return texArr[index].Sample(PSss, texCoords);
}

float4 PSMain(PSIn input) : SV_Target0 {

	// return float4(0.8f, 0.2f, 0.2f, 1.0f);

	// return sys_texBrdfLUT.Sample(PSss, input.texCoords);
	// float3 viewDir = input.worldPos - sys_cameraPos;
	// return irradianceMap.SampleLevel(PSss, viewDir, 0);
	// return radianceMap.SampleLevel(PSss, viewDir, 0);

#ifdef _SAIL_VK
	PBRMaterial mat = sys_materials[VSPSConsts.sys_materialIndex];
#else
	PBRMaterial mat = sys_materials[sys_materialIndex];
#endif

	float3 camToFrag = normalize(input.worldPos - sys_cameraPos);
	// return texCubeArr[mat.irradianceMapTexIndex].Sample(PSss, camToFrag);
	// return texArr[mat.albedoTexIndex].Sample(PSss, input.texCoords);
	// return texCubeArr[mat.radianceMapTexIndex].Sample(PSss, camToFrag);
	
	PBRScene scene;
	
	// Lights
	scene.dirLight = dirLight;
	// return float4(scene.dirLight.direction * 0.5 + 0.5, 1.0f);
	[unroll]
	for (uint i = 0; i < NUM_POINT_LIGHTS; i++) {
		scene.pointLights[i].color = pointLights[i].color;
		scene.pointLights[i].attRadius = pointLights[i].attRadius;
		scene.pointLights[i].intensity = pointLights[i].intensity;
		// World space vector poiting from the vertex position to the point light
		scene.pointLights[i].fragToLight = pointLights[i].fragToLight - input.worldPos;
	}

	scene.brdfLUT = texArr[mat.brdfLutTexIndex];
	scene.prefilterMap = texCubeArr[mat.radianceMapTexIndex];
	scene.irradianceMap = texCubeArr[mat.irradianceMapTexIndex];
	scene.linearSampler = PSss;
	scene.pointSampler = PSssPoint;
	
	PBRPixel pixel;
	pixel.inShadow = false;
    pixel.camPos = sys_cameraPos;
	pixel.worldPos = input.worldPos;

	pixel.albedo = mat.modelColor.rgb;
	float alpha = mat.modelColor.a;
	if (mat.albedoTexIndex != -1) {
		float4 albedoSample = sampleTexture(mat.albedoTexIndex, input.texCoords);
		pixel.albedo *= albedoSample.rgb;
		alpha *= albedoSample.a;
	}

	pixel.worldNormal = input.normal;
	if (mat.normalTexIndex != -1) {
		float3 normalSample = sampleTexture(mat.normalTexIndex, input.texCoords).rgb;
        normalSample.y = 1.f - normalSample.y;
        normalSample.x = 1.f - normalSample.x;
		
        pixel.worldNormal = mul(normalize(normalSample * 2.f - 1.f), input.tbn);
	}

	// return float4(pixel.worldNormal * 0.5f + 0.5f, 1.0f);

	pixel.metalness = mat.metalnessScale;
	pixel.roughness = mat.roughnessScale;
	pixel.ao = 1.0f;
	if (mat.mraoTexIndex != -1) {
		float3 mrao = sampleTexture(mat.mraoTexIndex, input.texCoords).rgb;
		pixel.metalness *= mrao.r;
		pixel.roughness *= 1.f - mrao.g; // Invert roughness from texture to make it correct
		pixel.ao = mrao.b + mat.aoIntensity;
	}

	// Shade
	float3 shadedColor = pbrShade(scene, pixel);

#if GAMMA_CORRECT
	// Gamma correction
    float3 output = shadedColor / (shadedColor + 1.0f);
    // Tone mapping using the Reinhard operator
    output = pow(output, 1.0f / 2.2f);
	return float4(output, alpha);
#else
	return float4(shadedColor, alpha);
	// return float4(input.worldPos, 1.0);
#endif
}

