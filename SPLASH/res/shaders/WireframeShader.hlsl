#include "Phong.hlsl"

struct VSIn {
	float4 position : POSITION0;
};

struct PSIn {
	float4 position : SV_Position;
	float clip : SV_ClipDistance0;
};

cbuffer VSPSSystemCBuffer : register(b0) {
	matrix sys_mWorld;
    matrix sys_mVP;
    Material sys_material;
    //float padding;
    float4 sys_clippingPlane;
    float3 sys_cameraPos;
}

struct PointLightInput {
	float3 color;
	float3 position;
    float attConstant;
    //float attLinear;
    //float attQuadratic;
};
cbuffer VSLights : register(b1) {
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

    output.position = mul(sys_mVP, output.position);
	
	return output;
}

float4 PSMain(PSIn input) : SV_Target0 {
	return float4(sys_material.modelColor.rgb, 1.0f);
}

