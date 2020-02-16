struct VSIn {
	float4 position : POSITION0;
};

struct PSIn {
	float4 position : SV_Position;
	float4 clipSpace : CLIPSPACE;
	float3 positionVS : POSVS;
};

cbuffer VSSystemCBuffer : register(b0) {
    matrix sys_mWorld;
    matrix sys_mView;
    matrix sys_mProj;
}

//cbuffer ModelData : register(b0) {
//	matrix mWV;
//	matrix mP;
//}

PSIn VSMain(VSIn input) {
	PSIn output;

	input.position.w = 1.f;
	output.position = input.position;
    matrix wv = mul(sys_mWorld, sys_mView);
	output.position = mul(input.position, wv);
	output.position = mul(output.position, sys_mProj);
	output.clipSpace = output.position;

	output.positionVS = mul(input.position.xyz, (float3x3)wv);

	return output;

}

Texture2D def_texDiffuse : register(t0);
Texture2D def_texNormal : register(t1);
Texture2D def_texSpecular : register(t2);
Texture2D def_texDepth : register(t3);
SamplerState PSss;

struct DeferredPointLightData {
	float3 color;
	float attConstant;
	float attLinear;
	float attQuadratic;
	float3 positionVS; // View space position of pointlight
};

cbuffer PSLight : register(b0) {
	DeferredPointLightData def_pointLightInput;
}

float3 deferredPhongShading(DeferredPointLightData light, float3 fragPosVS, float3 diffuse, float3 specular, float3 normal) {

	// View space vector poiting from the fragment position to the point light pos
	float3 fragToLight = light.positionVS - fragPosVS;
	// The world space distance from the vertex to the light
	float distanceToLight = length(fragToLight);
	fragToLight = normalize(fragToLight);

	float3 totalColor = float3(0.f, 0.f, 0.f);
	float3 fragToCam = normalize(-fragPosVS);

	// Point light phong shading

	float diffuseCoefficient = saturate(dot(normal, fragToLight));

	float3 specularCoefficient = float3(0.f, 0.f, 0.f);
	if (diffuseCoefficient > 0.f) {

		float3 r = reflect(-fragToLight, normal);
		r = normalize(r);
		specularCoefficient = pow(saturate(dot(fragToCam, r)), specular.y) * specular.x;

	}

	//light.attConstant = 0.f;
	//light.attLinear = 0.1f;
	//light.attQuadratic = 0.02f;

	float attenuation = 1.f / (light.attConstant + light.attLinear * distanceToLight + light.attQuadratic * pow(distanceToLight, 2.f));

	totalColor += (diffuseCoefficient + specularCoefficient) * diffuse * light.color * attenuation;
	//if (attenuation > 1/255.f)
		//totalColor = float3(1.f,1.f,1.f);

	// Debug
	//totalColor = float3(1.f, 0.f, 0.f);

	return saturate(totalColor);
}

float4 PSMain(PSIn input) : SV_Target0 {

	float2 texCoords;
	texCoords.x = input.clipSpace.x / input.clipSpace.w / 2.f + 0.5f;
	texCoords.y = -input.clipSpace.y / input.clipSpace.w / 2.f + 0.5f;

	float3 viewRay = float3(input.positionVS.xy / input.positionVS.z, 1.0f);

	// Calculate projection constants (TODO: do this on the CPU)
	float nearClipDistance = 0.1f;
	float farClipDistance = 1000.f;
	float projectionA = farClipDistance / (farClipDistance - nearClipDistance);
	float projectionB = (-farClipDistance * nearClipDistance) / (farClipDistance - nearClipDistance);

	// Sample the depth and convert to linear view space Z (aPSssume it gets sampled as a floating point value of the range [0,1])
	float depth = def_texDepth.Sample(PSss, texCoords).x;
	float linearDepth = projectionB / (depth - projectionA);
	float3 positionVS = viewRay * linearDepth;
	
    float3 diffuseColor = def_texDiffuse.Sample(PSss, texCoords).rgb;

    float3 normal = def_texNormal.Sample(PSss, texCoords).rgb * 2.f - 1.f;

    float3 specular = def_texSpecular.Sample(PSss, texCoords).rgb;

    return float4(deferredPhongShading(def_pointLightInput, positionVS, diffuseColor, specular, normal), 1.0f);
	//return float4(fragToCam, 1.0f);
	//return float4(diffuseColor + float3(0.1f, 0.1f, 0.1f), 1.f);
	//return float4(tex[0].Sample(PSss, texCoords).rgb, 1.0f);
	//return float4(linearDepth / 100.f, linearDepth / 100.f, linearDepth / 100.f, 1.f);
	//return float4(1.f, 0.f, 0.f, 1.f);
}

