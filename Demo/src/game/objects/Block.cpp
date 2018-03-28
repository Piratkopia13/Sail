#include "Block.h"

Block::Block() {
}

Block::Block(Model *drawModel) {
	setModel(drawModel);
	m_color = DirectX::SimpleMath::Vector4(1.f, 1.f, 1.f, 1.f);
}

Block::~Block() {

}

void Block::draw() {
	getModel()->setTransform(&getTransform());
	getModel()->getMaterial()->setColor(m_color);
	getModel()->draw();
}

void Block::setColor(const DirectX::SimpleMath::Vector4& color) {
	m_color = color;
}