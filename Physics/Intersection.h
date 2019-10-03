#pragma once

#include "BoundingBox.h"

class Intersection {
public:
	static bool AabbWithAabb(const BoundingBox& aabb1, const BoundingBox& aabb2);
	
	static bool AabbWithTriangle(const BoundingBox& aabb, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);

	static bool AabbWithPlane(const BoundingBox& aabb, const glm::vec3& normal, const float& distance);

	static bool TriangleWithTriangle(const glm::vec3 U[3], const glm::vec3 V[3]);

	static float RayWithAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const BoundingBox& aabb);

	static float RayWithTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);

	static float RayWithPaddedAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const BoundingBox& aabb, float padding);

	static float RayWithPaddedTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, float padding);

private:
	//Private constructor so an instance can't be created
	Intersection() {};
	~Intersection() {};

	static BoundingBox sPaddedReserved; //Used in RayWithPaddedAabb to avoid having to create a new bounding box every time

	static bool TriangleWithTriangleSupport(const glm::vec3 U[3], const glm::vec3 V[3], glm::vec3 outSegment[2]);
};
