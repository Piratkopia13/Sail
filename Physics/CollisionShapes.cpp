#include "PhysicsPCH.h"

#include "CollisionShapes.h"
#include "Intersection.h"



CollisionTriangle::CollisionTriangle(const glm::vec3& pos0, const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& normal) {
	m_positions[0] = pos0;
	m_positions[1] = pos1;
	m_positions[2] = pos2;
	m_collisionAxis = normal;
}

bool CollisionTriangle::isTrueCollision(BoundingBox* boundingBox) {
	glm::vec3 intersectionAxis;
	float intersectionDepth;

	if (Intersection::AabbWithTriangle(boundingBox->getPosition(), boundingBox->getHalfSize(), m_positions[0], m_positions[1], m_positions[2], &intersectionAxis, &intersectionDepth)) {
		if (glm::abs(glm::dot(intersectionAxis, m_collisionAxis)) > 0.98f) { //The smallest intersection is along the normal
			return true;
		}
	}
	return false;
}

glm::vec3 CollisionTriangle::getIntersectionPosition(BoundingBox* boundingBox) {
	// Calculate the plane that the triangle is on
	glm::vec3 triangleToWorldOrigo = glm::vec3(0.0f) - m_positions[0];
	float distance = -glm::dot(triangleToWorldOrigo, m_collisionAxis);
	return Intersection::PointProjectedOnPlane(boundingBox->getPosition(), m_collisionAxis, distance);
}

bool CollisionTriangle::getIntersectionDepthAndAxis(BoundingBox* boundingBox, glm::vec3* axis, float* depth) {
	if (Intersection::AabbWithTriangle(boundingBox->getPosition(), boundingBox->getHalfSize(), m_positions[0], m_positions[1], m_positions[2], axis, depth)) {
		return true;
	}
	return false;
}



CollisionAABB::CollisionAABB(const glm::vec3& position, const glm::vec3& halfSize, const glm::vec3& collisionAxis) {
	m_position = position;
	m_halfSize = halfSize;
	m_collisionAxis = collisionAxis;
}

bool CollisionAABB::isTrueCollision(BoundingBox* boundingBox) {
	glm::vec3 intersectionAxis;
	float intersectionDepth;

	if (Intersection::AabbWithAabb(boundingBox->getPosition(), boundingBox->getHalfSize(), m_position, m_halfSize, &intersectionAxis, &intersectionDepth)) {
		if (glm::abs(glm::dot(intersectionAxis, m_collisionAxis)) > 0.98f) { //The smallest intersection is along the expected axis
			return true;
		}
	}
	return false;
}

glm::vec3 CollisionAABB::getIntersectionPosition(BoundingBox* boundingBox) {
	// Calculate the plane of bounding box
	glm::vec3 planePositionToWorldOrigo = glm::vec3(0.0f) - (m_position + m_halfSize * m_collisionAxis);
	float distance = -glm::dot(planePositionToWorldOrigo, m_collisionAxis);
	return Intersection::PointProjectedOnPlane(boundingBox->getPosition(), m_collisionAxis, distance);
}

bool CollisionAABB::getIntersectionDepthAndAxis(BoundingBox* boundingBox, glm::vec3* axis, float* depth) {
	if (Intersection::AabbWithAabb(boundingBox->getPosition(), boundingBox->getHalfSize(), m_position, m_halfSize, axis, depth)) {
		return true;
	}
	return false;
}