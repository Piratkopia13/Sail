#include "../PBR.hlsl"

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

cbuffer VSPSSystemCBuffer : register(b0) {
	matrix sys_mWorld;
    matrix sys_mView;
    matrix sys_mProjection;
    PBRMaterial sys_material;
    float4 sys_clippingPlane;
}

PSIn VSMain(VSIn input) {
	PSIn output;

	input.position.w = 1.f;
	output.position = mul(sys_mWorld, input.position);

	// Calculate the distance from the vertex to the clipping plane
	// This needs to be done with world coordinates
    output.clip = dot(output.position, sys_clippingPlane);

	// World space vector pointing from the vertex position to the camera
    // output.worldPos = output.position.xyz;

	output.position = mul(sys_mView, output.position);
    output.vsPos = output.position.xyz;
	output.position = mul(sys_mProjection, output.position);

	output.tbn = 0.f;

	if (sys_material.hasNormalTexture) {
	    // Convert to tangent space
		float3x3 tbn = {
			mul((float3x3) sys_mWorld, normalize(input.tangent)),
			mul((float3x3) sys_mWorld, normalize(input.bitangent)),
			mul((float3x3) sys_mWorld, normalize(input.normal))
		};
		output.tbn = tbn;
    }

	output.normal = mul((float3x3) sys_mWorld, normalize(input.normal));
	output.texCoords = input.texCoords;

	return output;
}

Texture2D sys_texAlbedo : register(t3);
Texture2D sys_texNormal : register(t4);
Texture2D sys_texMRAO : register(t5);
SamplerState PSss : register(s0);

struct GBuffers {
	float4 positions    : SV_Target0;
	float4 worldNormals : SV_Target1;
	float4 albedo       : SV_Target2;
    float4 mrao         : SV_Target3; // Metalness, roughness, ao
};

GBuffers PSMain(PSIn input) {

	GBuffers gbuffers;

    gbuffers.positions = float4(input.vsPos * 0.5f + 0.5f, 1.f);

	float3 albedo = sys_material.modelColor.rgb;
	if (sys_material.hasAlbedoTexture)
		albedo *= sys_texAlbedo.Sample(PSss, input.texCoords).rgb;
    gbuffers.albedo = float4(albedo, 1.0f);

	float3 worldNormal = input.normal;
	if (sys_material.hasNormalTexture) {
		float3 normalSample = sys_texNormal.Sample(PSss, input.texCoords).rgb;
        normalSample.y = 1.f - normalSample.y;
        normalSample.x = 1.f - normalSample.x;
		
        worldNormal = mul(normalize(normalSample * 2.f - 1.f), input.tbn);
	}
    gbuffers.worldNormals = float4(worldNormal * 0.5f + 0.5f, 1.0f);

	float metalness = sys_material.metalnessScale;
	float roughness = sys_material.roughnessScale;
	float ao = 1.f;
	if (sys_material.hasMRAOTexture) {
		float3 mrao = sys_texMRAO.Sample(PSss, input.texCoords).rgb;
		metalness *= mrao.r;
		roughness *= 1.f - mrao.g; // Invert roughness from texture to make it correct
		ao = mrao.b + sys_material.aoIntensity;
	}
    gbuffers.mrao = float4(metalness, roughness, ao, 1.0f);

    return gbuffers;

}
