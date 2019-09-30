float3 fresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
} 
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness) {
    return F0 + (max(1.0f - roughness, F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}  

float DistributionGGX(float3 N, float3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float num   = NdotV;
    float denom = NdotV * (1.0f - k) + k;
	
    return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float4 pbrShade(float3 worldPosition, float3 worldNormal, float3 albedo, float metalness, float roughness, float ao) {
    float3 N = normalize(worldNormal); 
    float3 V = normalize(CB_SceneData.cameraPosition - worldPosition);

    float3 F0 = 0.04f;
    F0        = lerp(F0, albedo, metalness);

    // Reflectance equation
    float3 Lo = 0.0f;
    for(int i = 0; i < NUM_POINT_LIGHTS; i++) {
		PointLightInput p = CB_SceneData.pointLights[i];

        float3 L = normalize(p.position - worldPosition);
        float3 H = normalize(V + L);
        float distance = length(p.position - worldPosition);
    
        // Dont do any shading if in shadow or light is black
		if (Utils::rayHitAnything(worldPosition, L, distance) || all(p.color == 0.0f)) {
			continue;
		}

        float attenuation = 1.f / (p.attConstant + p.attLinear * distance + p.attQuadratic * distance * distance);
        float3 radiance   = p.color * attenuation;

        float3 F  = fresnelSchlick(max(dot(H, V), 0.0f), F0);

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);

        // Calculate the Cook-Torrance BDRF
        float3 numerator    = NDF * G * F;
        float  denominator  = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
        float3 specular     = numerator / max(denominator, 0.001f); // constrain the denominator to 0.001 to prevent a divide by zero in case any dot product ends up 0.0

        // The fresnel value directly corresponds to the specular contribution
        float3 kS = F;
        // The rest is the diffuse contribution (energy conserving)
        float3 kD = 1.0f - kS;
        // Because metallic surfaces don't refract light and thus have no diffuse reflections we enforce this property by nullifying kD if the surface is metallic
        kD *= 1.0f - metalness;

        // Calculate the light's outgoing reflectance value
        float NdotL = max(dot(N, L), 0.0f);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Use this when we have cube maps for irradiance, pre filtered reflections and brdfLUT
    // float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);

    // float3 kS = F;
    // float3 kD = 1.0f - kS;
    // kD *= 1.0f - metalness;	  
    
    // float3 irradiance = texture(irradianceMap, N).rgb;
    // float3 diffuse    = irradiance * albedo;
    
    // const float MAX_REFLECTION_LOD = 4.0f;
    // float3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;   
    // float2 envBRDF  = texture(brdfLUT, float2(max(dot(N, V), 0.0f), roughness)).rg;
    // float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);


    // Add the (improvised) ambient term to get the final color of the pixel
    float3 ambient = 0.0f * albedo * ao;
    float3 color   = ambient + Lo;

    // Gamma correction
    color = color / (color + 1.0f);
    // Tone mapping using the Reinhard operator
    color = pow(color, 1.0f / 2.2f);

    return float4(color, 1.0f);
}