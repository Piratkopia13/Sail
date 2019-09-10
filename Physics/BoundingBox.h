#pragma once

class Entity;

class BoundingBox {
private:
	glm::vec3 m_position;
	glm::vec3 m_size;

	Entity::SPtr m_modelEntity;

public:
	BoundingBox();
	~BoundingBox();

	glm::vec3 getPosition() const;
	glm::vec3 getSize() const;

	void setPosition(glm::vec3 position);
	void setSize(glm::vec3 size);

	void setModel(Entity::SPtr modelEntity);
};