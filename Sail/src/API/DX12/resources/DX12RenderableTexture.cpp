#include "pch.h"
#include "DX12RenderableTexture.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"

RenderableTexture* RenderableTexture::Create(unsigned int width, unsigned int height) {
	return SAIL_NEW DX12RenderableTexture(1, width, height);
}

DX12RenderableTexture::DX12RenderableTexture(UINT aaSamples, unsigned int width, unsigned int height, bool createDepthStencilView, bool createOnlyDSV, UINT bindFlags, UINT cpuAccessFlags)
	: m_width(width)
	, m_height(height)
{
	createTextures();
}

DX12RenderableTexture::~DX12RenderableTexture() {

}

void DX12RenderableTexture::begin() {
	assert(false); // Not implemented
}

void DX12RenderableTexture::end() {
	assert(false); // Not implemented

}

void DX12RenderableTexture::clear(const glm::vec4& color) {
	assert(false); // Not implemented

}

void DX12RenderableTexture::resize(int width, int height) {
	m_width = width;
	m_height = height;
	createTextures();
}

void DX12RenderableTexture::bind(ID3D12GraphicsCommandList* cmdList) const {
	// TODO: do similiar thing to Texture binding - copy srv to heap n' stuff
	//cmdList->
}

ID3D12Resource1* DX12RenderableTexture::getResource() const {
	return m_textureDefaultBuffer.Get();
}

void DX12RenderableTexture::createTextures() {
	auto context = Application::getInstance()->getAPI<DX12API>();

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width = m_width;
	textureDesc.Height = m_height;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	// A texture rarely updates its data, if at all, so it is stored in a default heap
	ThrowIfFailed(context->getDevice()->CreateCommittedResource(&DX12Utils::sDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_textureDefaultBuffer)));
	m_textureDefaultBuffer->SetName(L"Renderable texture default buffer");
}
