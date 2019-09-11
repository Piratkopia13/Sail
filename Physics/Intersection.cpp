#include "PhysicsPCH.h"
#include "BoundingBox.h"

#include "Intersection.h"


Intersection::Intersection(){}

Intersection::~Intersection(){}


bool Intersection::aabbWithTriangle(BoundingBox aabb, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) const {
	
	glm::vec3 center = aabb.getPosition();
	glm::vec3 triNormal = glm::cross(glm::vec3(v0 - v1), glm::vec3(v0 - v2)); // TODO: Calculate normal, might be wrong direction, needs testing.
	

	// Calculate triangle points relative to the AABB
	v0 = v0 - center;
	v1 = v1 - center;
	v2 = v2 - center;

	
	// Calculate the plane that the triangle is on
	float distance = -(triNormal.x + triNormal.y + triNormal.z);
	

	// Test the AABB against the plane that the triangle is on
	if (aabbWithPlane(aabb, triNormal, distance)) {

		// Testing using separating axis theorem(SAT)
		glm::vec3 e[3];
		e[0] = glm::vec3(1.f, 0.f, 0.f);
		e[1] = glm::vec3(0.f, 1.f, 0.f);
		e[2] = glm::vec3(0.f, 0.f, 1.f);
		
		glm::vec3 f[3];
		f[0] = v1 - v0;
		f[1] = v2 - v1;
		f[2] = v0 - v2;

		glm::vec3 aabbSize = aabb.getSize();
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				glm::vec3 a = glm::cross(e[i], f[j]);
				glm::vec3 p = glm::vec3(glm::dot(a, v0), glm::dot(a, v1), glm::dot(a, v2));
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

bool Intersection::aabbWithPlane(BoundingBox aabb, glm::vec3 normal, float distance) const {
	
	glm::vec3 extent = aabb.getSize();
	
	float radius = extent[0] * glm::abs(normal[0]) + extent[1] * glm::abs(normal[1]) + extent[2] * glm::abs(normal[2]);// TODO: Normal might be pointed in wrong direction, can't test at this point.
	
	if (glm::abs(glm::dot(normal, aabb.getPosition()) - distance) <= radius) {
		return true;
	}
	return false;
}