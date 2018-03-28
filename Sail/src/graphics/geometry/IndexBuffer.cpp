#include "IndexBuffer.h"
#include "../../api/Application.h"

IndexBuffer::IndexBuffer(Model::Data& modelData) {

	ULONG* indices = new ULONG[modelData.numIndices];

	// Fill the array with the model indices
	for (UINT i = 0; i < modelData.numIndices; i++) {
		indices[i] = modelData.indices[i];
	}

	// Set up index buffer description
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = modelData.numIndices * sizeof(UINT);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(indexData));
	indexData.pSysMem = indices;

	// Create the index buffer
	ThrowIfFailed(Application::getInstance()->getAPI()->getDevice()->CreateBuffer(&ibd, &indexData, &m_buffer));
	// Delete indices from cpu memory
	Memory::safeDeleteArr(indices);

}

IndexBuffer::~IndexBuffer() {
	Memory::safeRelease(m_buffer);
}

ID3D11Buffer* const* IndexBuffer::getBuffer() const {
	return &m_buffer;
}

void IndexBuffer::bind() const {
	// TODO: is 32 bits too much for indices?
	Application::getInstance()->getAPI()->getDeviceContext()->IASetIndexBuffer(m_buffer, DXGI_FORMAT_R32_UINT, 0);
}
