#pragma once

#include "BoundingBox.h"

class Intersection {
public:
	static bool aabbWithAabb(const BoundingBox& aabb1, const BoundingBox& aabb2);
	
	static bool aabbWithTriangle(const BoundingBox& aabb, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);

	static bool aabbWithPlane(const BoundingBox& aabb, const glm::vec3& normal, const float& distance);

	static bool triangleWithTriangle(const glm::vec3 U[3], const glm::vec3 V[3]);

	static float rayWithAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const BoundingBox& aabb);

	static float rayWithTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);

	static float rayWithPaddedAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const BoundingBox& aabb, float padding);

	static float rayWithPaddedTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, float padding);

private:
	//Private constructor so an instance can't be created
	Intersection() {};
	~Intersection() {};

	static BoundingBox m_paddedReserved; //Used in rayWithPaddedAabb to avoid having to create a new bounding box every time

	static bool triangleWithTriangleSupport(const glm::vec3 U[3], const glm::vec3 V[3], glm::vec3 outSegment[2]);
};
