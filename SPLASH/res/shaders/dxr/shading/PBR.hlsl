#define RENDER_SHADOWS_SEPARATELY

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

float4 pbrShade(float3 worldPosition, float3 worldNormal, float3 invViewDir, float3 albedo, float metalness, float roughness, float ao) {
    float3 N = normalize(worldNormal); 
    float3 V = normalize(invViewDir);

    float3 F0 = 0.04f;
    F0        = lerp(F0, albedo, metalness);

    // Initialize a random seed
    // TODO: move this somewhere else and pass it in as a param
	// uint randSeed = Utils::initRand( DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x, CB_SceneData.frameCount );
    // float shadowAmount = 0.f;

    // Reflectance equation
    float3 Lo = 0.0f;
    for(int i = 0; i < NUM_POINT_LIGHTS; i++) {
		PointLightInput p = pointLights[i];

        // Ignore point light if color is black
		if (all(p.color == 0.0f)) {
			continue;
		}

        float3 toLight = normalize(p.position - worldPosition);
        float lightRadius = 0.08f;
        // Calculate a vector perpendicular to L
        float3 perpL = cross(toLight, float3(0.f, 1.0f, 0.f));
        // Handle case where L = up -> perpL should then be (1,0,0)
        if (all(perpL == 0.0f)) {
            perpL.x = 1.0;
        }
        // Use perpL to get a vector from worldPosition to the edge of the light sphere
        float3 toLightEdge = normalize((p.position+perpL*lightRadius) - worldPosition);
        // Angle between L and toLightEdge. Used as the cone angle when sampling shadow rays
        float coneAngle = acos(dot(toLight, toLightEdge)) * 2.0f;
        float3 H = normalize(V + toLight);
        float distance = length(p.position - worldPosition);

        float lightShadowAmount = 1.f;
        // Dont do any shading if in shadow
        if (lightShadowAmount == 0.f) {
            continue;
        }

        float attenuation = 1.f / (p.attConstant + p.attLinear * distance + p.attQuadratic * distance * distance);
        float3 radiance   = p.color * attenuation;

        float3 F  = fresnelSchlick(max(dot(H, V), 0.0f), F0);

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, toLight, roughness);

        // Calculate the Cook-Torrance BDRF
        float3 numerator    = NDF * G * F;
        float  denominator  = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, toLight), 0.0f);
        float3 specular     = numerator / max(denominator, 0.001f); // constrain the denominator to 0.001 to prevent a divide by zero in case any dot product ends up 0.0
        specular *= attenuation;

        // The fresnel value directly corresponds to the specular contribution
        float3 kS = F;
        // The rest is the diffuse contribution (energy conserving)
        float3 kD = 1.0f - kS;
        // Because metallic surfaces don't refract light and thus have no diffuse reflections we enforce this property by nullifying kD if the surface is metallic
        kD *= 1.0f - metalness;

        // Calculate the light's outgoing reflectance value
        float NdotL = max(dot(N, toLight), 0.0f);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * lightShadowAmount;
    }

    // Use this when we have cube maps for irradiance, pre filtered reflections and brdfLUT
    float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);

    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= 1.0f - metalness;	  
    
    // Assume constant irradiance since game is played indoors with no or few indirect light sources
    // This could be read from a skymap or irradiance probe
    float3 irradiance = 0.01f;
    float3 diffuse    = irradiance * albedo;
    
    float3 R = reflect(-V, N);  

    // Reflection
    // float3 prefilteredColor = albedoBounceTwo;
    float3 prefilteredColor = float3(0.8f, 0.1f, 0.8f);

    // Assume roughness = 0
    // This is a very rough approximation used because ray traced reflections are always perfect
    // and blurring them to make accurate PBR calculations is very expensive in real-time
    float2 envBRDF = brdfLUT.SampleLevel(PSss, float2(max(dot(N, V), 0.0f), roughness), 0).rg;
    float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    float3 ambient = (kD * diffuse + specular) * ao;

    // Add the (improvised) ambient term to get the final color of the pixel
    float3 color = ambient + Lo;

    // Gamma correction
    color = color / (color + 1.0f);
    // Tone mapping using the Reinhard operator
    color = pow(color, 1.0f / 2.2f);

    return float4(color, 1.0f);
}