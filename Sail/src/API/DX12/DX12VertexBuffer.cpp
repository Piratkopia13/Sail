#include "pch.h"
#include "DX12VertexBuffer.h"
#include "Sail/Application.h"
#include "DX12Utils.h"

VertexBuffer* VertexBuffer::Create(const InputLayout& inputLayout, Mesh::Data& modelData) {
	return new DX12VertexBuffer(inputLayout, modelData);
}

// TODO: Take in usage (Static or Dynamic) and create a default heap for static only
// TODO: updateData and/or setData
DX12VertexBuffer::DX12VertexBuffer(const InputLayout& inputLayout, Mesh::Data& modelData)
	: VertexBuffer(inputLayout, modelData)
{
	DX12API* context = Application::getInstance()->getAPI<DX12API>();
	void* vertices = getVertexData(modelData);

	m_vertexBuffer = DX12Utils::CreateBuffer(context->getDevice(), getVertexDataSize(), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties);
	m_vertexBuffer->SetName(L"Vertex buffer");

	//ThrowIfFailed(context->getDevice()->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//	D3D12_HEAP_FLAG_NONE,
	//	&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
	//	D3D12_RESOURCE_STATE_COMMON,
	//	nullptr,
	//	IID_PPV_ARGS(m_vertexBuffer.GetAddressOf())));

}

DX12VertexBuffer::~DX12VertexBuffer() {

}

void DX12VertexBuffer::bind(void* cmdList) const {
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = static_cast<UINT>(getVertexDataSize());
	vbView.StrideInBytes = static_cast<UINT>(getVertexDataStride());
	// Later update to just put in a buffer on the renderer to set multiple vertex buffers at once
	dxCmdList->IASetVertexBuffers(0, 1, &vbView);
}
