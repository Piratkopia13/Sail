#include "../Phong.hlsl"

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
	float3 toCam : TOCAM;
	float3x3 TBN : TBN;
};

// Fast as frick data through the pipeline itself
[[vk::push_constant]]
struct {
	matrix sys_mWorld;
	uint sys_materialIndex;
	float3 padding;
} VSPSConsts;

// These cbuffers are shared between all draw calls
cbuffer VSSystemCBuffer : register(b0) {
    matrix sys_mVP;
    float4 sys_clippingPlane;
    float3 sys_cameraPos;
	float padding;
	DirectionalLight dirLight;
	PointLight pointLights[64];
}

cbuffer VSPSMaterials : register(b1) {
	PhongMaterial sys_materials[10];
}
// struct PointLightInput {
// 	float3 color;
//     float attRadius;
// 	float3 position;
// 	float intensity;
// };

PSIn VSMain(VSIn input) {
	PSIn output;

	PhongMaterial mat = sys_materials[VSPSConsts.sys_materialIndex];

	// REMOVE THESE LINES WHEN TEXTURE WORK IN VK
	matrix sys_mWorld = VSPSConsts.sys_mWorld;

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

	if (mat.normalTexIndex != -1) {
	    // Convert to tangent space
		output.TBN = float3x3(
			mul((float3x3) sys_mWorld, normalize(input.tangent)),
			mul((float3x3) sys_mWorld, normalize(input.bitangent)),
			mul((float3x3) sys_mWorld, normalize(input.normal))
		);
		output.TBN = transpose(output.TBN);
	}

	output.normal = mul((float3x3) sys_mWorld, input.normal);
	output.normal = normalize(output.normal);

	output.texCoords = input.texCoords;

	return output;

}

[[vk::binding(5)]]
SamplerState PSss : register(s0);

[[vk::binding(5)]]
Texture2D texArr[] : register(t3);

float4 sampleTexture(uint index, float2 texCoords) {
	return texArr[index].Sample(PSss, texCoords);
}

float4 PSMain(PSIn input) : SV_Target0 {

	PhongMaterial mat = sys_materials[VSPSConsts.sys_materialIndex];

	// REMOVE THIS LINE WHEN TEXTURE WORK IN VK
	// return sampleTexture(mat.diffuseTexIndex, input.texCoords);

	float3 toCam = input.toCam;

	if (mat.normalTexIndex != -1) {
		toCam = mul(toCam, input.TBN);

		// output.lights.dirLight.direction = mul(output.lights.dirLight.direction, TBN);
        // for (int i = 0; i < NUM_POINT_LIGHTS; i++)
        //     output.lights.pointLights[i].fragToLight = mul(output.lights.pointLights[i].fragToLight, TBN);
    }


	PhongInput phongInput;
	phongInput.mat = mat;
	phongInput.fragToCam = toCam;
	phongInput.dirLight = dirLight;
	phongInput.pointLights = pointLights;

	phongInput.diffuseColor = mat.modelColor;
	if (mat.diffuseTexIndex != -1)
		phongInput.diffuseColor *= sampleTexture(mat.diffuseTexIndex, input.texCoords);

	phongInput.normal = input.normal;
	if (mat.normalTexIndex != -1)
		phongInput.normal = sampleTexture(mat.normalTexIndex, input.texCoords).rgb * 2.f - 1.f;

	phongInput.specMap = float3(1.f, 1.f, 1.f);
	if (mat.specularTexIndex != -1)
		phongInput.specMap = sampleTexture(mat.specularTexIndex, input.texCoords).rgb;


    // //return sys_texDiffuse.Sample(PSss, input.texCoords);
	return float4(phongInput.normal * 0.5f + 0.5, 1.f);
    // return phongShade(phongInput);
    // //return float4(phongInput.lights.dirLight.direction, 1.f);
    // //return float4(phongInput.diffuseColor.rgb, 1.f);
    // return float4(0.f, 1.f, 0.f, 1.f);

}

