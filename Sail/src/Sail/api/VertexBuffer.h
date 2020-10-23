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

protected:
	void* mallocVertexData(const Mesh::Data& modelData);
	uint32_t getPositionsDataSize() const;
	uint32_t getTexCoordsDataSize() const;
	uint32_t getNormalsDataSize() const;
	uint32_t getTangentsDataSize() const;
	uint32_t getBitangentsDataSize() const;
	
	uint32_t getVertexBufferSize() const;

private:
	void* m_vertices;
	uint32_t m_positionsByteSize;
	uint32_t m_texCoordsByteSize;
	uint32_t m_normalsByteSize;
	uint32_t m_tangentsByteSize;
	uint32_t m_bitangentsByteSize;
	uint32_t m_byteSize;
};