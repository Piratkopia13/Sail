#pragma once
#include "PhysicsPCH.h"

class BoundingBox {
private:
	glm::vec3 m_position;
	glm::vec3 m_halfSize;

	std::vector<glm::vec3> m_corners;

	bool m_hasChanged;

	void updateCorners();

private:
	friend class Octree;
	const bool getChange(); //Only access this from Octree::updateRec

public:
	BoundingBox();
	~BoundingBox();

	const glm::vec3& getPosition() const;
	const glm::vec3& getHalfSize() const;

	const std::vector<glm::vec3>* getCorners() const;

	void setPosition(const glm::vec3& position);
	void setHalfSize(const glm::vec3& size);
};