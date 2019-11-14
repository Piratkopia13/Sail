#pragma once

class BoundingBox {
private:
	glm::vec3 m_position;
	glm::vec3 m_halfSize;

	glm::vec3 m_corners[8];

	bool m_hasChanged;
	bool m_cornersNeedUpdate;

	void updateCorners();

private:
	friend class Octree;
	const bool getChange(); //Only access this from Octree::updateRec

public:
	BoundingBox();
	~BoundingBox();

	const glm::vec3& getPosition() const;
	const glm::vec3& getHalfSize() const;


	void prepareCorners();
	const glm::vec3* getCornersWithUpdate();
	const glm::vec3* getCornersWithoutUpdate() const;


	void setPosition(const glm::vec3& position);
	void setHalfSize(const glm::vec3& size);
};