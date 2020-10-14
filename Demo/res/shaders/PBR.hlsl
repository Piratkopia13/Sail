#include "../variables.shared"

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
    denom = M_PI * denom * denom;
	
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

inline float3 shadeWithLight(ShaderShared::PointLight light, float3 N, float3 V, float3 F0, float3 albedo, float metalness, float roughness) {
    float3 L = normalize(light.fragToLight);
    float3 H = normalize(V + L);

    float distance = length(light.fragToLight);
    // UE4 attenuation
    float attenuation = pow(saturate(1.f - pow(distance/light.attRadius, 4.f)), 2.f) / (distance * distance + 1.f);
    // An intensity of anything else than 1 will not be physically accurate, this is up to the artist
    attenuation *= light.intensity;
    
    float3 radiance   = light.color * attenuation;

    float3 F  = fresnelSchlick(max(dot(H, V), 0.0f), F0);

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);

    // Calculate the Cook-Torrance BDRF
    float3 numerator    = NDF * G * F;
    float  denominator  = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
    float3 specular     = numerator / max(denominator, 0.001f); // constrain the denominator to 0.001 to prevent a divide by zero in case any dot product ends up 0.0
    specular *= attenuation;

    // The fresnel value directly corresponds to the specular contribution
    float3 kS = F;
    // The rest is the diffuse contribution (energy conserving)
    float3 kD = 1.0f - kS;
    // Because metallic surfaces don't refract light and thus have no diffuse reflections we enforce this property by nullifying kD if the surface is metallic
    kD *= 1.0f - metalness;

    // Calculate the light's outgoing reflectance value
    float NdotL = max(dot(N, L), 0.0f);
    return (kD * albedo / M_PI + specular) * radiance * NdotL;
}

// PBR input structs
struct PBRScene {
    ShaderShared::DirectionalLight dirLight;
    ShaderShared::PointLight pointLights[NUM_POINT_LIGHTS];
    Texture2D<float4> brdfLUT;
    TextureCube<float4> irradianceMap;
    TextureCube<float4> prefilterMap;
    SamplerState linearSampler;
    SamplerState pointSampler;
};
struct PBRPixel {
    float3 camPos;
    float3 worldPos;
    float3 worldNormal;
    float3 albedo;
    float metalness;
    float roughness;
    float ao;
    bool inShadow;
};

float3 pbrShade(PBRScene scene, PBRPixel pixel) {
    float3 N = normalize(pixel.worldNormal); 
    float3 V = normalize(pixel.camPos - pixel.worldPos);

    float3 F0 = 0.04f;
    F0        = lerp(F0, pixel.albedo, pixel.metalness);

    // Reflectance equation
    float3 Lo = 0.0f;

    // Do lighting from all light sources
    {
        // Directional lights
        ShaderShared::DirectionalLight dl = scene.dirLight;
        // Ignore light if color is black or pixel is in shadow
        if (!(all(dl.color == 0.0f) || pixel.inShadow)) {
            // Set up a point light that emulate a directional light
            ShaderShared::PointLight pl;
            pl.color = dl.color;
            pl.intensity = dl.intensity;
            pl.fragToLight = -dl.direction;
            pl.attRadius = 1e10f;
            Lo += shadeWithLight(pl, N, V, F0, pixel.albedo, pixel.metalness, pixel.roughness);
        }

        // Point lights
        ShaderShared::PointLight p;
        for(unsigned int i = 0; i < NUM_POINT_LIGHTS; i++) {
            p = scene.pointLights[i];
            // Ignore point light if color is black or intensity is zero
            if (p.intensity == 0.f || all(p.color == 0.0f)) {
                continue;
            }
            // Ignore lights that are too far away
            if (length(p.fragToLight) > p.attRadius) {
                continue;
            }

            Lo += shadeWithLight(p, N, V, F0, pixel.albedo, pixel.metalness, pixel.roughness);
        }
    }

    // Use this when we have cube maps for irradiance, pre filtered reflections and brdfLUT
    float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, pixel.roughness);
    
    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= 1.0f - pixel.metalness;
    
    float3 irradiance = scene.irradianceMap.SampleLevel(scene.linearSampler, N, 0).rgb;
    float3 diffuse    = irradiance * pixel.albedo;
    
    float3 R = reflect(-V, N);

    // Reflection
    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = scene.prefilterMap.SampleLevel(scene.linearSampler, R, pixel.roughness * MAX_REFLECTION_LOD).rgb;

    float2 envBRDF  = scene.brdfLUT.SampleLevel(scene.pointSampler, float2(max(dot(N, V), 0.0f), pixel.roughness), 0).rg;

    float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    float3 ambient = (kD * diffuse + specular) * pixel.ao;

    // Add the (improvised) ambient term to get the final color of the pixel
    float3 color = ambient + Lo;

    return color;
}