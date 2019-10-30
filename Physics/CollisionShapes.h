#pragma once

class BoundingBox;

class CollisionShape {
public:
	CollisionShape() : normal(glm::vec3(0.0f)) {};
	virtual ~CollisionShape() {};

	virtual glm::vec3 getIntersectionPosition(const BoundingBox* boundingBox) = 0;
	virtual bool getIntersectionDepthAndAxis(const BoundingBox* boundingBox, glm::vec3* axis, float* depth) = 0;

	virtual glm::vec3 getNormal() { return normal; };

	int keeperTracker = 1;

protected:
	glm::vec3 normal;
};


class CollisionTriangle : public CollisionShape {
public:
	CollisionTriangle(const glm::vec3& pos0, const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& _normal);
	~CollisionTriangle() {};

	glm::vec3 getIntersectionPosition(const BoundingBox* boundingBox);
	bool getIntersectionDepthAndAxis(const BoundingBox* boundingBox, glm::vec3* axis, float* depth);

private:
	glm::vec3 m_positions[3];
};

class CollisionAABB : public CollisionShape {
public:
	CollisionAABB(const glm::vec3& position, const glm::vec3& halfSize, const glm::vec3& _normal);
	~CollisionAABB() {};

	glm::vec3 getIntersectionPosition(const BoundingBox* boundingBox);
	bool getIntersectionDepthAndAxis(const BoundingBox* boundingBox, glm::vec3* axis, float* depth);

private:
	glm::vec3 m_position;
	glm::vec3 m_halfSize;
};