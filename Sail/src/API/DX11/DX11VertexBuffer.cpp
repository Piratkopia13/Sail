#include "pch.h"
#include "DX11API.h"
#include "DX11VertexBuffer.h"
#include "Sail/Application.h"

VertexBuffer* VertexBuffer::Create(const Mesh::Data& modelData, bool allowUpdates) {
	return SAIL_NEW DX11VertexBuffer(modelData, allowUpdates);
}

DX11VertexBuffer::DX11VertexBuffer(const Mesh::Data& modelData, bool allowUpdates)
	: VertexBuffer(modelData)
	, m_instanceBuffer(nullptr)
{
	void* vertices = mallocVertexData(modelData);

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));

	vbd.Usage = (allowUpdates) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;;
	vbd.ByteWidth = getVertexBufferSize();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = (allowUpdates) ? D3D11_CPU_ACCESS_WRITE : 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(vertexData));
	vertexData.pSysMem = vertices;

	// Create the vertex buffer
	ThrowIfFailed(Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateBuffer(&vbd, &vertexData, &m_vertBuffer));
	
	// Delete vertices from cpu memory
	free(vertices);
	
	// Set up instanceData buffer if instances are set
	if (modelData.numInstances > 0) {
		assert(false && "fix this");
		// Set up instance buffer
		/*D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.Usage = (allowUpdates) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = inputLayout.getInstanceSize() * modelData.numInstances;
		ibd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		ibd.CPUAccessFlags = (allowUpdates) ? D3D11_CPU_ACCESS_WRITE : 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		ThrowIfFailed(Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateBuffer(&ibd, nullptr, &m_instanceBuffer));*/
	}
}

DX11VertexBuffer::~DX11VertexBuffer() {
	Memory::SafeRelease(m_vertBuffer);
	Memory::SafeRelease(m_instanceBuffer);
}

ID3D11Buffer* const* DX11VertexBuffer::getBuffer() const {
	return &m_vertBuffer;
}

void DX11VertexBuffer::bind(void* cmdList) {
	if (m_instanceBuffer) {
		assert(false && "Fix this!");
		//// Bind both vertex and instance buffers
		//UINT strides[2] = { inputLayout.getVertexSize(), inputLayout.getInstanceSize() };
		//UINT offsets[2] = { 0, 0 };
		//ID3D11Buffer* bufferPtrs[2] = { m_vertBuffer, m_instanceBuffer };
		//Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->IASetVertexBuffers(0, 2, bufferPtrs, strides, offsets);
	} else {
		// Bind vertex buffer
		ID3D11Buffer* vertBuffers[5];
		UINT offsets[5];
		UINT strides[5];
		UINT sizes[5];

		sizes[0] = static_cast<UINT>(getPositionsDataSize());
		offsets[0] = 0;
		strides[0] = static_cast<UINT>(sizeof(Mesh::vec3));

		sizes[1] = static_cast<UINT>(getTexCoordsDataSize());
		offsets[1] = sizes[0];
		strides[1] = static_cast<UINT>(sizeof(Mesh::vec2));

		sizes[2] = static_cast<UINT>(getNormalsDataSize());
		offsets[2] = offsets[1] + sizes[1];
		strides[2] = static_cast<UINT>(sizeof(Mesh::vec3));

		sizes[3] = static_cast<UINT>(getTangentsDataSize());
		offsets[3] = offsets[2] + sizes[2];
		strides[3] = static_cast<UINT>(sizeof(Mesh::vec3));

		sizes[4] = static_cast<UINT>(getBitangentsDataSize());
		offsets[4] = offsets[3] + sizes[3];
		strides[4] = static_cast<UINT>(sizeof(Mesh::vec3));

		// Bind all missing attributes to the zero buffer (last 3 floats)
		for (unsigned int i = 0; i < ARRAYSIZE(vertBuffers); i++) {
			vertBuffers[i] = m_vertBuffer; // All use the same buffer
			if (sizes[i] == 0) {
				offsets[i] = getVertexBufferSize() - sizeof(float) * 3;
				strides[i] = 0;
			}
		}

		Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->IASetVertexBuffers(0, 5, vertBuffers, strides, offsets);
	}
}

void DX11VertexBuffer::update(Mesh::Data& data) {
	auto* devCon = Application::getInstance()->getAPI<DX11API>()->getDeviceContext();
	
	void* vertices = mallocVertexData(data);
	// Update vertex buffer
	D3D11_MAPPED_SUBRESOURCE resource;
	devCon->Map(m_vertBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, vertices, getVertexBufferSize());
	devCon->Unmap(m_vertBuffer, 0);
	free(vertices);
}
