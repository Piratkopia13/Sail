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
	, m_cpuRtvDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1)
{
	isRenderableTex = true;
	m_rtvHeapCDH = m_cpuRtvDescHeap.getCPUDescriptorHandleForIndex(0);
	createTextures();
	m_context = Application::getInstance()->getAPI<DX12API>();
}

DX12RenderableTexture::~DX12RenderableTexture() {

}

void DX12RenderableTexture::begin(void* cmdList) {
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);

	transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	dxCmdList->OMSetRenderTargets(1, &m_rtvHeapCDH, true, &m_context->getDepthStencilViewCDH());

	dxCmdList->RSSetViewports(1, m_context->getViewport());
	dxCmdList->RSSetScissorRects(1, m_context->getScissorRect());
}

void DX12RenderableTexture::end(void* cmdList) {
	// Does nothing
}

void DX12RenderableTexture::clear(const glm::vec4& color, void* cmdList) {
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	// Clear
	float clearColor[4];
	clearColor[0] = color.r;
	clearColor[1] = color.g;
	clearColor[2] = color.b;
	clearColor[3] = color.a;

	dxCmdList->ClearRenderTargetView(m_rtvHeapCDH, clearColor, 0, nullptr);
	dxCmdList->ClearDepthStencilView(m_context->getDepthStencilViewCDH(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DX12RenderableTexture::resize(int width, int height) {
	if (width == m_width && height == m_height) return;

	m_width = width;
	m_height = height;
	createTextures();
}

ID3D12Resource1* DX12RenderableTexture::getResource() const {
	return textureDefaultBuffer.Get();
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
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	// A texture rarely updates its data, if at all, so it is stored in a default heap
	ThrowIfFailed(context->getDevice()->CreateCommittedResource(&DX12Utils::sDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &textureDesc, state, &optimizedClearValue, IID_PPV_ARGS(&textureDefaultBuffer)));
	textureDefaultBuffer->SetName(L"Renderable texture default buffer");

	// Create a shader resource view (descriptor that points to the texture and describes it)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	context->getDevice()->CreateShaderResourceView(textureDefaultBuffer.Get(), &srvDesc, srvHeapCDH);

	// Create a unordered access view
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	context->getDevice()->CreateUnorderedAccessView(textureDefaultBuffer.Get(), nullptr, &uavDesc, uavHeapCDH);

	// Create a render target view
	context->getDevice()->CreateRenderTargetView(textureDefaultBuffer.Get(), nullptr, m_rtvHeapCDH);
	// Creating an RTV changes the state for some reason, so make sure its updated
	//state = D3D12_RESOURCE_STATE_RENDER_TARGET;

}
