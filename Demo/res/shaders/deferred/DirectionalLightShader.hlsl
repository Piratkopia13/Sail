struct VSIn {
  float4 position : POSITION0;
};

struct PSIn {
  float4 position : SV_Position;
  float3 viewRay : VIEWRAY;
  float4 clipSpace : CLIPSPACE;
};

cbuffer ModelData : register(b0) {
  matrix mInvP;
}

PSIn VSMain(VSIn input) {
  PSIn output;

  input.position.w = 1.f;
	// input position is already in clip space coordinates
  output.position = input.position;
  output.clipSpace = output.position;
  
  float3 positionVS = mul(input.position, mInvP).xyz;
  output.viewRay = float3(positionVS.xy / positionVS.z, 1.f);

  return output;

}

Texture2D tex[4] : register(t0);

Texture2D lightDepthTex : register(t10);
//Texture2DArray depthTextures : register(t5);
SamplerState ss : register(s0);

//Texture2D playerCamDepthTex : register(t9);
//Texture2D lightDepthTex : register(t10);

struct LightData {
  float3 directionVS; // View space direction of directional light
  float3 color;
};

cbuffer Light : register(b0) {
  LightData lightInput;
}

SamplerState shadowSS : register(s1);

cbuffer ShadowLightBuffer : register(b1) {
  matrix mInvV;
  matrix mLightV;
  matrix mLightP;
}

float3 deferredPhongShading(LightData light, float3 fragToCam, float3 diffuse, float3 specular, float3 normal) {

  float3 totalColor = float3(0.f, 0.f, 0.f);
  fragToCam = normalize(fragToCam);

	// Directional light
  light.directionVS = normalize(light.directionVS);

  float diffuseCoefficient = saturate(dot(normal, -light.directionVS));

  float3 specularCoefficient = float3(0.f, 0.f, 0.f);
  if (diffuseCoefficient > 0.f) {

    float3 r = reflect(light.directionVS, normal);
    r = normalize(r);
    specularCoefficient = pow(saturate(dot(fragToCam, r)), specular.y) * specular.x;

  }
  totalColor += (diffuseCoefficient + specularCoefficient) * diffuse * light.color;
	
  return saturate(totalColor);
}

float calcLightValue(float3 camToFrag) {
    // The camToFrag position transformed to world space
  float4 projectedCamToFrag = mul(float4(camToFrag, 1.f), mInvV);
    //float4 projectedCamToFrag = float4(camToFrag, 1.f);
    // The camToFrag position transformed to the light's view space
  projectedCamToFrag = mul(projectedCamToFrag, mLightV);
    // The camToFrag position transformed to the light's clip space
  projectedCamToFrag = mul(projectedCamToFrag, mLightP);
    // Diving by W to go to NDC
  projectedCamToFrag.xyz /= projectedCamToFrag.w;


    // The texture coordinates ranging from 0 to 1
  float2 texCoords = projectedCamToFrag.xy * float2(0.5f, 0.5f) + float2(0.5f, 0.5f);

    // Flipping the y coordinate
  texCoords.y = 1 - texCoords.y;

    /*
        Loop index MUST be >= 1.
        loopIndex:  1 2  3  4  5   6 
        numSamples: 4 16 36 64 100 144
    */
  const int loopIndex = 2;
  const int numSamples = 4 * pow(loopIndex, 2);
    // Resolution of the texture
    //float2 smapSize = float2(16384.f, 8640.f);
  //float2 smapSize = float2(8192.f, 4320.f);
  float2 smapSize = float2(4096.f, 2160.f);
    // Size of each pixel on the texture
  float dx = 1.f / smapSize.x;
  float dy = 1.f / smapSize.y;
  float bias = 0.0001f;

    // The light coefficient to return (how lit the fragment should be, range [0,1])
  float lightCoeff = 0.f;
    // Loops through and samples points around the current 'main' point
    // The loop returns 0 if the point is occluded, 1 if it's not.
  for (float i = -loopIndex + 0.5f; i < loopIndex + 0.5f; i++)
    for (float j = -loopIndex + 0.5f; j < loopIndex + 0.5f; j++)
      lightCoeff += lightDepthTex.Sample(shadowSS, float2(texCoords.x + dx * i, texCoords.y + dy * j)).x + bias < projectedCamToFrag.z ? 0.0f : 1.0f;
    //Divide the value by how many samples were made
  lightCoeff = lightCoeff / float(numSamples);
    
    //for (float i = -1.5; i <= 0.5; i+=2)
    //    for (float j = -0.5; j <= 1.5; j+=2)
    //        lightCoeff += lightDepthTex.Sample(shadowSS, float2(texCoords.x + dx * i, texCoords.y + dy * j)).x + bias < projectedCamToFrag.z ? 0.0f : 1.0f;

    //// Removed this as it doesn't effect the result enough to make up the cost
    ////for (float i = -0.5; i <= 0.5; i++)
    //    //for (float j = -0.5; j <= 0.5; j++)
    //        //lightCoeff += lightDepthTex.Sample(shadowSS, float2(texCoords.x + dx * i, texCoords.y + dy * j)).x + bias < projectedCamToFrag.z ? 0.0f : 1.0f;

    //for (float i = -0.5; i <= 1.5; i += 2)
    //    for (float j = -1.5; j <= 0.5; j += 2)
    //        lightCoeff += lightDepthTex.Sample(shadowSS, float2(texCoords.x + dx * i, texCoords.y + dy * j)).x + bias < projectedCamToFrag.z ? 0.0f : 1.0f;

    //lightCoeff /= 8.f;

    //if (lightCoeff > 0.4f)
    //    lightCoeff = 1.f;

    // Clamps the value so the shadows don't get too dark.
  lightCoeff = clamp(lightCoeff, 0.5f, 1.f);


    // Lecture PCF
    //float s0 = lightDepthTex.Sample(shadowSS, texCoords).r + bias < projectedCamToFrag.z ? 0.0f : 1.0f;
    //float s1 = lightDepthTex.Sample(shadowSS, texCoords + float2(dx, 0.0f)).r + bias < projectedCamToFrag.z ? 0.0f : 1.0f;
    //float s2 = lightDepthTex.Sample(shadowSS, texCoords + float2(0.0f, dy)).r + bias < projectedCamToFrag.z ? 0.0f : 1.0f;
    //float s3 = lightDepthTex.Sample(shadowSS, texCoords + float2(dx, dy)).r + bias < projectedCamToFrag.z ? 0.0f : 1.0f;
    //lightCoeff = lerp(lerp(s0, s1, lerps.x), lerp(s2, s3, lerps.x), lerps.y);
    // Percentage closer filtering
    //float2 texelPos = float2(texCoords.x * smap_size.x, texCoords.y * smap_size.y);
    //float2 lerps = frac(texelPos);
    //float shadowCoeff = lerp(lerp(depthValues[0], depthValues[1], lerps.x), lerp(depthValues[2], depthValues[3], lerps.x), lerps.y);




    // THE WAR OF THE ANTS!----------
    //float s[16];
    //for (float i = -2.f; i < 2.f; i++)
        //for (float j = -2.f; j < 2.f; j++)
            //s[i] = lightDepthTex.Sample(shadowSS, texCoords).r + bias < projectedCamToFrag.z ? 0.0f : 1.0f;
    //float2 texelPos = texCoords * smapSize;

    //float2 lerps = frac(texelPos);
    //float lerp1 = lerp(s[0], s[1], lerps.x);
    //float lerp2 = lerp(s[2], s[3], lerps.x);
    //float lerp3 = lerp(s[4], s[5], lerps.x);
    //float lerp4 = lerp(s[6], s[7], lerps.x);

    //float lerp5 = lerp(s[8], lerp1, lerps.y);
    //float lerp6 = lerp(s[9], lerp2, lerps.y);
    //float lerp7 = lerp(s[10], lerp3, lerps.y);
    //float lerp8 = lerp(s[11], lerp4, lerps.y);
    
    //float lerp9 = lerp(s[12], lerp5, lerps.x);
    //float lerp10 = lerp(s[13], lerp6, lerps.x);
    //float lerp11 = lerp(s[14], lerp7, lerps.x);
    //float lerp12 = lerp(s[15], lerp8, lerps.x);

    //float lerp13 = lerp(lerp9, lerp10, lerps.x);
    //float lerp14 = lerp(lerp11, lerp12, lerps.x);

    //float lerp15 = lerp(lerp13, lerp14, lerps.y);

    //lightCoeff = lerp15;
    //--------------------
    

  return lightCoeff;
}

float4 PSMain(PSIn input) : SV_Target0 {

  float2 texCoords;
  texCoords.x = input.clipSpace.x / input.clipSpace.w / 2.f + 0.5f;
  texCoords.y = -input.clipSpace.y / input.clipSpace.w / 2.f + 0.5f;

	// Calculate projection constants (TODO: do this on the CPU)
  float nearClipDistance = 0.1f;
  float farClipDistance = 1000.f;
  float projectionA = farClipDistance / (farClipDistance - nearClipDistance);
  float projectionB = (-farClipDistance * nearClipDistance) / (farClipDistance - nearClipDistance);

	// Sample the depth and convert to linear view space Z (assume it gets sampled as a floating point value of the range [0,1])
  float depth = tex[3].Sample(ss, texCoords).x;
  float linearDepth = projectionB / (depth - projectionA);
  float3 positionVS = input.viewRay * linearDepth;

  float3 fragToCam = -positionVS;

  float shadow = calcLightValue(positionVS);

  float4 diffuseColor = tex[0].Sample(ss, texCoords);

  // Dont perform lighting on bright pixels to get the "tron" glow
  if (diffuseColor.a < 1.f)
    return float4(diffuseColor.rgb, 1.f);

  if (shadow > 0.f) {

    float3 normal = (tex[1].Sample(ss, texCoords).rgb * 2.f - 1.f);

    float3 specular = tex[2].Sample(ss, texCoords).rgb;

    return float4(deferredPhongShading(lightInput, fragToCam, diffuseColor.rgb, specular, normal) * shadow, 1.f);
  } else {
    return float4(0.f, 0.f, 0.f, 1.f);
  }
	//return float4(fragToCam, 1.0f);
	//return float4(diffuseColor + float3(0.1f, 0.1f, 0.1f), 1.f);
	//return float4(tex[0].Sample(ss, texCoords).rgb, 1.0f);
	//return float4(linearDepth / 100.f, linearDepth / 100.f, linearDepth / 100.f, 1.f);
	//return float4(1.f, 0.f, 0.f, 1.f);
}

