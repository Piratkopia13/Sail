#include "../variables.shared"

struct VSIn {
	float4 position : POSITION0;
	float3 normal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	float3 tangent : TANGENT0;
	float3 bitangent : BINORMAL0;
};

struct PSIn {
	float4 position : SV_Position;
	float3 normal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	float clip : SV_ClipDistance0;
	float3 vsPos : VSPOS;
	float3x3 tbn : TBN;
};

[[vk::push_constant]]
struct {
	matrix sys_mWorld;
	uint sys_materialIndex;
} VSPSConsts;

cbuffer VSPSSystemCBuffer : register(b0) {
	matrix sys_mView;
    matrix sys_mProjection;
    float4 sys_clippingPlane;
}

cbuffer VSPSMaterials : register(b1) : SAIL_BIND_ALL_MATERIALS {
	PBRMaterial sys_materials[1024];
}

PSIn VSMain(VSIn input) {
	PSIn output;

	PBRMaterial mat = sys_materials[VSPSConsts.sys_materialIndex];
	matrix mWorld = VSPSConsts.sys_mWorld;

	input.position.w = 1.f;
	output.position = mul(mWorld, input.position);

	// Calculate the distance from the vertex to the clipping plane
	// This needs to be done with world coordinates
    output.clip = dot(output.position, sys_clippingPlane);

	// World space vector pointing from the vertex position to the camera
    // output.worldPos = output.position.xyz;

	output.position = mul(sys_mView, output.position);
    output.vsPos = output.position.xyz;
	output.position = mul(sys_mProjection, output.position);

	output.tbn = 0.f;

	if (mat.normalTexIndex != -1) {
	    // Convert to tangent space
		float3x3 tbn = {
			mul((float3x3) mWorld, normalize(input.tangent)),
			mul((float3x3) mWorld, normalize(input.bitangent)),
			mul((float3x3) mWorld, normalize(input.normal))
		};
		output.tbn = tbn;
    }

	// float4 worldNormal = mul(mWorld, float4(normalize(input.normal), 0.f));
	// output.normal = mul(sys_mView, worldNormal).xyz;
    output.normal = mul(mWorld, float4(normalize(input.normal), 0.f)).xyz;
	output.texCoords = input.texCoords;

	return output;
}

Texture2D texArr[] : register(t2) : SAIL_BIND_ALL_TEXTURES;
SamplerState PSss : register(s3) : SAIL_SAMPLER_ANIS_WRAP;

struct GBuffers {
	float4 positions    : SV_Target0;
	float4 worldNormals : SV_Target1;
	float4 albedo       : SV_Target2;
    float4 mrao         : SV_Target3; // Metalness, roughness, ao
};

float4 sampleTexture(uint index, float2 texCoords) {
	return texArr[index].Sample(PSss, texCoords);
}

GBuffers PSMain(PSIn input) {
	PBRMaterial mat = sys_materials[VSPSConsts.sys_materialIndex];

	GBuffers gbuffers;
    gbuffers.positions = float4(input.vsPos, 1.f);

	float3 albedo = mat.modelColor.rgb;
	if (mat.albedoTexIndex != -1)
		albedo *= sampleTexture(mat.albedoTexIndex, input.texCoords).rgb;
    gbuffers.albedo = float4(albedo, 1.0f);

	float3 worldNormal = input.normal;
	if (mat.normalTexIndex != -1) {
		float3 normalSample = sampleTexture(mat.normalTexIndex, input.texCoords).rgb;
        normalSample.y = 1.f - normalSample.y;
        normalSample.x = 1.f - normalSample.x;
		
        worldNormal = mul(normalize(normalSample * 2.f - 1.f), input.tbn);
	}
    gbuffers.worldNormals = float4(worldNormal, 1.0f);
    // gbuffers.worldNormals = float4(normalize(input.normal), 1.0f);

	float metalness = mat.metalnessScale;
	float roughness = mat.roughnessScale;
	float ao = 1.f;
	if (mat.mraoTexIndex != -1) {
		float3 mrao = sampleTexture(mat.mraoTexIndex, input.texCoords).rgb;
		metalness *= mrao.r;
		roughness *= 1.f - mrao.g; // Invert roughness from texture to make it correct
		ao = mrao.b + mat.aoIntensity;
	}
    gbuffers.mrao = float4(metalness, roughness, ao, 1.0f);

    return gbuffers;

}
