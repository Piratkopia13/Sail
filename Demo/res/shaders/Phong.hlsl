#include "../variables.shared"

float4 phongShade(PhongInput input) {

	float3 ambientCoefficient = float3(0.3f, 0.3f, 0.3f);

	float3 totalColor = float3(0.f, 0.f, 0.f);

	input.fragToCam = normalize(input.fragToCam);
	input.normal = normalize(input.normal);

	// Directional light

	input.dirLight.direction = normalize(input.dirLight.direction);

	float diffuseCoefficient = saturate(dot(input.normal, -input.dirLight.direction));

	float3 specularCoefficient = float3(0.f, 0.f, 0.f);
	if (diffuseCoefficient > 0.f) {

		float3 r = reflect(input.dirLight.direction, input.normal);
		r = normalize(r);
		specularCoefficient = pow(saturate(dot(input.fragToCam, r)), input.mat.shininess) * input.specMap;

	}
	totalColor += (input.mat.kd * diffuseCoefficient + input.mat.ks * specularCoefficient) * input.diffuseColor.rgb * input.dirLight.color;

	// Point lights
	[unroll]
	for (int i = 0; i < NUM_POINT_LIGHTS; i++) {
		PointLight p = input.pointLights[i];

		diffuseCoefficient = saturate(dot(input.normal, normalize(p.fragToLight)));

		specularCoefficient = float3(0.f, 0.f, 0.f);
		if (diffuseCoefficient > 0.f) {

			float3 r = reflect(-normalize(p.fragToLight), input.normal);
			r = normalize(r);
			specularCoefficient = pow(saturate(dot(input.fragToCam, r)), input.mat.shininess) * input.specMap;

		}
		float distance = length(p.fragToLight);
		// UE4 attenuation
    	float attenuation = pow(saturate(1.f - pow(distance/p.attRadius, 4.f)), 2.f) / (distance * distance + 1.f);
		attenuation *= p.intensity;

		totalColor += (input.mat.kd * diffuseCoefficient + input.mat.ks * specularCoefficient) * input.diffuseColor.rgb * p.color * attenuation;
	}

	return float4(saturate(input.mat.ka * ambientCoefficient * input.diffuseColor.rgb + totalColor), 1.0f);

}