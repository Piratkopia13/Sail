#pragma once

#include <glm/glm.hpp>
#include "common\Object.h"

class Block : public Object {
private:
	glm::vec4 m_color;

public:
	Block();
	Block(Model *drawModel);
	virtual ~Block();

	virtual void draw();
	void setColor(const glm::vec4& color);
};