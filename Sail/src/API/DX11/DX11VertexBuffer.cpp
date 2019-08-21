#include "pch.h"
#include "DX11API.h"
#include "DX11VertexBuffer.h"
#include "Sail/Application.h"

VertexBuffer* VertexBuffer::Create(const InputLayout& inputLayout, Mesh::Data& modelData) {
	return SAIL_NEW DX11VertexBuffer(inputLayout, modelData);
}

DX11VertexBuffer::DX11VertexBuffer(const InputLayout& inputLayout, Mesh::Data& modelData)
	: VertexBuffer(inputLayout, modelData)
	, m_instanceBuffer(nullptr)
{
	void* vertices = getVertexData(modelData);

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	//vbd.ByteWidth = modelData.numVertices * sizeof(DeferredGeometryShader::Vertex);
	vbd.ByteWidth = getVertexDataSize();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(vertexData));
	vertexData.pSysMem = vertices;

	// Create the vertex buffer
	ThrowIfFailed(Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateBuffer(&vbd, &vertexData, &m_vertBuffer));
	
	// Delete vertices from cpu memory
	free(vertices);
	
	// Set up instanceData buffer if instances are set
	if (modelData.numInstances > 0) {
		// Set up instance buffer
		D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.Usage = D3D11_USAGE_DYNAMIC;
		ibd.ByteWidth = inputLayout.getInstanceSize() * modelData.numInstances;
		ibd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		ThrowIfFailed(Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateBuffer(&ibd, nullptr, &m_instanceBuffer));
	}

}


DX11VertexBuffer::~DX11VertexBuffer() {
	Memory::SafeRelease(m_vertBuffer);
	Memory::SafeRelease(m_instanceBuffer);
}

ID3D11Buffer* const* DX11VertexBuffer::getBuffer() const {
	return &m_vertBuffer;
}

void DX11VertexBuffer::bind(void* cmdList) const {
	if (m_instanceBuffer) {
		// Bind both vertex and instance buffers
		UINT strides[2] = { inputLayout.getVertexSize(), inputLayout.getInstanceSize() };
		UINT offsets[2] = { 0, 0 };
		ID3D11Buffer* bufferPtrs[2] = { m_vertBuffer, m_instanceBuffer };
		Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->IASetVertexBuffers(0, 2, bufferPtrs, strides, offsets);
	} else {
		// Bind vertex buffer
		UINT offset = 0;
		unsigned int stride = getVertexDataStride();
		Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->IASetVertexBuffers(0, 1, &m_vertBuffer, &stride, &offset);
	}
}
