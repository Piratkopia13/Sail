#include "PBR.hlsl"

float4 phongShade(float3 worldPosition, float3 worldNormal, float3 diffuseColor) {
    float3 shadedColor = float3(0.f, 0.f, 0.f);
	
	float3 ambientCoefficient = float3(0.0f, 0.0f, 0.0f);
	// TODO: read these from model data
	float shininess = 10.0f;
	float specMap = 1.0f;
	float kd = 1.0f;
	float ka = 1.0f;
	float ks = 1.0f;

	for (uint i = 0; i < NUM_POINT_LIGHTS; i++) {
		PointLightInput p = CB_SceneData.pointLights[i];

		float3 hitToLight = p.position - worldPosition;
		float distanceToLight = length(hitToLight);

		// Dont do any shading if in shadow or light is black
		if (Utils::rayHitAnything(worldPosition, normalize(hitToLight), distanceToLight) || all(p.color == 0.0f)) {
			continue;
		}

		float3 hitToCam = CB_SceneData.cameraPosition - worldPosition;

		float diffuseCoefficient = saturate(dot(worldNormal, hitToLight));

		float3 specularCoefficient = float3(0.f, 0.f, 0.f);
		if (diffuseCoefficient > 0.f) {
			float3 r = reflect(-hitToLight, worldNormal);
			r = normalize(r);
			specularCoefficient = pow(saturate(dot(normalize(hitToCam), r)), shininess) * specMap;
		}

		float attenuation = 1.f / (p.attConstant + p.attLinear * distanceToLight + p.attQuadratic * pow(distanceToLight, 2.f));

		shadedColor += (kd * diffuseCoefficient + ks * specularCoefficient) * diffuseColor * p.color * attenuation;
	}

	float3 ambient = diffuseColor * ka * ambientCoefficient;

	return float4(saturate(ambient + shadedColor), 1.0f);
}

void shade(float3 worldPosition, float3 worldNormal, float3 albedo, float metalness, float roughness, float ao, inout RayPayload payload, bool calledFromClosestHit = false, int reflectionBounces = 1, float reflectionAtt = 0.9f) {
	float3 rayDir = (calledFromClosestHit) ? WorldRayDirection() : worldPosition - CB_SceneData.cameraPosition;
	
	// payload.color = pbrShade(worldPosition, worldNormal, -rayDir, albedo, metalness, roughness, ao, payload);
	payload.color = phongShade(worldPosition, worldNormal, albedo);

	// float4 phongColor = phongShade(worldPosition, worldNormal, albedo);
	
	// if (payload.recursionDepth < reflectionBounces + 1) {
	// 	// Ray direction for first ray when cast from GBuffer must be calculated using camera position
	// 	float3 rayDir = (calledFromClosestHit) ? WorldRayDirection() : worldPosition - CB_SceneData.cameraPosition;

	// 	// Trace reflection ray
	// 	float3 reflectedDir = reflect(rayDir, worldNormal);
	// 	TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, Utils::getRayDesc(reflectedDir, worldPosition), payload);
	// 	payload.color = payload.color * (1.0f - reflectionAtt) + phongColor * reflectionAtt;
	// } else {
	// 	// Reflection ray, return color
	// 	payload.color = phongColor;
	// }
}