#pragma once

#include "Mesh.h"

class IndexBuffer {
public:
	static IndexBuffer* Create(Mesh::Data& modelData);
	IndexBuffer(Mesh::Data& modelData) { }
	virtual ~IndexBuffer() { }

	virtual void bind() const = 0;

protected:

private:

};