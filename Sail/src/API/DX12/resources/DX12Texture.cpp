#include "pch.h"
#include "DX12Texture.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"

Texture* Texture::Create(const std::string& filename) {
	return new DX12Texture(filename);
}

DX12Texture::DX12Texture(const std::string& filename) 
	: m_cpuDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1)
	, m_isInitialized(false)
	, m_textureData(getTextureData(filename))
{
	m_context = Application::getInstance()->getAPI<DX12API>();

	m_textureDesc = {};
	m_textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO: read this from texture data
	m_textureDesc.Width = m_textureData.getWidth();
	m_textureDesc.Height = m_textureData.getHeight();
	m_textureDesc.DepthOrArraySize = 1;
	m_textureDesc.MipLevels = 1;
	m_textureDesc.SampleDesc.Count = 1;
	m_textureDesc.SampleDesc.Quality = 0;
	m_textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	m_textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	// A texture rarely updates its data, if at all, so it is stored in a default heap
	m_state = D3D12_RESOURCE_STATE_COPY_DEST;
	ThrowIfFailed(m_context->getDevice()->CreateCommittedResource(&DX12Utils::sDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &m_textureDesc, m_state, nullptr, IID_PPV_ARGS(&m_textureDefaultBuffer)));
	m_textureDefaultBuffer->SetName((std::wstring(L"Texture default buffer for ") + std::wstring(filename.begin(), filename.end())).c_str());

	// Store the cpu descriptor handle that will contain the srv for this texture
	m_heapCDH = m_cpuDescHeap.getCPUDescriptorHandleForIndex(0);

	// Create a shader resource view (descriptor that points to the texture and describes it)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	m_context->getDevice()->CreateShaderResourceView(m_textureDefaultBuffer.Get(), &srvDesc, m_heapCDH);
	
}

DX12Texture::~DX12Texture() {

}

void DX12Texture::initBuffers(ID3D12GraphicsCommandList4* cmdList) {
	UINT64 textureUploadBufferSize;
	// this function gets the size an upload buffer needs to be to upload a texture to the gpu.
	// each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
	// eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
	m_context->getDevice()->GetCopyableFootprints(&m_textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	// Create the upload heap
	// TODO: release the upload heap when it has been copied to the default buffer
	// This could be done in a buffer manager owned by dx12api
	m_textureUploadBuffer.Attach(DX12Utils::CreateBuffer(m_context->getDevice(), textureUploadBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties));
	m_textureUploadBuffer->SetName(L"Texture upload buffer");

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = m_textureData.getTextureData();
	textureData.RowPitch = m_textureData.getWidth() * m_textureData.getBytesPerPixel();
	textureData.SlicePitch = textureData.RowPitch * m_textureData.getHeight();
	// Copy the upload buffer contents to the default heap using a helper method from d3dx12.h
	DX12Utils::UpdateSubresources(cmdList, m_textureDefaultBuffer.Get(), m_textureUploadBuffer.Get(), 0, 0, 1, &textureData);
	DX12Utils::SetResourceTransitionBarrier(cmdList, m_textureDefaultBuffer.Get(), m_state, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_isInitialized = true;
}

bool DX12Texture::hasBeenInitialized() const {
	return m_isInitialized;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Texture::getCDH() const {
	return m_heapCDH;
}

ID3D12Resource1* DX12Texture::getBuffer() const {
	return m_textureDefaultBuffer.Get();
}

void DX12Texture::transitionStateTo(ID3D12GraphicsCommandList4* cmdList, D3D12_RESOURCE_STATES newState) {
	if (m_state == newState) return;

	DX12Utils::SetResourceTransitionBarrier(cmdList, m_textureDefaultBuffer.Get(), m_state, newState);
	m_state = newState;

}
