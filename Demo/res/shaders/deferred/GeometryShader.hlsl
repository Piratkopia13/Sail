#include "Phong.hlsl"

struct VSIn {
	float4 position : POSITION0;
	float3 normal : NORMAL0;
	float2 texCoords : TEXCOORD0;
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
    float3x3 tbn : TBN;
    float clip : SV_ClipDistance0;
};

cbuffer VSPSSystemCBuffer : register(b0) {
    matrix sys_mWorld;
    matrix sys_mView;
    matrix sys_mProj;
    Material sys_material;
}

GSIn VSMain(VSIn input) {
    GSIn output;

    matrix wv = mul(sys_mWorld, sys_mView);

	output.texCoords = input.texCoords;
	input.position.w = 1.f;
	output.position = mul(input.position, wv);
	output.posVS = output.position;
	// Convert position into projection space
    output.position = mul(output.position, sys_mProj);
    
	// Convert normal into view space and normalize
	output.normal = mul(input.normal, (float3x3) wv);
	output.normal = normalize(output.normal);

	// Create TBN matrix to go from tangent space to view space
	output.tbn = float3x3(
	  normalize(mul(input.tangent, (float3x3) wv)),
	  normalize(mul(input.bitangent, (float3x3) wv)),
	  output.normal
	);

	return output;

}

cbuffer GSWorldData : register(b0) {
    float4 sys_clippingPlane;
}
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
            psin.normal = input[i].normal;
            psin.position = input[i].position;
            psin.tbn = input[i].tbn;
            psin.texCoords = input[i].texCoords;
            // Calculate the distance from the vertex to the clipping plane
            psin.clip = 0; // TODO: re-add the next line when clipping planes work
            //psin.clip = dot(input[i].position, sys_clippingPlane);
            output.Append(psin);
        }
    }
        
}



Texture2D sys_texDiffuse : register(t0);
Texture2D sys_texNormal : register(t1);
Texture2D sys_texSpecular : register(t2);
SamplerState PSss;

struct GBuffers {
	float4 diffuse : SV_Target0;
	float4 normal : SV_Target1;
	float4 specular : SV_Target2;
	float4 ambient : SV_Target3;
};

GBuffers PSMain(PSIn input) {

	GBuffers gbuffers;

	float3 ambientCoefficient = float3(0.5f, 0.5f, 0.5f);

	gbuffers.diffuse = float4(1.f, 1.f, 1.f, 1.f);
	//gbuffers.diffuse = float4(material.modelColor.rgb, 1.0f);
	if (sys_material.hasDiffuseTexture) {
		gbuffers.diffuse = sys_texDiffuse.Sample(PSss, input.texCoords);
		if (gbuffers.diffuse.a < 1.f) {
			gbuffers.diffuse.rgb *= sys_material.modelColor.rgb;
		}
		//gbuffers.diffuse.a = 1.f;
	} else {
        gbuffers.diffuse *= float4(sys_material.modelColor.rgb, 1.f);
    }
    gbuffers.diffuse.rgb *= sys_material.kd;
	
	// Write the bloom cutoff
	//gbuffers.bloom = gbuffers.diffuse * dot(gbuffers.diffuse.rgb, float3(0.2126, 0.7152, 0.0722));

	// bind light paPSss target and render ambient to it
    gbuffers.ambient = float4(gbuffers.diffuse.rgb * ambientCoefficient * sys_material.ka, 1.0f);

    gbuffers.normal = float4(normalize(input.normal) / 2.f + .5f, 1.f);
    if (sys_material.hasNormalTexture)
        gbuffers.normal = float4(mul(normalize(sys_texNormal.Sample(PSss, input.texCoords).rgb * 2.f - 1.f), input.tbn) / 2.f + .5f, 1.0f);

    gbuffers.specular = float4(1.f, sys_material.shininess, 1.f, 1.f);
    if (sys_material.hasSpecularTexture)
        gbuffers.specular.x = sys_texSpecular.Sample(PSss, input.texCoords).r * sys_material.ks;

    return gbuffers;

  }

