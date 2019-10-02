#include "Utils.hlsl"
#define HLSL
#include "Common_hlsl_cpp.hlsl"

RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> lOutput : register(u0);

ConstantBuffer<SceneCBuffer> CB_SceneData : register(b0, space0);
ConstantBuffer<MeshCBuffer> CB_MeshData : register(b1, space0);
StructuredBuffer<Vertex> vertices : register(t1, space0);
StructuredBuffer<uint> indices : register(t1, space1);
StructuredBuffer<float3> metaballs : register(t1, space2);

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
	payload.color = float4(0, 0, 0, 0);
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
	if (payload.recursionDepth >= 10) {
		payload.hit = 1;
		payload.color = float4(0, 0, 1, 1);
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
		shadowPayload.recursionDepth = 10;
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
	if (payload.recursionDepth >= 2) {
		payload.hit = 1;
		return;
	}

	float3 normalInWorldSpace = normalize(mul(ObjectToWorld3x4(), attribs.normal.xyz));
	float refractIndex = 0.3; // 1.333f;
	RayPayload reflect_payload = payload;
	RayPayload refract_payload = payload;
	float3 reflectVector = reflect(WorldRayDirection(), attribs.normal.xyz);
	float3 refractVector = refract(WorldRayDirection(), attribs.normal.xyz, refractIndex); //Refract index of water is 1.333, so thats what we will use.

	RayDesc reflectRaydesc = Utils::getRayDesc(reflectVector);
	RayDesc reftractRaydesc = Utils::getRayDesc(refractVector);
	reflectRaydesc.Origin += reflectRaydesc.Direction * 0.0001;
	reftractRaydesc.Origin += reftractRaydesc.Direction * 0.0001;

	TraceRay(gRtScene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF & ~0x01, 0, 0, 0, reflectRaydesc, reflect_payload);
	TraceRay(gRtScene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF &~ 0x01, 0, 0, 0, reftractRaydesc, refract_payload);

	//diffuseColor = nextBounce.color;
	float4 reflect_color = reflect_payload.color;
	reflect_color.r *= 0.5;
	reflect_color.g *= 0.5;
	//reflect_color.b *= 0.9;
	reflect_color.b += 0.1;
	saturate(reflect_color);

	float4 refract_color = refract_payload.color;
	refract_color.r *= 0.9;
	refract_color.g *= 0.9;
	//refract_color.b *= 0.9;
	refract_color.b += 0.05;
	saturate(refract_color);

	float3 hitToCam = CB_SceneData.cameraPosition - Utils::HitWorldPosition();
	float refconst = 1 - abs(dot(normalize(hitToCam), normalInWorldSpace));

	//float4 finaldiffusecolor = saturate(reflect_color * refconst + refract_color * (1 - refconst));
	//float4 finaldiffusecolor = float4(CB_SceneData.nMetaballs, CB_SceneData.nMetaballs, CB_SceneData.nMetaballs,1) * 10 / MAX_NUM_METABALLS;// saturate((reflect_color * 0.2 + refract_color) / 1.5);
	float4 finaldiffusecolor = /*float4(InstanceID() / 10.0f, 0, 1, 1);*/ saturate((reflect_color * 0.2 + refract_color) / 1.5);
	//float4 finaldiffusecolor = reflect_color;
	finaldiffusecolor.a = 1;
	/////////////////////////

#define USE_PHONG_SHADEING
#ifdef USE_PHONG_SHADEING
	float3 shadedColor = float3(0.f, 0.f, 0.f);

	float3 ambientCoefficient = float3(0.0f, 0.0f, 0.0f);
	// TODO: read these from model data
	float shininess = 100.0f;
	float specMap = 1.0f;
	float kd = 1.0f;
	float ka = 1.0f;
	float ks = 3.0f;

	for (uint i = 0; i < NUM_POINT_LIGHTS; i++) {
		PointLightInput p = CB_SceneData.pointLights[i];

		// Treat pointlights with no color as no light
		if (p.color.r == 0.f && p.color.g == 0.f && p.color.b == 0.f) {
			continue;
		}

		// Shoot a ray towards the point light to figure out if in shadow or not
		float3 towardsLight = p.position - Utils::HitWorldPosition();
		float dstToLight = length(towardsLight);

		float3 hitToLight = p.position - Utils::HitWorldPosition();
		float distanceToLight = length(hitToLight);

		float diffuseCoefficient = 1;// clamp(dot(normalInWorldSpace, hitToLight), 0.2, 1);

		float3 specularCoefficient = float3(0.f, 0.f, 0.f);
		if (diffuseCoefficient > 0.f) {
			float3 r = reflect(-hitToLight, normalInWorldSpace);
			r = normalize(r);
			specularCoefficient = pow(saturate(dot(normalize(hitToCam), r)), shininess) * specMap;
		}

		float attenuation = 1.f / (p.attConstant + p.attLinear * distanceToLight + p.attQuadratic * pow(distanceToLight, 2.f));

		shadedColor += (kd * diffuseCoefficient + ks * specularCoefficient) * finaldiffusecolor.rgb * p.color * attenuation;
	}

	float3 ambient = finaldiffusecolor.rgb * ka * ambientCoefficient;

	/////////////////////////
	payload.color = float4(shadedColor, 1);
#else
	payload.color = finaldiffusecolor;
#endif
	return;
}

bool solveQuadratic(in float a, in float b, in float c, inout float x0, inout float x1) {
	float discr = b * b - 4 * a * c;
	if (discr < 0) {
		return false;
	} else if (discr == 0) {
		x0 = x1 = -0.5 * b / a;
	} else {
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

// Calculate a magnitude of an influence from a Metaball charge.
// Return metaball potential range: <0,1>
// mbRadius - largest possible area of metaball contribution - AKA its bounding sphere.
float CalculateMetaballPotential(in float3 position, in float3 ballpos, in float ballradius) {
	float distance = length(position - ballpos) / ballradius;

	////float3 rel = (ballpos - position) / ballradius;

	if (distance <= 1) {
		/*===Lowest Quality===*/
		//return 1 / distance;

		/*===OK Quality===*/

		float t = (1 - distance * distance);
		return t * t;


		/*===Good Quality===*/
	//	float d = distance;
	//	// Quintic polynomial field function.
	//	// The advantage of this polynomial is having smooth second derivative. Not having a smooth
	//	// second derivative may result in a sharp and visually unpleasant normal vector jump.
	//	// The field function should return 1 at distance 0 from a center, and 1 at radius distance,
	//	// but this one gives f(0) = 0, f(radius) = 1, so we use the distance to radius instead.
	//	d = abs(ballradius) - d;

	//	float r = ballradius;
	//	float q2 = d / r;
	//	float q = q2 * q2 * q2;

	//	float p = 6 * q2 * q2 * q
	//		- 15 * q * q2
	//		+ 10 * q;

	//	return p;
	}
	return 0;
}

// Calculate field potential from all active metaballs.
float CalculateMetaballsPotential(in float3 position) {
	//return 1;
	float sumFieldPotential = 0;
	uint nballs = CB_SceneData.nMetaballs;

	int mid = InstanceID();
	int nWeights = 5;

	int start = mid - nWeights;
	int end = mid + nWeights;

	if (start < 0)
		start = 0;
	if (end > nballs)
		end = nballs;

	for (int i = start; i < end; i++) {
		sumFieldPotential += CalculateMetaballPotential(position, metaballs[i], 0.15f);
	}
	return sumFieldPotential;
}

// Calculate a normal via central differences.
float3 CalculateMetaballsNormal(in float3 position) {
	//return float3(0,0,0);
	//float e = 0.5773 * 0.00001;
	float e = 0.00001;
	return normalize(float3(
		CalculateMetaballsPotential(position + float3(-e, 0, 0)) -
		CalculateMetaballsPotential(position + float3(e, 0, 0)),
		CalculateMetaballsPotential(position + float3(0, -e, 0)) -
		CalculateMetaballsPotential(position + float3(0, e, 0)),
		CalculateMetaballsPotential(position + float3(0, 0, -e)) -
		CalculateMetaballsPotential(position + float3(0, 0, e))));
}

[shader("intersection")]
void IntersectionShader() {
	float startT = 0;

	RayDesc rayWorld;
	rayWorld.Origin = WorldRayOrigin();
	rayWorld.Direction = WorldRayDirection();
	rayWorld.TMax = 1000000;
	rayWorld.TMin = 0.00001;

	RayDesc rayLocal;
	rayLocal.Origin = ObjectRayOrigin();
	rayLocal.Direction = ObjectRayDirection();
	rayLocal.TMax = 1000000;
	rayLocal.TMin = 0.00001;

	ProceduralPrimitiveAttributes attr;

	////////////////////////////////
	/*find a point on the ray that are close to the metaballs and start stepping from there instead of using ObjectRayOrigin() or WorldRayOrigin() as starting point.*/
	float4 dummy;
	float val;
	if (length(ObjectRayOrigin()) > 0.2) {//TODO: CHANGE THIS
		if (intersect(rayLocal, float3(0, 0, 0), 0.2, val, dummy)) {
			startT = val;
			rayWorld.Origin += startT * rayWorld.Direction;
		}
	}

	//ReportHit(startT, 0, attr);
	//return;
	////////////////////////////////

	float tmin = 0, tmax = 1;
	unsigned int MAX_LARGE_STEPS = 16;//If these steps dont hit any metaball no hit is reported.
	unsigned int MAX_SMALL_STEPS = 32;//If a large step hit a metaball, use small steps to adjust go backwards

	//unsigned int seed = 2;
	//float t = tmin + Utils::nextRand(seed);
	float t = tmin;
	float minTStep = (tmax - tmin) / (MAX_LARGE_STEPS / 1.0f);
	unsigned int iStep = 0;

	float3 currPos = rayWorld.Origin + t * rayWorld.Direction;
	while (iStep++ < MAX_LARGE_STEPS) {
		float sumFieldPotential = CalculateMetaballsPotential(currPos); // Sum of all metaball field potentials.

		const float Threshold = 0.95f;

		if (sumFieldPotential >= Threshold) {
			float restep_step = minTStep / 2;
			int restep_step_dir = -1;

			for (int i = 0; i < MAX_SMALL_STEPS; i++) {
				t += restep_step * restep_step_dir;
				currPos = rayWorld.Origin + t * rayWorld.Direction;
				float sumFieldPotential_recomp = CalculateMetaballsPotential(currPos); // Sum of all metaball field potentials.
				if (sumFieldPotential_recomp >= Threshold) {
					restep_step *= 0.5;
					restep_step_dir = -1;
				} else {
					restep_step *= 0.8;
					restep_step_dir = 1;
				}
			}

			attr.normal = float4(CalculateMetaballsNormal(currPos), 0);
			ReportHit(t + startT, 0, attr);
			return;
		}

		t += minTStep;
		currPos = rayWorld.Origin + t * rayWorld.Direction;
	}
}