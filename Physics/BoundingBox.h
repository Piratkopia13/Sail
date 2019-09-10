#pragma once

//#include "Sail.h"

class BoundingBox {
private:
	glm::vec3 m_position;
	glm::vec3 m_size;

public:
	BoundingBox();
	~BoundingBox();

	glm::vec3 getPosition() const;
	glm::vec3 getSize() const;

	void setPosition(glm::vec3 position);
	void setSize(glm::vec3 size);
};