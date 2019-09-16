#pragma once

#include "BoundingBox.h"

namespace Intersection {
	bool aabbWithAabb(const BoundingBox& aabb1, const BoundingBox& aabb2);
	
	bool aabbWithTriangle(const BoundingBox& aabb, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);

	bool aabbWithPlane(const BoundingBox& aabb, const glm::vec3& normal, const float& distance);
};
