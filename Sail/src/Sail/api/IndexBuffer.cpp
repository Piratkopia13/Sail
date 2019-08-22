#include "pch.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(Mesh::Data& modelData) {
	m_byteSize = modelData.numIndices * sizeof(unsigned int);
}

unsigned long* IndexBuffer::getIndexData(Mesh::Data& modelData) {
	ULONG* indices = SAIL_NEW ULONG[modelData.numIndices];

	// Fill the array with the model indices
	for (UINT i = 0; i < modelData.numIndices; i++) {
		indices[i] = modelData.indices[i];
	}

	return indices;
}

unsigned int IndexBuffer::getIndexDataSize() const {
	return m_byteSize;
}
