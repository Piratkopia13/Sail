#include "PhysicsPCH.h"
#include <algorithm>

#include "Intersection.h"

BoundingBox Intersection::m_paddedReserved;

bool Intersection::aabbWithAabb(const BoundingBox& aabb1, const BoundingBox& aabb2) {

	glm::vec3 center1 = aabb1.getPosition();
	glm::vec3 center2 = aabb2.getPosition();
	glm::vec3 halfWidth1 = aabb1.getHalfSize();
	glm::vec3 halfWidth2 = aabb2.getHalfSize();

	if (glm::abs(center1.x - center2.x) > (halfWidth1.x + halfWidth2.x)) {
		return false;
	}
	if (glm::abs(center1.y - center2.y) > (halfWidth1.y + halfWidth2.y)) {
		return false;
	}
	if (glm::abs(center1.z - center2.z) > (halfWidth1.z + halfWidth2.z)) {
		return false;
	}
	return true;
}

bool Intersection::aabbWithTriangle(const BoundingBox& aabb, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {

	glm::vec3 center = aabb.getPosition();
	//Calculate normal for triangle
	glm::vec3 triNormal = glm::normalize(glm::cross(glm::vec3(v0 - v1), glm::vec3(v0 - v2)));

	// Calculate triangle points relative to the AABB
	glm::vec3 newV0 = v0 - center;
	glm::vec3 newV1 = v1 - center;
	glm::vec3 newV2 = v2 - center;

	//Don't intersect with triangles faceing away from the boundingBox
	if (glm::dot(newV0, triNormal) > 0.0f) {
		return false;
	}

	// Calculate the plane that the triangle is on
	glm::vec3 triangleToWorldOrigo = glm::vec3(0.0f) - v0;
	float distance = -glm::dot(triangleToWorldOrigo, triNormal);

	// Test the AABB against the plane that the triangle is on
	if (aabbWithPlane(aabb, triNormal, distance)) {
		// Testing AABB with triangle using separating axis theorem(SAT)
		glm::vec3 e[3];
		e[0] = glm::vec3(1.f, 0.f, 0.f);
		e[1] = glm::vec3(0.f, 1.f, 0.f);
		e[2] = glm::vec3(0.f, 0.f, 1.f);

		glm::vec3 f[3];
		f[0] = newV1 - newV0;
		f[1] = newV2 - newV1;
		f[2] = newV0 - newV2;

		glm::vec3 aabbSize = aabb.getHalfSize();
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				glm::vec3 a = glm::cross(e[i], f[j]);
				glm::vec3 p = glm::vec3(glm::dot(a, newV0), glm::dot(a, newV1), glm::dot(a, newV2));
				float r = aabbSize.x * glm::abs(a.x) + aabbSize.y * glm::abs(a.y) + aabbSize.z * glm::abs(a.z);
				if (glm::min(p.x, glm::min(p.y, p.z)) > r || glm::max(p.x, glm::max(p.y, p.z)) < -r) {
					return false;
				}
			}
		}
	}
	else {
		return false;
	}

	return true;
}

bool Intersection::aabbWithPlane(const BoundingBox& aabb, const glm::vec3& normal, const float& distance) {
	//Actually creates a sphere from the aabb half size and tests sphere with plane
	float radius = glm::length(aabb.getHalfSize());

	if (glm::abs(glm::dot(normal, aabb.getPosition()) - distance) <= radius) {
		return true;
	}
	return false;
}

bool Intersection::triangleWithTriangle(const glm::vec3 U[3], const glm::vec3 V[3]) {
	glm::vec3 S0[2], S1[2];
	if (triangleWithTriangleSupport(V, U, S0) && triangleWithTriangleSupport(U, V, S1))
	{
		// Theoretically, the segments lie on the same line.  A direction D
		// of the line is the Cross(NormalOf(U),NormalOf(V)).  We choose the
		// average A of the segment endpoints as the line origin.
		glm::vec3 uNormal = glm::cross(U[1] - U[0], U[2] - U[0]);
		glm::vec3 vNormal = glm::cross(V[1] - V[0], V[2] - V[0]);
		glm::vec3 D = glm::normalize(glm::cross(uNormal, vNormal));
		glm::vec3 A = 0.25f * (S0[0] + S0[1] + S1[0] + S1[1]);

		// Each segment endpoint is of the form A + t*D.  Compute the
		// t-values to obtain I0 = [t0min,t0max] for S0 and I1 = [t1min,t1max]
		// for S1.  The segments intersect when I0 overlaps I1.  Although this
		// application acts as a "test intersection" query, in fact the
		// construction here is a "find intersection" query.
		float t00 = glm::dot(D, S0[0] - A), t01 = glm::dot(D, S0[1] - A);
		float t10 = glm::dot(D, S1[0] - A), t11 = glm::dot(D, S1[1] - A);
		auto I0 = std::minmax(t00, t01);
		auto I1 = std::minmax(t10, t11);
		return (I0.second > I1.first && I0.first < I1.second);
	}
	return false;
}

bool Intersection::triangleWithTriangleSupport(const glm::vec3 U[3], const glm::vec3 V[3], glm::vec3 outSegment[2]) {
	// Compute the plane normal for triangle U.
	glm::vec3 edge1 = U[1] - U[0];
	glm::vec3 edge2 = U[2] - U[0];
	glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

	// Test whether the edges of triangle V transversely intersect the
	// plane of triangle U.
	float d[3];
	int positive = 0, negative = 0, zero = 0;
	for (int i = 0; i < 3; ++i)
	{
		d[i] = glm::dot(normal, V[i] - U[0]);
		if (d[i] > 0.0f)
		{
			++positive;
		}
		else if (d[i] < 0.0f)
		{
			++negative;
		}
		else
		{
			++zero;
		}
	}
	// positive + negative + zero == 3

	if (positive > 0 && negative > 0)
	{
		if (positive == 2)  // and negative == 1
		{
			if (d[0] < 0.0f)
			{
				outSegment[0] = (d[1] * V[0] - d[0] * V[1]) / (d[1] - d[0]);
				outSegment[1] = (d[2] * V[0] - d[0] * V[2]) / (d[2] - d[0]);
			}
			else if (d[1] < 0.0f)
			{
				outSegment[0] = (d[0] * V[1] - d[1] * V[0]) / (d[0] - d[1]);
				outSegment[1] = (d[2] * V[1] - d[1] * V[2]) / (d[2] - d[1]);
			}
			else  // d[2] < 0.0f
			{
				outSegment[0] = (d[0] * V[2] - d[2] * V[0]) / (d[0] - d[2]);
				outSegment[1] = (d[1] * V[2] - d[2] * V[1]) / (d[1] - d[2]);
			}
		}
		else if (negative == 2)  // and positive == 1
		{
			if (d[0] > 0.0f)
			{
				outSegment[0] = (d[1] * V[0] - d[0] * V[1]) / (d[1] - d[0]);
				outSegment[1] = (d[2] * V[0] - d[0] * V[2]) / (d[2] - d[0]);
			}
			else if (d[1] > 0.0f)
			{
				outSegment[0] = (d[0] * V[1] - d[1] * V[0]) / (d[0] - d[1]);
				outSegment[1] = (d[2] * V[1] - d[1] * V[2]) / (d[2] - d[1]);
			}
			else  // d[2] > 0.0f
			{
				outSegment[0] = (d[0] * V[2] - d[2] * V[0]) / (d[0] - d[2]);
				outSegment[1] = (d[1] * V[2] - d[2] * V[1]) / (d[1] - d[2]);
			}
		}
		else  // positive == 1, negative == 1, zero == 1
		{
			if (d[0] == 0.0f)
			{
				outSegment[0] = V[0];
				outSegment[1] = (d[2] * V[1] - d[1] * V[2]) / (d[2] - d[1]);
			}
			else if (d[1] == 0.0f)
			{
				outSegment[0] = V[1];
				outSegment[1] = (d[0] * V[2] - d[2] * V[0]) / (d[0] - d[2]);
			}
			else  // d[2] == 0.0f
			{
				outSegment[0] = V[2];
				outSegment[1] = (d[1] * V[0] - d[0] * V[1]) / (d[1] - d[0]);
			}
		}
		return true;
	}

	// Triangle V does not transversely intersect triangle U, although it is
	// possible a vertex or edge of V is just touching U.  In this case, we
	// do not call this an intersection.
	return false;
}

float Intersection::rayWithAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const BoundingBox& aabb) {
	float returnValue = -1.0f;
	glm::vec3 normalizedRay = glm::normalize(rayVec);
	bool noHit = false; //Boolean for early exits from the for-loop
	float tMin = -std::numeric_limits<float>::infinity(); //tMin initialized at negative infinity
	float tMax = std::numeric_limits<float>::infinity(); //tMax initialized at positive infinity
	glm::vec3 p = aabb.getPosition() - rayStart; //Vector to center off AABB
	for (int i = 0; i < 3; i++) {
		float tempH = aabb.getHalfSize()[i]; //Temporary variable to store the current half axis

		float e = p[i];
		float f = normalizedRay[i];
		if (f != 0.0f) { //Ray is not parallel to slab
			float tempF = 1 / f; //temporary variable to avoid calculating division with the (possibly) very small value f multiple times since it is an expensive calculation

			//Finds the entering and exiting points of the current slab.
			float t1 = (e + tempH) * tempF;
			float t2 = (e - tempH) * tempF;
			if (t1 > t2) { //Swaps values if t1 is bigger than t2
				float tempT = t2;
				t2 = t1;
				t1 = tempT;
			}
			if (t1 > tMin) {//Replaces tMin if t1 is bigger
				tMin = t1;
			}
			if (t2 < tMax) {//Replaces tMax if t2 is smaller
				tMax = t2;
			}
			if (tMin > tMax || tMax < 0) { //tMin > tMax - A slab was exited before all slabs had been entered. tMax < 0 - the AABB is "behind" the ray start
				//Exit test, no hit.
				i = 3;
				noHit = true;
			}
		}
		else if (-e - tempH > 0 || -e + tempH < 0) { //Ray is parallel to slab and it does not go through OBB
			//Exit test, no hit.
			i = 3;
			noHit = true;
		}
	}

	if (noHit == false) { //The AABB was hit by the ray
		if (tMin > 0) { //tMin is bigger than 0 so it is the first intersection
			returnValue = tMin;
		}
		else if (tMax > 0) { //tMin is smaller than 0 so try with tMax.
			returnValue = 0.0f; //Ray started in AABB, instant hit. Distance is 0
		}
	}
	return returnValue;
}

float Intersection::rayWithTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
	float returnValue = -1.0f;
	glm::vec3 normalizedRay = glm::normalize(rayDir); //Normalize ray direction vec just to be sure

	//Calculate triangle edges
	glm::vec3 edge0 = v2 - v1;
	glm::vec3 edge1 = v3 - v1;

	//Determines s to use in cramer's rule
	glm::vec3 cramersS = rayStart - v1;

	glm::vec4 tuvw(-1.0f);

	normalizedRay *= -1.0f;

	float determinantVal = glm::determinant(glm::mat3(normalizedRay, edge0, edge1));
	if (determinantVal != 0) { //Makes sure no division by 0
		glm::vec3 detVec(glm::determinant(glm::mat3(cramersS, edge0, edge1)), glm::determinant(glm::mat3(normalizedRay, cramersS, edge1)), glm::determinant(glm::mat3(normalizedRay, edge0, cramersS))); //Vector containing determinant of the three matrixes
		glm::vec3 tuv = detVec * (1 / determinantVal);
		tuvw = glm::vec4(tuv.x, tuv.y, tuv.z, 1 - tuv.y - tuv.z);
	}

	if (tuvw.x >= 0.0f &&
		tuvw.y >= 0.0f && tuvw.y <= 1.0f &&
		tuvw.z >= 0.0f && tuvw.z <= 1.0f &&
		tuvw.w >= 0.0f && tuvw.w <= 1.0f) { //Hit if t is not negative and if u, v, and w are all between 0 and 1)
		returnValue = tuvw.x;
	}

	return returnValue;
}

float Intersection::rayWithPaddedAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const BoundingBox& aabb, float padding) {
	float returnValue = -1.0f;
	
	if (padding != 0.0f) {
		//Add padding
		m_paddedReserved.setPosition(aabb.getPosition());
		m_paddedReserved.setHalfSize(aabb.getHalfSize() + glm::vec3(padding));

		returnValue = rayWithAabb(rayStart, rayVec, m_paddedReserved);
	}
	else {
		returnValue = rayWithAabb(rayStart, rayVec, aabb);
	}

	return returnValue;
}

float Intersection::rayWithPaddedTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, float padding) {
	float returnValue = -1.0f;
	
	glm::vec3 triangleNormal = glm::normalize(glm::cross(glm::vec3(v1 - v2), glm::vec3(v1 - v3)));

	glm::vec3 middle = (v1 + v2 + v3) / 3.0f;

	if (glm::dot(middle - rayStart, triangleNormal) < 0.0f) {
		//Only check if triangle is facing ray start
		if (padding != 0.0f) {
			//Add padding
			glm::vec3 newV1 = v1 + (glm::normalize(v1 - middle) * 20.0f + triangleNormal) * padding;
			glm::vec3 newV2 = v2 + (glm::normalize(v2 - middle) * 20.0f + triangleNormal) * padding;
			glm::vec3 newV3 = v3 + (glm::normalize(v3 - middle) * 20.0f + triangleNormal) * padding;

			returnValue = rayWithTriangle(rayStart, rayDir, newV1, newV2, newV3);
		}
		else {
			returnValue = rayWithTriangle(rayStart, rayDir, v1, v2, v3);
		}
	}
	
	return returnValue;
}