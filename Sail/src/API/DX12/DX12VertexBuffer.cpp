#include "pch.h"
#include "DX12VertexBuffer.h"
#include "Sail/Application.h"
#include "DX12Utils.h"

VertexBuffer* VertexBuffer::Create(const InputLayout& inputLayout, Mesh::Data& modelData) {
	return SAIL_NEW DX12VertexBuffer(inputLayout, modelData);
}

// TODO: Take in usage (Static or Dynamic) and create a default heap for static only
// TODO: updateData and/or setData
DX12VertexBuffer::DX12VertexBuffer(const InputLayout& inputLayout, Mesh::Data& modelData)
	: VertexBuffer(inputLayout, modelData)
{
	m_context = Application::getInstance()->getAPI<DX12API>();
	void* vertices = getVertexData(modelData);

	auto numSwapBuffers = m_context->getNumSwapBuffers();

	m_hasBeenUpdated.resize(numSwapBuffers, false);
	m_vertexBuffers.resize(numSwapBuffers);

	for (unsigned int i = 0; i < numSwapBuffers; i++) {
		m_vertexBuffers[i].Attach(DX12Utils::CreateBuffer(m_context->getDevice(), getVertexDataSize(), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties));
		m_vertexBuffers[i]->SetName(L"Vertex buffer");

		// Place verticies in the buffer
		void* pData;
		D3D12_RANGE readRange{ 0, 0 };
		ThrowIfFailed(m_vertexBuffers[i]->Map(0, &readRange, &pData));
		memcpy(pData, vertices, getVertexDataSize());
		m_vertexBuffers[i]->Unmap(0, nullptr);

	}
	// Delete vertices from cpu memory
	free(vertices);
}

DX12VertexBuffer::~DX12VertexBuffer() {
	m_context->waitForGPU();
}

void DX12VertexBuffer::bind(void* cmdList) const {
	auto frameIndex = m_context->getFrameIndex();
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = m_vertexBuffers[frameIndex]->GetGPUVirtualAddress();
	vbView.SizeInBytes = static_cast<UINT>(getVertexDataSize());
	vbView.StrideInBytes = static_cast<UINT>(getVertexDataStride());
	// Later update to just put in a buffer on the renderer to set multiple vertex buffers at once
	dxCmdList->IASetVertexBuffers(0, 1, &vbView);
}

ID3D12Resource1* DX12VertexBuffer::getBuffer() const {
	auto frameIndex = m_context->getFrameIndex();
	return m_vertexBuffers[frameIndex].Get();
}

void DX12VertexBuffer::update(Mesh::Data& data) {
	auto frameIndex = m_context->getFrameIndex();
	void* vertices = getVertexData(data);
	// Place verticies in the buffer
	void* pData;
	D3D12_RANGE readRange{ 0, 0 };
	ThrowIfFailed(m_vertexBuffers[frameIndex]->Map(0, &readRange, &pData));
	memcpy(pData, vertices, getVertexDataSize());
	m_vertexBuffers[frameIndex]->Unmap(0, nullptr);
	free(vertices);

	//for (unsigned int i = 0; i < m_context->getNumSwapBuffers(); i++) {
		m_hasBeenUpdated[frameIndex] = true;
	//}
}

bool DX12VertexBuffer::hasBeenUpdated() const {
	auto frameIndex = m_context->getFrameIndex();
	return m_hasBeenUpdated[frameIndex];
}

void DX12VertexBuffer::resetHasBeenUpdated() { 
	auto frameIndex = m_context->getFrameIndex();
	m_hasBeenUpdated[frameIndex] = false;
}
