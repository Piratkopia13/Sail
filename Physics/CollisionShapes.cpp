#include "PhysicsPCH.h"

#include "CollisionShapes.h"
#include "Intersection.h"



CollisionTriangle::CollisionTriangle(const glm::vec3& pos0, const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& _normal) {
	m_positions[0] = pos0;
	m_positions[1] = pos1;
	m_positions[2] = pos2;
	normal = _normal;
}

glm::vec3 CollisionTriangle::getIntersectionPosition(BoundingBox* boundingBox) {
	// Calculate the plane that the triangle is on
	glm::vec3 triangleToWorldOrigo = glm::vec3(0.0f) - m_positions[0];
	float distance = -glm::dot(triangleToWorldOrigo, normal);
	return Intersection::PointProjectedOnPlane(boundingBox->getPosition(), normal, distance);
}

bool CollisionTriangle::getIntersectionDepthAndAxis(BoundingBox* boundingBox, glm::vec3* axis, float* depth) {
	if (Intersection::AabbWithTriangle(boundingBox->getPosition(), boundingBox->getHalfSize(), m_positions[0], m_positions[1], m_positions[2], axis, depth)) {
		return true;
	}
	return false;
}



CollisionAABB::CollisionAABB(const glm::vec3& position, const glm::vec3& halfSize, const glm::vec3& _normal) {
	m_position = position;
	m_halfSize = halfSize;
	normal = _normal;
}

glm::vec3 CollisionAABB::getIntersectionPosition(BoundingBox* boundingBox) {
	// Calculate the plane of bounding box
	glm::vec3 planePositionToWorldOrigo = glm::vec3(0.0f) - (m_position + m_halfSize * normal);
	float distance = -glm::dot(planePositionToWorldOrigo, normal);
	return Intersection::PointProjectedOnPlane(boundingBox->getPosition(), normal, distance);
}

bool CollisionAABB::getIntersectionDepthAndAxis(BoundingBox* boundingBox, glm::vec3* axis, float* depth) {
	if (Intersection::AabbWithAabb(boundingBox->getPosition(), boundingBox->getHalfSize(), m_position, m_halfSize, axis, depth)) {
		return true;
	}
	return false;
}