#include "PhysicsPCH.h"

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
	glm::vec3 triNormal = glm::normalize(glm::cross(glm::vec3(v0 - v1), glm::vec3(v0 - v2))); // TODO: Might be wrong direction, needs testing.
	

	// Calculate triangle points relative to the AABB
	glm::vec3 newV0 = v0 - center;
	glm::vec3 newV1 = v1 - center;
	glm::vec3 newV2 = v2 - center;

	
	// Calculate the plane that the triangle is on
	float distance = -(triNormal.x + triNormal.y + triNormal.z);
	

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
				if (glm::min(p.x, glm::min(p.y, p.z)) > r || glm::max(p.x, glm::max(p.y, p.z))) {
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
	
	glm::vec3 extent = aabb.getHalfSize();
	
	float radius = extent[0] * glm::abs(normal[0]) + extent[1] * glm::abs(normal[1]) + extent[2] * glm::abs(normal[2]);// TODO: Normal might be pointed in wrong direction, needs testing.
	
	if (glm::abs(glm::dot(normal, aabb.getPosition()) - distance) <= radius) {
		return true;
	}
	return false;
}