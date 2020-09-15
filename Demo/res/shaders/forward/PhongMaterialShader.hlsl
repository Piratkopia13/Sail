#include "../Phong.hlsl"

struct VSIn {
	float4 position : POSITION0;
	float2 texCoords : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	float3 bitangent : BINORMAL0;
	unsigned int vertexID : SV_VERTEXID;
};

struct PSIn {
	float4 position : SV_Position;
	float3 normal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	float clip : SV_ClipDistance0;
	float3 toCam : TOCAM;
	//Material material : MAT;
	unsigned int vertexID : ASD;
	LightList lights : LIGHTS;
};

// These cbuffers are shared between all draw calls
cbuffer VSPSSystemCBuffer : register(b0) {
	// matrix sys_mWorld;
    matrix sys_mVP;
    // PhongMaterial sys_material;
    //float padding;
    float4 sys_clippingPlane;
    float3 sys_cameraPos;
}

ConstantBuffer<PhongMaterial> VSPSMaterials[] : register(b1);

// Fast as frick data through the pipeline itself
[[vk::push_constant]]
struct {
	matrix sys_mWorld;
	uint materialIndex;
} VSPCTest;

struct PointLightInput {
	float3 color;
    float attRadius;
	float3 position;
	float intensity;
};
cbuffer VSLights : register(b2) {
	DirectionalLight dirLight;
    PointLightInput pointLights[NUM_POINT_LIGHTS];
}

PSIn VSMain(VSIn input) {
	PSIn output;

	// REMOVE THESE LINES WHEN TEXTURE WORK IN VK
	matrix sys_mWorld = VSPCTest.sys_mWorld;

	// input.position.w = 1.f;
	// output.position = mul(sys_mWorld, input.position);
    // output.position = mul(output.position, sys_mVP);
	// return output;

	output.vertexID = input.vertexID;
	// Copy over the directional light
	// output.lights.dirLight = dirLight;
	// // Copy over point lights
    // for (uint i = 0; i < NUM_POINT_LIGHTS; i++) {
    //     output.lights.pointLights[i].attRadius = pointLights[i].attRadius;
    //     output.lights.pointLights[i].color = pointLights[i].color;
    //     output.lights.pointLights[i].intensity = pointLights[i].intensity;
    // }

	input.position.w = 1.f;
	output.position = mul(sys_mWorld, input.position);
	// output.position = mul(input.position, sys_mWorld);

	// Calculate the distance from the vertex to the clipping plane
	// This needs to be done with world coordinates
    output.clip = dot(output.position, sys_clippingPlane);

	// World space vector pointing from the vertex position to the camera
    output.toCam = sys_cameraPos - output.position.xyz;

    // for (uint j = 0; j < NUM_POINT_LIGHTS; j++) {
	// 	// World space vector poiting from the vertex position to the point light
    //     output.lights.pointLights[j].fragToLight = pointLights[j].position - output.position.xyz;
    // }


    output.position = mul(sys_mVP, output.position);

	if (VSPSMaterials[0].hasNormalTexture) {
	    // Convert to tangent space
		float3x3 TBN = {
			mul((float3x3) sys_mWorld, normalize(input.tangent)),
			mul((float3x3) sys_mWorld, normalize(input.bitangent)),
			mul((float3x3) sys_mWorld, normalize(input.normal))
		};
		TBN = transpose(TBN);

		output.toCam = mul(output.toCam, TBN);
		// output.lights.dirLight.direction = mul(output.lights.dirLight.direction, TBN);
        // for (int i = 0; i < NUM_POINT_LIGHTS; i++)
        //     output.lights.pointLights[i].fragToLight = mul(output.lights.pointLights[i].fragToLight, TBN);
    }

	output.normal = mul((float3x3) sys_mWorld, input.normal);
	output.normal = normalize(output.normal);

	output.texCoords = input.texCoords;

	return output;

}

// [[vk::binding(5)]] // Since 0 and 1 are used for cbuffers - start textures after that in vk
// Texture2D sys_texDiffuse : register(t0);
// [[vk::binding(6)]]
// Texture2D sys_texNormal : register(t1);
// [[vk::binding(7)]]
// Texture2D sys_texSpecular : register(t2);
[[vk::binding(5)]]
SamplerState PSss : register(s0);

[[vk::binding(5)]]
Texture2D texArr[] : register(t3);

float4 PSMain(PSIn input) : SV_Target0 {

	// REMOVE THIS LINE WHEN TEXTURE WORK IN VK
	// return float4(0.2f, 0.8f, 0.8f, 1.0f);
	// return sys_texDiffuse.Sample(PSss, input.texCoords);
	if (input.vertexID == 0)
		return texArr[0].Sample(PSss, input.texCoords);
	else
		return texArr[1].Sample(PSss, input.texCoords);

	// PhongInput phongInput;
	// phongInput.mat = sys_material;
	// phongInput.fragToCam = input.toCam;
	// phongInput.lights = input.lights;

	// phongInput.diffuseColor = sys_material.modelColor;
	// if (sys_material.hasDiffuseTexture)
	// 	phongInput.diffuseColor *= sys_texDiffuse.Sample(PSss, input.texCoords);

	// phongInput.normal = input.normal;
	// if (sys_material.hasNormalTexture)
	// 	phongInput.normal = sys_texNormal.Sample(PSss, input.texCoords).rgb * 2.f - 1.f;

	// phongInput.specMap = float3(1.f, 1.f, 1.f);
	// if (sys_material.hasSpecularTexture)
	// 	phongInput.specMap = sys_texSpecular.Sample(PSss, input.texCoords).rgb;


    // //return sys_texDiffuse.Sample(PSss, input.texCoords);
	// // return float4(phongInput.normal * 0.5f + 0.5, 1.f);
    // return phongShade(phongInput);
    // //return float4(phongInput.lights.dirLight.direction, 1.f);
    // //return float4(phongInput.diffuseColor.rgb, 1.f);
    // //return float4(0.f, 1.f, 0.f, 1.f);

}

