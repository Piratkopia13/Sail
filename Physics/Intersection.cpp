#include "PhysicsPCH.h"
#include <algorithm>

#include "Intersection.h"


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

