#pragma once

//class Entity;
#include "Sail/entities/Entity.h"
#include "Sail/graphics/Scene.h"
#include "Sail/graphics/geometry/Model.h"


class BoundingBox {
private:
	glm::vec3 m_position;
	glm::vec3 m_halfSize;

	std::vector<glm::vec3> m_corners;

	Entity::SPtr m_modelEntity;

	bool m_hasChanged;

	void updateCorners();
public:
	BoundingBox();
	~BoundingBox();

	glm::vec3 getPosition() const;
	glm::vec3 getHalfSize() const;

	const std::vector<glm::vec3>* getCorners() const;

	const bool getChange();

	void setPosition(glm::vec3 position);
	void setHalfSize(glm::vec3 size);

	void setModel(Scene* scene, Model* model);
};