#pragma once

#include "common\Object.h"

class Block : public Object {
private:
	DirectX::SimpleMath::Vector4 m_color;

public:
	Block();
	Block(Model *drawModel);
	virtual ~Block();

	virtual void draw();
	void setColor(const DirectX::SimpleMath::Vector4& color);
};