#include "pch.h"
#include "AABB.h"

AABB::AABB(const glm::vec3& minPos, const glm::vec3& maxPos)
	: m_minPos(minPos)
	, m_originalMinPos(minPos)
	, m_maxPos(maxPos)
	, m_originalMaxPos(maxPos)
{}
AABB::~AABB() {}

const glm::vec3& AABB::getMinPos() const {
	return m_minPos;
}
const glm::vec3& AABB::getMaxPos() const {
	return m_maxPos;
}

bool AABB::containsOrIntersects(const AABB& other) {

	if ((greaterThan(other.m_minPos, m_minPos) && lessThan(other.m_minPos, m_maxPos)) || (lessThan(other.m_maxPos, m_maxPos) && greaterThan(other.m_maxPos, m_minPos)))
		return true;

	return false;
}

bool AABB::contains(const AABB& other) {

	if (greaterThan(other.m_minPos, m_minPos) && lessThan(other.m_maxPos, m_maxPos))
		return true;

	return false;
}

bool AABB::lessThan(const glm::vec3& first, const glm::vec3& second) {
	return (first.x <= second.x &&
		first.y <= second.y &&
		first.z <= second.z);
}
bool AABB::greaterThan(const glm::vec3& first, const glm::vec3& second) {
	return (first.x >= second.x &&
		first.y >= second.y &&
		first.z >= second.z);
}

void AABB::updateTransform(const glm::mat4& transform) {

	glm::vec3 AMin, AMax;

	// Copy box A into min and max array.
	AMin = m_originalMinPos;
	AMax = m_originalMaxPos;

	// Begin at T.
	m_minPos = transform[3]; // Translation part of mat
	m_maxPos = transform[3];

	// Find extreme points by considering product of 
	// min and max with each component of M.
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			float a = transform[i][j] * getElementByIndex(AMin, i);
			float b = transform[i][j] * getElementByIndex(AMax, i);
			if (a < b) {
				getElementByIndex(m_minPos, j) += a;
				getElementByIndex(m_maxPos, j) += b;
			} else {
				getElementByIndex(m_minPos, j) += b;
				getElementByIndex(m_maxPos, j) += a;
			}
		}
	}

	//m_minPos = glm::vec3::Transform(m_minPos, transform);
	//m_maxPos = glm::vec3::Transform(m_maxPos, transform);
}

void AABB::updateTranslation(const glm::vec3& translation) {
	m_minPos = m_originalMinPos + translation;
	m_maxPos = m_originalMaxPos + translation;
}

void AABB::setMinPos(const glm::vec3& minPos) {
	m_minPos = minPos;
	m_originalMinPos = minPos;
}
void AABB::setMaxPos(const glm::vec3& maxPos) {
	m_maxPos = maxPos;
	m_originalMaxPos = maxPos;
}

glm::vec3 AABB::getHalfSizes() const {
	return (m_maxPos - m_minPos) / 2.f;
}
glm::vec3 AABB::getCenterPos() const {
	return (m_maxPos + m_minPos) / 2.f;
}

float& AABB::getElementByIndex(glm::vec3& vec, int index) {
	if (index == 0)
		return vec.x;
	if (index == 1)
		return vec.y;
	if (index == 2)
		return vec.z;

	return vec.x;
}