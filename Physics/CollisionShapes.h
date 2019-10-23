#pragma once

class BoundingBox;

class CollisionShape {
public:
	CollisionShape() {};
	virtual ~CollisionShape() {};

	virtual bool isTrueCollision(BoundingBox* boundingBox) = 0;
	virtual glm::vec3 getIntersectionPosition(BoundingBox* boundingBox) = 0;
	virtual bool getIntersectionDepthAndAxis(BoundingBox* boundingBox, glm::vec3* axis, float* depth) = 0;
};


class CollisionTriangle : public CollisionShape {
public:
	CollisionTriangle(const glm::vec3& pos0, const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& normal);
	~CollisionTriangle() {};

	bool isTrueCollision(BoundingBox* boundingBox);
	glm::vec3 getIntersectionPosition(BoundingBox* boundingBox);
	bool getIntersectionDepthAndAxis(BoundingBox* boundingBox, glm::vec3* axis, float* depth);

private:
	glm::vec3 m_positions[3];
	glm::vec3 m_normal;
};

class CollisionAABB : public CollisionShape {
public:
	CollisionAABB(const glm::vec3& position, const glm::vec3& halfSize, const glm::vec3& intersectionAxis);
	~CollisionAABB() {};

	bool isTrueCollision(BoundingBox* boundingBox);
	glm::vec3 getIntersectionPosition(BoundingBox* boundingBox);
	bool getIntersectionDepthAndAxis(BoundingBox* boundingBox, glm::vec3* axis, float* depth);

private:
	glm::vec3 m_position;
	glm::vec3 m_halfSize;
	glm::vec3 m_intersectionAxis;
};