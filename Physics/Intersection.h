#pragma once

#include "BoundingBox.h"
#include "Cylinder.h"

class Intersection {
public:
	struct LineWithCirclePoints {
		float small;
		float big;
	};

	static bool aabbWithAabb(const BoundingBox& aabb1, const BoundingBox& aabb2);
	static bool aabbWithTriangle(const BoundingBox& aabb, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
	static bool aabbWithPlane(const BoundingBox& aabb, const glm::vec3& normal, const float& distance);
	static bool aabbWithVerticalCylinder(const BoundingBox& aabb, const VerticalCylinder& cyl);

	static bool triangleWithTriangle(const glm::vec3 U[3], const glm::vec3 V[3]);
	static bool triangleWithVerticalCylinder(const glm::vec3 tri[3], const VerticalCylinder& cyl);

	static bool pointWithVerticalCylinder(const glm::vec3 p, const VerticalCylinder& cyl);

	static bool lineSegmentWithVerticalCylinder(const glm::vec3& start, const glm::vec3& end, const VerticalCylinder& cyl);

	static float rayWithAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const BoundingBox& aabb);
	static float rayWithTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
	static float rayWithVerticalCylinder(const glm::vec3& rayStart, const glm::vec3& rayVec, const VerticalCylinder& cyl);

private:
	//Private constructor so an instance can't be created
	Intersection() {};
	~Intersection() {};

	static bool triangleWithTriangleSupport(const glm::vec3 U[3], const glm::vec3 V[3], glm::vec3 outSegment[2]);
};
