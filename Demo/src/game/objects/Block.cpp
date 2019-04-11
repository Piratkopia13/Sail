#include "Block.h"

Block::Block() {
}

Block::Block(Model *drawModel) {
	setModel(drawModel);
	m_color = glm::vec4(1.f);
}

Block::~Block() {

}

void Block::draw() {
	getModel()->setTransform(&getTransform());
	getModel()->getMaterial()->setColor(m_color);
	getModel()->draw();
}

void Block::setColor(const glm::vec4& color) {
	m_color = color;
}