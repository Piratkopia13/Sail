#include "pch.h"
#include "VertexBuffer.h"
#include "Sail/utils/Utils.h"

VertexBuffer::VertexBuffer(const Mesh::Data& modelData)
	: m_vertices(nullptr)
{
	m_positionsByteSize = (modelData.positions) ? sizeof(Mesh::vec3) * modelData.numVertices : 0;
	m_texCoordsByteSize = (modelData.texCoords) ? sizeof(Mesh::vec2) * modelData.numVertices : 0;
	m_normalsByteSize = (modelData.normals) ? sizeof(Mesh::vec3) * modelData.numVertices : 0;
	m_tangentsByteSize = (modelData.tangents) ? sizeof(Mesh::vec3) * modelData.numVertices : 0;
	m_bitangentsByteSize = (modelData.bitangents) ? sizeof(Mesh::vec3) * modelData.numVertices : 0;
	
	m_byteSize = m_positionsByteSize + m_texCoordsByteSize + m_normalsByteSize + m_tangentsByteSize + m_bitangentsByteSize 
				 + sizeof(float) * 3; // Extra bytes used for shaders trying to access data not available in the mesh

	m_stride = (m_positionsByteSize + m_texCoordsByteSize + m_normalsByteSize + m_tangentsByteSize + m_bitangentsByteSize) / modelData.numVertices;
}

void* VertexBuffer::mallocVertexData(const Mesh::Data& modelData) {
	void* vertices = malloc(m_byteSize);
	char* addr = (char*)vertices;
	if (modelData.positions) {
		memcpy(addr, &modelData.positions->vec, m_positionsByteSize);
		addr += m_positionsByteSize;
	}
	if (modelData.texCoords) {
		memcpy(addr, &modelData.texCoords->vec, m_texCoordsByteSize);
		addr += m_texCoordsByteSize;
	} 
	if (modelData.normals) {
		memcpy(addr, &modelData.normals->vec, m_normalsByteSize);
		addr += m_normalsByteSize;
	}
	if (modelData.tangents) {
		memcpy(addr, &modelData.tangents->vec, m_tangentsByteSize);
		addr += m_tangentsByteSize;
	}
	if (modelData.bitangents) {
		memcpy(addr, &modelData.bitangents->vec, m_bitangentsByteSize);
		addr += m_bitangentsByteSize;
	}
	// Set last floats to 0, this will be bound to any shader inputs that tries to access data not available in the mesh
	memset(addr, 0.f, sizeof(float) * 3);

	return vertices;
}

unsigned int VertexBuffer::getPositionsDataSize() const {
	return m_positionsByteSize;
}

unsigned int VertexBuffer::getTexCoordsDataSize() const {
	return m_texCoordsByteSize;
}

unsigned int VertexBuffer::getNormalsDataSize() const {
	return m_normalsByteSize;
}

unsigned int VertexBuffer::getTangentsDataSize() const {
	return m_tangentsByteSize;
}

unsigned int VertexBuffer::getBitangentsDataSize() const {
	return m_bitangentsByteSize;
}

unsigned int VertexBuffer::getVertexBufferSize() const {
	return m_byteSize;
}

uint32_t VertexBuffer::getStride() const {
	return m_stride;
}
