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
	float3 normalWorldSpace : NORMAL0;
	float2 texCoords : TEXCOORD0;
	float clip : SV_ClipDistance0;
	float3 worldPosition : WORLDPOS;
	float3 toCam : TOCAM;
	float3x3 TBN : TBN;
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

// These cbuffers are shared between all draw calls
cbuffer VSSystemCBuffer : register(b0) {
    matrix sys_mVP;
    float4 sys_clippingPlane;
    float3 sys_cameraPos;
	float padding;
	DirectionalLight dirLight;
	PointLight pointLights[8];
}

cbuffer VSPSMaterials : register(b1) : SAIL_BIND_ALL_MATERIALS {
	PhongMaterial sys_materials[1024];
}

PSIn VSMain(VSIn input) {
	PSIn output;

#ifdef _SAIL_VK
	PhongMaterial mat = sys_materials[VSPSConsts.sys_materialIndex];
	matrix mWorld = VSPSConsts.sys_mWorld;
#else
	PhongMaterial mat = sys_materials[sys_materialIndex];
	matrix mWorld = sys_mWorld;
#endif

	input.position.w = 1.f;
	output.position = mul(mWorld, input.position);
	// output.position = mul(input.position, mWorld);

	output.worldPosition = output.position.xyz;

	// Calculate the distance from the vertex to the clipping plane
	// This needs to be done with world coordinates
    output.clip = dot(output.position, sys_clippingPlane);

	// World space vector pointing from the vertex position to the camera
    output.toCam = sys_cameraPos - output.position.xyz;

    output.position = mul(sys_mVP, output.position);

	float3 tangentWorldSpace = normalize(mul(mWorld, float4(input.tangent, 0.f)).xyz);
	float3 bitangentWorldSpace = normalize(mul(mWorld, float4(input.bitangent, 0.f)).xyz);
	output.normalWorldSpace = normalize(mul(mWorld, float4(input.normal, 0.f)).xyz);
	
	if (mat.normalTexIndex != -1) {
	    // TBN matrix to go from tangent space to world space
		output.TBN = float3x3(
			tangentWorldSpace,
			bitangentWorldSpace,
			output.normalWorldSpace
		);
		// output.TBN = transpose(output.TBN);
	}

	output.texCoords = input.texCoords;

	return output;

}

SamplerState PSss  : SAIL_SAMPLER_ANIS_WRAP : register(s2);
Texture2D texArr[] : SAIL_BIND_ALL_TEXTURES : register(t3);

float4 sampleTexture(uint index, float2 texCoords) {
	return texArr[index].Sample(PSss, texCoords);
}

float4 PSMain(PSIn input) : SV_Target0 {

#ifdef _SAIL_VK
	PhongMaterial mat = sys_materials[VSPSConsts.sys_materialIndex];
#else
	PhongMaterial mat = sys_materials[sys_materialIndex];
#endif

	PointLight myPointLights[NUM_POINT_LIGHTS] = pointLights;
	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
		myPointLights[i].fragToLight = myPointLights[i].fragToLight - input.worldPosition;

	PhongInput phongInput;
	phongInput.mat = mat;
	phongInput.fragToCam = input.toCam;
	phongInput.dirLight = dirLight;
	phongInput.pointLights = myPointLights;

	phongInput.diffuseColor = mat.modelColor;
	if (mat.diffuseTexIndex != -1)
		phongInput.diffuseColor *= sampleTexture(mat.diffuseTexIndex, input.texCoords);

	phongInput.normal = input.normalWorldSpace;
	if (mat.normalTexIndex != -1) {
		// Sample tangent space normal from texture
		float3 normalSample = sampleTexture(mat.normalTexIndex, input.texCoords).rgb;
		// normalSample.y = 1.0f - normalSample.y;
		normalSample.x = 1.0f - normalSample.x;
		phongInput.normal = normalize(normalSample * 2.f - 1.f);

		// Convert to world space
		phongInput.normal = mul(phongInput.normal, input.TBN);
	}

	phongInput.specMap = float3(1.f, 1.f, 1.f);
	if (mat.specularTexIndex != -1)
		phongInput.specMap = sampleTexture(mat.specularTexIndex, input.texCoords).rgb;


	// return float4(myPointLights[0].fragToLight, 1.0);
    // //return sys_texDiffuse.Sample(PSss, input.texCoords);
	// return float4(phongInput.normal * 0.5f + 0.5, 1.f);
	// return float4(toCam * 0.5f + 0.5, 1.f);
    return phongShade(phongInput);
    // //return float4(phongInput.lights.dirLight.direction, 1.f);
    // return float4(phongInput.diffuseColor.rgb, 1.f);
    // return float4(0.f, 1.f, 0.f, 1.f);

}

