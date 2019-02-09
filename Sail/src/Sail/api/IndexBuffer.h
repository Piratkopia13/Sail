#pragma once

#include "Mesh.h"

class IndexBuffer {
public:
	IndexBuffer(Mesh::Data& modelData) { }
	virtual ~IndexBuffer() { }

	virtual void bind() const = 0;

protected:

private:

};