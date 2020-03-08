#pragma once

#include "shader/InputLayout.h"
#include "Mesh.h"

class VertexBuffer {
public:
	static VertexBuffer* Create(const Mesh::Data& modelData, bool allowUpdates = false);

	VertexBuffer(const Mesh::Data& modelData);
	virtual ~VertexBuffer() {};

	virtual void bind(void* cmdList = nullptr) = 0;
	virtual void update(Mesh::Data& data) = 0;

	unsigned int getPositionsOffset() const;
	unsigned int getTexCoordsOffset() const;
	unsigned int getNormalsOffset() const;
	unsigned int getTangentsOffset() const;
	unsigned int getBitangentsOffset() const;

protected:
	void* mallocVertexData(const Mesh::Data& modelData);
	unsigned int getPositionsDataSize() const;
	unsigned int getTexCoordsDataSize() const;
	unsigned int getNormalsDataSize() const;
	unsigned int getTangentsDataSize() const;
	unsigned int getBitangentsDataSize() const;
	
	unsigned int getVertexBufferSize() const;

private:
	void* m_vertices;
	unsigned int m_positionsByteSize;
	unsigned int m_texCoordsByteSize;
	unsigned int m_normalsByteSize;
	unsigned int m_tangentsByteSize;
	unsigned int m_bitangentsByteSize;
	unsigned int m_byteSize;
};