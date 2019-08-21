#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>

class AABB {
public:
	AABB(const glm::vec3& minPos, const glm::vec3& maxPos);
	~AABB();

	void setMinPos(const glm::vec3& minPos);
	void setMaxPos(const glm::vec3& maxPos);
	const glm::vec3& getMinPos() const;
	const glm::vec3& getMaxPos() const;
	glm::vec3 getHalfSizes() const;
	glm::vec3 getCenterPos() const;
	void updateTransform(const glm::mat4& transform);
	void updateTranslation(const glm::vec3& translation);

	bool containsOrIntersects(const AABB& other);
	bool contains(const AABB& other);

private:
	bool lessThan(const glm::vec3& first, const glm::vec3& second);
	bool greaterThan(const glm::vec3& first, const glm::vec3& second);
	float& getElementByIndex(glm::vec3& vec, int index);

private:
	glm::vec3 m_minPos, m_originalMinPos;
	glm::vec3 m_maxPos, m_originalMaxPos;

};