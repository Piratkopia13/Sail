#include "PBR.hlsl"

float4 phongShade(float3 worldPosition, float3 worldNormal, float3 diffuseColor) {
    float3 shadedColor = float3(0.f, 0.f, 0.f);
	
	float3 ambientCoefficient = float3(0.f, 0.f, 0.f);
	// TODO: read these from model data
	float shininess = 10.0f;
	float specMap = 1.0f;
	float kd = 1.0f;
	float ka = 1.0f;
	float ks = 1.0f;

	for (uint i = 0; i < NUM_POINT_LIGHTS; i++) {
		PointLightInput p = CB_SceneData.pointLights[i];

		// Ignore point light if color is black
		if (all(p.color == 0.0f)) {
			continue;
		}

		float3 hitToLight = p.position - worldPosition;
		float distanceToLight = length(hitToLight);

		// Dont do any shading if in shadow or light is black
		if (Utils::rayHitAnything(worldPosition, normalize(hitToLight), distanceToLight)) {
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

void shade(float3 worldPosition, float3 worldNormal, float3 albedo, float metalness, float roughness, float ao, inout RayPayload payload, bool calledFromClosestHit = false, int reflectionBounces = 1, float reflectionAtt = 0.5f) {
	// Ray direction for first ray when cast from GBuffer must be calculated using camera position
	float3 rayDir = (calledFromClosestHit) ? WorldRayDirection() : worldPosition - CB_SceneData.cameraPosition;

	// =================================================
	//  Render pixel as water if close to a water point
	// =================================================

	static const float3 mapSize = float3(56.f, 10.f, 56.f);
	static const float3 arrSize = float3(WATER_GRID_X, WATER_GRID_Y, WATER_GRID_Z);
	static const float3 mapStart = float3(-3.5f, 0.0f, -3.5f);
	static const float maxRadius = 0.2f;

	float waterOpacity = 1.0f;
	bool renderWater = false;

	float3 cellWorldSize = mapSize / arrSize;
	// int3 ind = round(( (worldPosition - mapStart) / mapSize) * arrSize);
	// int3 indMin = ind;
	// int3 indMax = ind;
	int3 indMin = round(( (worldPosition - maxRadius - mapStart) / mapSize) * arrSize);
	int3 indMax = round(( (worldPosition + maxRadius - mapStart) / mapSize) * arrSize);

	float sum = 0.f;
	for (int x = indMin.x; x <= indMax.x; x++) {
		for (int y = indMin.y; y <= indMax.y; y++) {
			for (int z = indMin.z; z <= indMax.z; z++) {
				int i = Utils::to1D(int3(x,y,z), arrSize.x, arrSize.y);
				i = clamp(i, 0, WATER_ARR_SIZE - 1);
				
				float r = waterData[i] * maxRadius;
				if (r > 0.f) {
					// lOutput[launchIndex] = float4(1.0f, 0.f, 0.f, 1.0f);
					// return;
					float3 waterPointWorldPos = float3(x,y,z) * cellWorldSize + mapStart;

					float3 dstV = worldPosition - waterPointWorldPos;
					float dstSqrd = dot(dstV, dstV);
					// r = clamp(r, 0.08f, 10.f);
					sum += r / dstSqrd;
				}
			}
		}
	}

	if (sum > 10.f) {
		renderWater = true;
		waterOpacity = clamp(sqrt(sum) / 6.f, 0.f, 0.8f);
		// lOutput[launchIndex] = float4(sum / 10.0f, 0.f, 0.f, 1.0f);
		// return;
	}

	if (renderWater) {
		// Shade as water

		metalness = lerp(metalness, 1.0f, waterOpacity);
		roughness = lerp(roughness, 0.01f, waterOpacity);
		ao 		  = lerp(ao, 0.5f, waterOpacity);
		albedo = lerp(albedo, albedo * 0.8f, waterOpacity);
		// metalness = 1.0f;
		// roughness = 0.01f;
		// ao = 0.5f;
	}
	payload.color = pbrShade(worldPosition, worldNormal, -rayDir, albedo, metalness, roughness, ao, payload);
	// payload.color = phongShade(worldPosition, worldNormal, albedo);
}