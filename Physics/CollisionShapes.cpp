#include "PhysicsPCH.h"

#include "CollisionShapes.h"
#include "Intersection.h"



CollisionTriangle::CollisionTriangle(const glm::vec3 pos0, const glm::vec3 pos1, const glm::vec3 pos2, const glm::vec3 normal) {
	m_positions[0] = pos0;
	m_positions[1] = pos1;
	m_positions[2] = pos2;
	m_normal = normal;
}

bool CollisionTriangle::isTrueCollision(BoundingBox* boundingBox) {
	glm::vec3 intersectionAxis;
	float intersectionDepth;
	float normalDepth;

	if (Intersection::AabbWithTriangle(*boundingBox, m_positions[0], m_positions[1], m_positions[2], &intersectionAxis, &intersectionDepth, &normalDepth)) {
		if (intersectionDepth == normalDepth) { //The smallest intersection is along the normal
			return true;
		}
	}
	return false;
}

glm::vec3 CollisionTriangle::getIntersectionPosition(BoundingBox* boundingBox) {
	// Calculate the plane that the triangle is on
	glm::vec3 triangleToWorldOrigo = glm::vec3(0.0f) - m_positions[0];
	float distance = -glm::dot(triangleToWorldOrigo, m_normal);
	return Intersection::PointProjectedOnPlane(boundingBox->getPosition(), m_normal, distance);
}

CollisionAABB::CollisionAABB(const glm::vec3 position, const glm::vec3 halfSize, const glm::vec3 intersectionAxis) {
	m_position = position;
	m_halfSize = halfSize;
	m_intersectionAxis = intersectionAxis;
}

bool CollisionAABB::isTrueCollision(BoundingBox* boundingBox) {
	return true;
}

glm::vec3 CollisionAABB::getIntersectionPosition(BoundingBox* boundingBox) {
	// Calculate the plane that the triangle is on
	glm::vec3 planePositionToWorldOrigo = glm::vec3(0.0f) - m_position + m_halfSize * m_intersectionAxis;
	float distance = -glm::dot(planePositionToWorldOrigo, m_intersectionAxis);
	return Intersection::PointProjectedOnPlane(boundingBox->getPosition(), m_intersectionAxis, distance);
}
