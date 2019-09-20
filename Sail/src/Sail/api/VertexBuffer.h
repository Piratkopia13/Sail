#pragma once

#include "shader/InputLayout.h"
#include "Mesh.h"

class VertexBuffer {
public:
	static VertexBuffer* Create(const InputLayout& inputLayout, Mesh::Data& modelData);
	VertexBuffer(const InputLayout& inputLayout, Mesh::Data& modelData);;
	virtual ~VertexBuffer() {};

	virtual void bind(void* cmdList = nullptr) const = 0;
	virtual void update(Mesh::Data& data) = 0;

protected:
	void* getVertexData(Mesh::Data& modelData);
	unsigned int getVertexDataSize() const;
	unsigned int getVertexDataStride() const;
protected:
	const InputLayout& inputLayout;

private:
	unsigned int m_byteSize;
	unsigned int m_stride;

};