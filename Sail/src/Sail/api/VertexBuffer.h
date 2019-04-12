#pragma once

#include "shader/InputLayout.h"
#include "Mesh.h"

class VertexBuffer {
public:
	static VertexBuffer* Create(const InputLayout& inputLayout, Mesh::Data& modelData);
	VertexBuffer(const InputLayout& inputLayout, Mesh::Data& modelData) : inputLayout(inputLayout) {};
	virtual ~VertexBuffer() {};

	virtual void bind() const = 0;

protected:
	const InputLayout& inputLayout;

private:

};