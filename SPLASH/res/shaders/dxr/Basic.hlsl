#include "Utils.hlsl"
#define HLSL
#include "Common_hlsl_cpp.hlsl"

RaytracingAccelerationStructure gRtScene : register(t0);
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

[shader("raygeneration")]
void rayGen() {
	float3 rayDir;
	float3 origin;

	uint2 launchIndex = DispatchRaysIndex().xy;
	// Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
	generateCameraRay(launchIndex, origin, rayDir);

	RayDesc ray;
	ray.Origin = origin;
	ray.Direction = rayDir;

	// Set TMin to a non-zero small value to avoid aliasing issues due to floating point errors
	// TMin should be kept small to prevent missing geometry at close contact areas
	ray.TMin = 0.00001;
	ray.TMax = 10000.0;

	RayPayload payload;
	payload.recursionDepth = 0;
	payload.hit = 0;
	payload.color = float4(0,0,0,0);
	TraceRay(gRtScene, 0, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);
	lOutput[launchIndex] = payload.color;

	// lOutput[launchIndex] = float4(1.0f, 0.2f, 0.2f, 1.0f);
}

[shader("miss")]
void miss(inout RayPayload payload) {
	payload.color = float4(0.01f, 0.01f, 0.01f, 1.0f);
}

float4 getColor(MeshData data, float2 texCoords) {
	float4 color = data.color;
	if (data.flags & MESH_HAS_DIFFUSE_TEX)
		color *= sys_texDiffuse.SampleLevel(ss, texCoords, 0);
	if (data.flags & MESH_HAS_NORMAL_TEX)
		color += sys_texNormal.SampleLevel(ss, texCoords, 0) * 0.1f;
	if (data.flags & MESH_HAS_SPECULAR_TEX)
		color += sys_texSpecular.SampleLevel(ss, texCoords, 0) * 0.1f;
	return color;
}

[shader("closesthit")]
void closestHitTriangle(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs) {
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

	float4 diffuseColor = getColor(CB_MeshData.data[instanceID], texCoords);
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
		TraceRay(gRtScene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, Utils::getRayDesc(normalize(towardsLight), dstToLight), shadowPayload);

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


[shader("closesthit")]
void closestHitProcedural(inout RayPayload payload, in ProceduralPrimitiveAttributes attribs) {
	payload.recursionDepth++;

	// TODO: move to shadow shader 
	// If this is the second bounce, return as hit and do nothing else
	if (payload.recursionDepth == 2) {
		payload.hit = 1;
		return;
	}

	uint instanceID = InstanceID();
	uint primitiveID = PrimitiveIndex();

	float3 normalInWorldSpace = normalize(mul(ObjectToWorld3x4(), attribs.normal.xyz));
	//float4 diffuseColor = float4(0.f, 1.f, 1.f, 1.0f);// CB_MeshData.data[instanceID].color;
	float4 diffuseColor = CB_MeshData.data[instanceID].color;
	//float4 diffuseColor = float4(sqrt(attribs.normal.a), 0, 0, 1.0f);
	//diffuseColor += float4(1,1,1,0);
	//diffuseColor *= 0.5;
	//diffuseColor.a = 1.0f;

	//payload.color = diffuseColor;
	//return;

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
		RayDesc r = Utils::getRayDesc(normalize(towardsLight), dstToLight);

		r.Origin += normalize(attribs.normal.xyz) * 0.001;
		TraceRay(gRtScene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, r, shadowPayload);

		// Dont do any shading if in shadow
		if (shadowPayload.hit == 1) {
			//diffuseColor = float4(1.f, 1.f, 1.f, 1.0f);// CB_MeshData.data[instanceID].color;
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

bool solveQuadratic(in float a, in float b, in float c, inout float x0, inout float x1) {
	float discr = b * b - 4 * a * c;
	if (discr < 0) {
		return false;
	}
	else if (discr == 0) {
		x0 = x1 = -0.5 * b / a;
	}
	else {
		float q = (b > 0) ?
			-0.5 * (b + sqrt(discr)) :
			-0.5 * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}

	if (x0 > x1) {
		float temp = x0;
		x0 = x1;
		x1 = temp;
	}

	return true;
}

bool intersect(in RayDesc ray, in float3 center, in float radius, out float t, out float4 normal) {
	float t0, t1; // solutions for t if the ray intersects 

	// analytic solution
	float3 L = ray.Origin - center;
	float a = dot(ray.Direction, ray.Direction);
	float b = 2 * dot(ray.Direction, L);
	float c = dot(L, L) - radius * radius;
	if (!solveQuadratic(a, b, c, t0, t1)) {
		return false;
	}

	if (t0 > t1) {
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}

	if (t0 < 0) {
		t0 = t1; // if t0 is negative, let's use t1 instead 
		if (t0 < 0) {
			return false; // both t0 and t1 are negative 
		}
	}

	t = t0;
	normal = float4(normalize((ray.Origin + t * normalize(ray.Direction)) - center), 0);

	return true;
}

[shader("intersection")]
void IntersectionShader()
{
	RayDesc ray;
	ray.Origin		= ObjectRayOrigin();			// mul(float4(ObjectRayOrigin(), 1), attr.bottomLevelASToLocalSpace).xyz;
	ray.Direction	= ObjectRayDirection();	// mul(ObjectRayDirection(), (float3x3) attr.bottomLevelASToLocalSpace);

	ProceduralPrimitiveAttributes attr;
	attr.normal = float4(1, 1, 1, 0);
#define Many
#ifdef Many
	//Multiple Spheres
	const int N = 3;
	float  radii[N] = { 0.6, 0.3, 0.15 };
	float3 centers[N] =
	{
		float3(-0.3, -0.3, -0.3),
		float3(0.1, 0.1, 0.4),
		float3(0.35,0.35, 0.0)
	};
	//center[0].x = (CB_MeshData.data[instanceID].color.x * 2 - 1);

	float minT = 0;
	float4 normal;
	bool hit = false;
	for (int i = 0; i < N; i++) {
		float t = 0;
		float4 tempNormal;
		if (intersect(ray, centers[i], radii[i], t, tempNormal)) {
			if (t < minT || !hit) {
				minT = t;
				normal = tempNormal;
			}
			hit = true;
		}
	}

	if (hit) {
		attr.normal = normal;
		ReportHit(minT, 0, attr);
	} else {
		//ReportHit(RayTCurrent(), 0, attr);
	}
#else
	float t = 0;
	float3 center = float3(0, 0, 0);
	float radius = 1;
	if (intersect(ray, center, radius, t, attr.normal)) {
		float thit = t;
		attr.normal.a = thit;
		ReportHit(thit, 0, attr);
	}
#endif
	
	//ReportHit(RayTCurrent(), 0, attr);
}