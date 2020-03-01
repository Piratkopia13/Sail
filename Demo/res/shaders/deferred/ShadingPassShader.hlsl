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
    int useSSAO;
    int useShadowTexture;
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
// RaytracingAccelerationStructure rtScene : register(t8);
SamplerState PSss            : SAIL_SAMPLER_ANIS_WRAP; // s0
SamplerState PSLinearSampler : SAIL_SAMPLER_LINEAR_CLAMP; // s2

float4 PSMain(PSIn input) : SV_Target0 {
	// return sys_texBrdfLUT.Sample(PSss, input.texCoord);
	// float3 viewDir = input.worldPos - sys_cameraPos;
	// return irradianceMap.SampleLevel(PSss, viewDir, 0);
	// return radianceMap.SampleLevel(PSss, viewDir, 0);

    float3 worldPos = mul(sys_mViewInv, def_positions.Sample(PSss, input.texCoord)).xyz;
	float3 worldNormal = def_worldNormals.Sample(PSss, input.texCoord).rgb;
    // return float4(worldPos / 50.f, 1.0f);


	// Inline raytracing
	// RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
    //          RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
    //          RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
	
	// // Cast a shadow ray from the directional light
	// RayDesc ray;
	// ray.Direction = -dirLight.direction;
	// ray.Origin = worldPos;
	// ray.Origin += worldNormal * 0.1f; // Offset slightly to avoid self-shadowing
	// 									   // This should preferably be done with the vertex normal and not a normal-mapped normal

	// // Set TMin to a non-zero small value to avoid aliasing issues due to floating point errors
	// // TMin should be kept small to prevent missing geometry at close contact areas
	// ray.TMin = 0.00001;
	// ray.TMax = 10000.0;

	// float4 shadows = 0.f;
	// Set up a trace - No work is done yet
    // q.TraceRayInline(
    //     rtScene,
    //     0, // OR'd with flags above
    //     0xFF,
    //     ray);
	// // Do the work
	// q.Proceed();
	// if(q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
	// 	// hit
	// 	shadows = 1.f;
	// } else {
	// 	// miss
	// }

	float shadows = 0.f;
	if (useShadowTexture)
		shadows = 1.f - tex_shadows.Sample(PSss, input.texCoord).r;
	// return shadows;

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

	pixel.inShadow = shadows;
	pixel.albedo = def_albedo.Sample(PSss, input.texCoord).rgb;
	pixel.worldNormal = worldNormal;
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

