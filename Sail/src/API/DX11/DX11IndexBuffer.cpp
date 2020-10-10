#include "pch.h"
#include "DX11API.h"
#include "DX11IndexBuffer.h"
#include "Sail/Application.h"

IndexBuffer* IndexBuffer::Create(Mesh::Data& modelData) {
	return SAIL_NEW DX11IndexBuffer(modelData);
}

DX11IndexBuffer::DX11IndexBuffer(Mesh::Data& modelData) 
	: IndexBuffer(modelData)
{
	ULONG* indices = getIndexData(modelData);

	// Set up index buffer description
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = getIndexDataSize();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(indexData));
	indexData.pSysMem = indices;

	// Create the index buffer
	ThrowIfFailed(Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateBuffer(&ibd, &indexData, &m_buffer));
	// Delete indices from cpu memory
	Memory::SafeDeleteArr(indices);
}

DX11IndexBuffer::~DX11IndexBuffer() {
	Memory::SafeRelease(m_buffer);
}

ID3D11Buffer* const* DX11IndexBuffer::getBuffer() const {
	return &m_buffer;
}

void DX11IndexBuffer::bind(void* cmdList) {
	// TODO: is 32 bits too much for indices? nah
	Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->IASetIndexBuffer(m_buffer, DXGI_FORMAT_R32_UINT, 0);
}
