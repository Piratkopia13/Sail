// These should be read from material properties or textures

// static float3  albedo;
static float   metallic = 0.8f;
static float   roughness = 0.4f;
static float   ao = 1.0f;

float3 fresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
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

float4 pbrShade(float3 worldPosition, float3 worldNormal, float3 albedo) {
    float3 N = normalize(worldNormal); 
    float3 V = normalize(CB_SceneData.cameraPosition - worldPosition);

    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0        = lerp(F0, albedo, metallic);

    // Reflectance equation
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < NUM_POINT_LIGHTS; i++) {
		PointLightInput p = CB_SceneData.pointLights[i];

        float3 L = normalize(p.position - worldPosition);
        float3 H = normalize(V + L);
    
        float distance    = length(p.position - worldPosition);
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
        float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
        // Because metallic surfaces don't refract light and thus have no diffuse reflections we enforce this property by nullifying kD if the surface is metallic
        kD *= 1.0f - metallic;

        // Calculate the light's outgoing reflectance value
        float NdotL = max(dot(N, L), 0.0f);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Add the (improvised) ambient term to get the final color of the pixel
    float3 ambient = float3(0.03f, 0.03f, 0.03f) * albedo * ao;
    float3 color   = ambient + Lo;

    // Gamma correction
    color = color / (color + float3(1.0f, 1.0f, 1.0f));
    // Tone mapping using the Reinhard operator
    float q = 1.0f / 2.2f;
    color = pow(color, float3(q, q, q));

    return float4(color, 1.0f);
}