#include "../Phong.hlsl"

struct VSIn {
	float4 position : POSITION0;
	float2 texCoords : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	float3 bitangent : BINORMAL0;
};

struct GSIn {
    float4 position : SV_Position;
	float4 posVS : POSVS;
    float3 normal : NORMAL0;
    float2 texCoords : TEXCOORD0;
    float3x3 tbn : TBN;
};

struct PSIn {
    float4 position : SV_Position;
    float3 normal : NORMAL0;
    float2 texCoords : TEXCOORD0;
    // float clip : SV_ClipDistance0;
    float3x3 tbn : TBN;
};

cbuffer VSSystemCBuffer : register(b0) {
    matrix sys_mWorld;
    matrix sys_mView;
    matrix sys_mProj;
}

cbuffer PSSystemCBuffer : register(b1) {
    Material sys_material;
}

GSIn VSMain(VSIn input) {
    GSIn output;

    matrix wv = mul(sys_mView, sys_mWorld);

	output.texCoords = input.texCoords;
	input.position.w = 1.f;
	output.position = mul(wv, input.position);
	output.posVS = output.position;
	// Convert position into projection space
    output.position = mul(sys_mProj, output.position);
    
	// Convert normal into world space and normalize
	output.normal = mul((float3x3) sys_mWorld, input.normal);
	output.normal = normalize(output.normal);

	// Create TBN matrix to go from tangent space to world space
	output.tbn = float3x3(
	  normalize(mul((float3x3) sys_mWorld, input.tangent)),
	  normalize(mul((float3x3) sys_mWorld, input.bitangent)),
	  output.normal
	);

	return output;
}

// Geometry shader only used to cull back-facing triangles
[maxvertexcount(3)]
void GSMain(triangle GSIn input[3], inout TriangleStream<PSIn> output) {
    float3 edge1 = input[1].posVS.xyz - input[0].posVS.xyz;
    float3 edge2 = input[2].posVS.xyz - input[0].posVS.xyz;
    float3 normal = normalize(cross(edge1, edge2));

    // Append triangle to output if normal is facing the screen
    if (dot(-normalize(input[0].posVS.xyz), normal) > 0) {
        for (uint i = 0; i < 3; i++) {
            PSIn psin;
            // Copy input to output
            psin.position = input[i].position;
            psin.normal = input[i].normal;
            psin.texCoords = input[i].texCoords;
            psin.tbn = input[i].tbn;
            // Calculate the distance from the vertex to the clipping plane
            // psin.clip = 0;
            output.Append(psin);
        }
    }
}

// TODO: check if it is worth the extra VRAM to write diffuse and specular gbuffers instead of sampling them in the raytracing shaders
//       The advantage is that mip map level can be calculated automatically here
//      
Texture2D sys_texDiffuse : register(t0);
// Texture2D sys_texSpecular : register(t2);
Texture2D sys_texNormal : register(t1);
SamplerState PSss;

struct GBuffers {
	float4 normal  : SV_Target0;
	float4 diffuse : SV_Target1;
	// float4 diffuse : SV_Target0;
	// float4 specular : SV_Target2;
	// float4 ambient : SV_Target3;
};

GBuffers PSMain(PSIn input) {
	GBuffers gbuffers;

	gbuffers.normal = float4(normalize(input.normal) / 2.f + .5f, 1.f);
    if (sys_material.hasNormalTexture)
        gbuffers.normal = float4(mul(normalize(sys_texNormal.Sample(PSss, input.texCoords).rgb * 2.f - 1.f), input.tbn) / 2.f + .5f, 1.0f);

    // TODO: handle texcoords outside of [0..1] range
    gbuffers.diffuse = sys_texDiffuse.Sample(PSss, input.texCoords);

    return gbuffers;
}
