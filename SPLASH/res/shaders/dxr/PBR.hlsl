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



// TODO: replace with pre-calculated look up texture
float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N) {
    return float2(float(i)/float(N), RadicalInverse_VdC(i));
}  
float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness) {
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    float3 up        = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent   = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}  
float2 IntegrateBRDF(float NdotV, float roughness) {
    float3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0;
    float B = 0.0;

    float3 N = float3(0.0, 0.0, 1.0);

    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H  = ImportanceSampleGGX(Xi, N, roughness);
        float3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return float2(A, B);
}



float4 pbrShade(float3 worldPosition, float3 worldNormal, float3 invViewDir, float3 albedo, float metalness, float roughness, float ao, inout RayPayload payload) {
    float3 N = normalize(worldNormal); 
    float3 V = normalize(invViewDir);

    float3 F0 = 0.04f;
    F0        = lerp(F0, albedo, metalness);

    // Reflectance equation
    float3 Lo = 0.0f;
    for(int i = 0; i < NUM_POINT_LIGHTS; i++) {
		PointLightInput p = CB_SceneData.pointLights[i];

        // Ignore point light if color is black
		if (all(p.color == 0.0f)) {
			continue;
		}

        float3 L = normalize(p.position - worldPosition);
        float3 H = normalize(V + L);
        float distance = length(p.position - worldPosition);
    
        // Dont do any shading if in shadow or light is black
        if (Utils::rayHitAnything(worldPosition, L, distance)) {
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
    float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);

    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= 1.0f - metalness;	  
    
    // float3 irradiance = texture(irradianceMap, N).rgb;
    float3 irradiance = 0.01f;
    float3 diffuse    = irradiance * albedo;
    
    float3 R = reflect(-V, N);  
    // Assume roughness = 0
    // const float MAX_REFLECTION_LOD = 4.0f;
    // float3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;

    float3 ambient = 0.f;
    if (payload.recursionDepth < 2) {
		// Ray direction for first ray when cast from GBuffer must be calculated using camera position
		// float3 rayDir = (calledFromClosestHit) ? WorldRayDirection() : worldPosition - CB_SceneData.cameraPosition;

		// Trace reflection ray
        RayDesc ray;
        ray.Origin = worldPosition;
        ray.Direction = R;
        ray.TMin = 0.0001;
        ray.TMax = 1000;
		TraceRay(gRtScene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, ray, payload);
        float3 prefilteredColor = payload.color.rgb;
        // float3 prefilteredColor = 0.5f;

        // return float4(prefilteredColor, 1.0f);

        float2 envBRDF = sys_brdfLUT.SampleLevel(ss, float2(max(dot(N, V), 0.0f), roughness), 0).rg;
        // float2 envBRDF  = IntegrateBRDF(max(dot(N, V), 0.0f), 1.0f - roughness);
        float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

        ambient = (kD * diffuse + specular) * ao; 
	} else {
		// Reflection ray, return color from only direct light and irradiance
        ambient = irradiance * albedo * ao;
	}

    // Add the (improvised) ambient term to get the final color of the pixel
    float3 color = ambient + Lo;

    // Gamma correction
    color = color / (color + 1.0f);
    // Tone mapping using the Reinhard operator
    color = pow(color, 1.0f / 2.2f);

    return float4(color, 1.0f);
}