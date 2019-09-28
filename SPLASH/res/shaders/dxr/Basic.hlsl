#include "Utils.hlsl"
#define HLSL
#include "Common_hlsl_cpp.hlsl"

RaytracingAccelerationStructure gRtScene : register(t0);
Texture2D<float4> sys_inTex_normals : register(t10);
Texture2D<float4> sys_inTex_diffuse : register(t11);
Texture2D<float> sys_inTex_depth : register(t12);

RWTexture2D<float4> lOutput : register(u0);

ConstantBuffer<SceneCBuffer> CB_SceneData : register(b0, space0);
ConstantBuffer<MeshCBuffer> CB_MeshData : register(b1, space0);
StructuredBuffer<Vertex> vertices : register(t1, space0);
StructuredBuffer<uint> indices : register(t1, space1);

// Texture2DArray<float4> textures : register(t2, space0);
Texture2D<float4> sys_texDiffuse : register(t2);
Texture2D<float4> sys_texNormal : register(t3);
Texture2D<float4> sys_texSpecular : register(t4);

SamplerState ss : register(s0);

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void generateCameraRay(uint2 index, out float3 origin, out float3 direction) {
	float2 xy = index + 0.5f; // center in the middle of the pixel.
	float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

	// Invert Y for DirectX-style coordinates.
	screenPos.y = -screenPos.y;

	// Unproject the pixel coordinate into a ray.
	float4 world = mul(CB_SceneData.projectionToWorld, float4(screenPos, 0, 1));

	world.xyz /= world.w;
	origin = CB_SceneData.cameraPosition.xyz;
	direction = normalize(world.xyz - origin);
}

// [shader("callable")]
// void callableTest(inout RayPayload payload) {
    
// }

[shader("raygeneration")]
void rayGen() {
	uint2 launchIndex = DispatchRaysIndex().xy;

	float2 screenTexCoord = float2(launchIndex.x, launchIndex.y) / DispatchRaysDimensions().xy;

	float2 xy = launchIndex + 0.5f; // center in the middle of the pixel.
	float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;
	// Invert Y for DirectX-style coordinates.
	screenPos.y = -screenPos.y;

	// Use G-Buffers to calculate/get world position, normal and texture coordinates for this screen pixel
	// G-Buffers contain data in world space
	float3 worldNormal = sys_inTex_normals.SampleLevel(ss, screenTexCoord, 0).rgb * 2.f - 1.f;
	float4 diffuse = sys_inTex_diffuse.SampleLevel(ss, screenTexCoord, 0);

	// ---------------------------------------------------
	// --- Calculate world position from depth texture ---

	// TODO: move calculations to cpu
	float projectionA = CB_SceneData.farZ / (CB_SceneData.farZ - CB_SceneData.nearZ);
    float projectionB = (-CB_SceneData.farZ * CB_SceneData.nearZ) / (CB_SceneData.farZ - CB_SceneData.nearZ);

	float depth = sys_inTex_depth.SampleLevel(ss, screenTexCoord, 0);
	float linearDepth = projectionB / (depth - projectionA);

    float3 screenVS = mul(CB_SceneData.clipToView, float4(screenPos, 0.f, 1.0f)).xyz;
	float3 viewRay = float3(screenVS.xy / screenVS.z, 1.f);

	// float3 viewRay = normalize(float3(screenPos, 1.0f));
	float4 vsPosition = float4(viewRay * linearDepth, 1.0f);

	float4 worldPosition = mul(CB_SceneData.viewToWorld, vsPosition);
	// ---------------------------------------------------

	//lOutput[launchIndex] = float4(depth / 1, 0.f, 0.f, 1.0f);
	// lOutput[launchIndex] = float4(worldPosition, 1.0f);
	// lOutput[launchIndex] = float4(worldPosition.xyz * 0.5f + 0.5f, 1.0f);

	//float3 rayDir;
	//float3 origin;
	// Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
	//generateCameraRay(launchIndex, origin, rayDir);

	// RayDesc ray;
	// ray.Origin = worldPosition;
	// ray.Direction = normalize(reflect(worldPosition - CB_SceneData.cameraPosition, worldNormal));
	// // Set TMin to a non-zero small value to avoid aliasing issues due to floating point errors
	// // TMin should be kept small to prevent missing geometry at close contact areas
	// ray.TMin = 0.00001;
	// ray.TMax = 10000.0;

	// RayPayload payload;
	// payload.recursionDepth = 0;
	// payload.hit = 0;
	// payload.color = float4(0,0,0,0);
	// TraceRay(gRtScene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);

	// lOutput[launchIndex] = float4(diffuse * 0.8f + payload.color.rgb * 0.2f, 1.0f);



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
        rayDesc.TMin = 0.0001;
        rayDesc.TMax = dstToLight;

		RayPayload shadowPayload;
		shadowPayload.recursionDepth = 1;
		shadowPayload.hit = 0;
		TraceRay(gRtScene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, rayDesc, shadowPayload);

		// Dont do any shading if in shadow
		if (shadowPayload.hit == 1) {
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

		shadedColor += (kd * diffuseCoefficient + ks * specularCoefficient) * diffuse.rgb * p.color * attenuation;
	}

	float3 ambient = diffuse.rgb * ka * ambientCoefficient;

	lOutput[launchIndex] = float4(saturate(ambient + shadedColor), 1.0f);


}

[shader("miss")]
void miss(inout RayPayload payload) {
	payload.color = float4(0.01f, 0.01f, 0.01f, 1.0f);
}

float4 getColor(MeshData data, float2 texCoords) {
	float4 color = data.color;
	if (data.flags & MESH_HAS_DIFFUSE_TEX)
		color *= sys_texDiffuse.SampleLevel(ss, texCoords, 0);		
	// if (data.flags & MESH_HAS_NORMAL_TEX)
	// 	color += sys_texNormal.SampleLevel(ss, texCoords, 0) * 0.1f;
	// if (data.flags & MESH_HAS_SPECULAR_TEX)
	// 	color += sys_texSpecular.SampleLevel(ss, texCoords, 0) * 0.1f;
	return color;
}

[shader("closesthit")]
void closestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs) {
	payload.recursionDepth++;

	// TODO: move to shadow shader 
	// If this is the second bounce, return as hit and do nothing else
	if (payload.recursionDepth == 2) {
		payload.hit = 1;
		return;
	}

	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	uint instanceID = InstanceID();
	uint primitiveID = PrimitiveIndex();

	uint verticesPerPrimitive = 3;
	uint i1 = primitiveID * verticesPerPrimitive;
	uint i2 = primitiveID * verticesPerPrimitive + 1;
	uint i3 = primitiveID * verticesPerPrimitive + 2;
	// Use indices if available
	if (CB_MeshData.data[instanceID].flags & MESH_USE_INDICES) {
		i1 = indices[i1];
		i2 = indices[i2];
		i3 = indices[i3];
	}
	Vertex vertex1 = vertices[i1];
	Vertex vertex2 = vertices[i2];
	Vertex vertex3 = vertices[i3];

	float3 normalInLocalSpace = Utils::barrypolation(barycentrics, vertex1.normal, vertex2.normal, vertex3.normal);
	float3 normalInWorldSpace = normalize(mul(ObjectToWorld3x4(), normalInLocalSpace));
	float2 texCoords = Utils::barrypolation(barycentrics, vertex1.texCoords, vertex2.texCoords, vertex3.texCoords);

	// if (payload.recursionDepth < 1) {
	// 	float3 reflectedDir = reflect(WorldRayDirection(), normalInWorldSpace);
	// 	TraceRay(gRtScene, 0, 0xFF, 0, 0, 0, Utils::getRayDesc(reflectedDir), payload);
	// 	payload.color = payload.color * 0.2f + getColor(CB_MeshData.data[instanceID], texCoords) * 0.8f;

	// } else {
	// 	// Max recursion, return color
	// 	payload.color = getColor(CB_MeshData.data[instanceID], texCoords);
	// }

	float4 diffuseColor = getColor(CB_MeshData.data[instanceID], texCoords);

	payload.color = diffuseColor;
	return;

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
		float3 towardsLight = p.position - Utils::HitWorldPosition();
		float dstToLight = length(towardsLight);

		RayPayload shadowPayload;
		shadowPayload.recursionDepth = 1;
		shadowPayload.hit = 0;
		TraceRay(gRtScene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, Utils::getRayDesc(normalize(towardsLight), dstToLight), shadowPayload);

		// Dont do any shading if in shadow
		if (shadowPayload.hit == 1) {
			continue;
		}

		float3 hitToLight = p.position - Utils::HitWorldPosition();
		float3 hitToCam = CB_SceneData.cameraPosition - Utils::HitWorldPosition();
		float distanceToLight = length(hitToLight);

		float diffuseCoefficient = saturate(dot(normalInWorldSpace, hitToLight));

		float3 specularCoefficient = float3(0.f, 0.f, 0.f);
		if (diffuseCoefficient > 0.f) {
			float3 r = reflect(-hitToLight, normalInWorldSpace);
			r = normalize(r);
			specularCoefficient = pow(saturate(dot(normalize(hitToCam), r)), shininess) * specMap;
		}

		float attenuation = 1.f / (p.attConstant + p.attLinear * distanceToLight + p.attQuadratic * pow(distanceToLight, 2.f));

		shadedColor += (kd * diffuseCoefficient + ks * specularCoefficient) * diffuseColor.rgb * p.color * attenuation;
	}

	float3 ambient = diffuseColor.rgb * ka * ambientCoefficient;

	payload.color = float4(saturate(ambient + shadedColor), 1.0f);


}
