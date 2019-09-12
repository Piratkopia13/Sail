#pragma once

class Intersection {
private:

public:
	Intersection();
	~Intersection();

	bool aabbWithAabb(BoundingBox aabb1, BoundingBox aabb2) const;
	
	bool aabbWithTriangle(BoundingBox aabb, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) const;

	bool aabbWithPlane(BoundingBox aabb, glm::vec3 normal, float distance) const;
};
