#pragma once

#include "Sail/graphics/shader/InputLayout.h"

class VertexBuffer {
public:
	VertexBuffer(const InputLayout& inputLayout, Mesh::Data& modelData) : inputLayout(inputLayout) {};
	virtual ~VertexBuffer();

	virtual void bind() const = 0;

protected:
	const InputLayout& inputLayout;

private:

};