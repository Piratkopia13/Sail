float4 shade(float3 worldPosition, float3 worldNormal, float4 diffuseColor) {
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

		// Treat pointlights with no color as no light
		if (p.color.r == 0.f && p.color.g == 0.f && p.color.b == 0.f) {
			continue;
		}

		// Shoot a ray towards the point light to figure out if in shadow or not
		float3 towardsLight = p.position - worldPosition;
		float dstToLight = length(towardsLight);

        RayDesc rayDesc;
        rayDesc.Origin = worldPosition;
        rayDesc.Direction = normalize(towardsLight);
        rayDesc.TMin = 0.00001;
        rayDesc.TMax = dstToLight;

		ShadowRayPayload shadowPayload;
		shadowPayload.isHit = true; // Assume hit, miss shader will set to false
		TraceRay(gRtScene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 1 /*NULL hit group*/, 0, 1 /*Shadow miss shader*/, rayDesc, shadowPayload);

		// Dont do any shading if in shadow
		if (shadowPayload.isHit) {
			continue;
		}

		float3 hitToLight = p.position - worldPosition;
		float3 hitToCam = CB_SceneData.cameraPosition - worldPosition;
		float distanceToLight = length(hitToLight);

		float diffuseCoefficient = saturate(dot(worldNormal, hitToLight));

		float3 specularCoefficient = float3(0.f, 0.f, 0.f);
		if (diffuseCoefficient > 0.f) {
			float3 r = reflect(-hitToLight, worldNormal);
			r = normalize(r);
			specularCoefficient = pow(saturate(dot(normalize(hitToCam), r)), shininess) * specMap;
		}

		float attenuation = 1.f / (p.attConstant + p.attLinear * distanceToLight + p.attQuadratic * pow(distanceToLight, 2.f));

		shadedColor += (kd * diffuseCoefficient + ks * specularCoefficient) * diffuseColor.rgb * p.color * attenuation;
	}

	float3 ambient = diffuseColor.rgb * ka * ambientCoefficient;

	return float4(saturate(ambient + shadedColor), 1.0f);
}