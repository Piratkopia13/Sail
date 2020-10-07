#include "pch.h"
#include "DX12RenderableTexture.h"
#include "Sail/Application.h"
#include "Sail/api/Window.h"
#include "../DX12Utils.h"

// TODO: fix compilation error
// Depth texture is now handled separately, see VK implementation for details
RenderableTexture* RenderableTexture::Create(uint32_t width, uint32_t height, UsageFlags usage, const std::string& name, 
	ResourceFormat::TextureFormat format, bool singleBuffer, uint32_t arraySize, const glm::vec4& clearColor) 
{
	return SAIL_NEW DX12RenderableTexture(width, height, usage, name, format, clearColor, singleBuffer, arraySize);
}

DX12RenderableTexture::DX12RenderableTexture(uint32_t width, uint32_t height, UsageFlags usage, const std::string& name, 
	ResourceFormat::TextureFormat format, const glm::vec4& clearColor, bool singleBuffer, unsigned int arraySize)
	: m_width(width)
	, m_height(height)
	, m_cpuRtvDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, Application::getInstance()->getAPI<DX12API>()->getNumSwapBuffers())
	, m_cpuDsvDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, Application::getInstance()->getAPI<DX12API>()->getNumSwapBuffers())
	, m_arraySize(arraySize)
	, m_clearColor(clearColor)
	, m_usageFlags(usage)
{
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	isRenderableTex = true;
	context = Application::getInstance()->getAPI<DX12API>();

	m_name = name;
	m_isDepthStencil = (format == ResourceFormat::DEPTH);
	m_format = DX12Texture::ConvertToDXGIFormat(format);

	m_numSwapBuffers = context->getNumSwapBuffers();
	if (singleBuffer) {
		// RenderableTextures are not multi-buffered, consecutive frames will use the same render textures.
		useOneResource = true;
		m_numSwapBuffers = 1;
	}

	m_rtvHeapCDHs.resize(m_numSwapBuffers);
	m_dsvHeapCDHs.resize(m_numSwapBuffers);
	for (unsigned int i = 0; i < m_numSwapBuffers; i++) {
		m_rtvHeapCDHs[i] = m_cpuRtvDescHeap.getCPUDescriptorHandleForIndex(i);
		m_dsvHeapCDHs[i] = m_cpuDsvDescHeap.getCPUDescriptorHandleForIndex(i);
	}
	if (m_isDepthStencil) {
		createDepthTextures();
	} else {
		createTextures();
	}
}

DX12RenderableTexture::~DX12RenderableTexture() { }

void DX12RenderableTexture::begin(void* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);

	//transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	const D3D12_CPU_DESCRIPTOR_HANDLE* dsvCdh = (m_isDepthStencil) ? &m_dsvHeapCDHs[getSwapIndex()] : &context->getDsvCDH();
	dxCmdList->OMSetRenderTargets(1, &m_rtvHeapCDHs[getSwapIndex()], true, dsvCdh);

	dxCmdList->RSSetViewports(1, context->getViewport());
	dxCmdList->RSSetScissorRects(1, context->getScissorRect());
}

void DX12RenderableTexture::end(void* cmdList) {
	// Does nothing
}

void DX12RenderableTexture::clear(const glm::vec4& color, void* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	// Clear
	FLOAT clearColor[4];
	clearColor[0] = color.r;
	clearColor[1] = color.g;
	clearColor[2] = color.b;
	clearColor[3] = color.a;

	if (m_isDepthStencil) {
		// Resource state must be set to render target before clearing
		transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		dxCmdList->ClearDepthStencilView(m_dsvHeapCDHs[getSwapIndex()], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	} else {
		// Resource state must be set to render target before clearing
		transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

		dxCmdList->ClearRenderTargetView(m_rtvHeapCDHs[getSwapIndex()], clearColor, 0, nullptr);
	}
}

void DX12RenderableTexture::changeFormat(ResourceFormat::TextureFormat newFormat) {
	auto newDxgiFormat = DX12Texture::ConvertToDXGIFormat(newFormat);
	if (m_format == newDxgiFormat) {
		return;
	}
	m_format = newDxgiFormat;
	m_isDepthStencil = (newFormat == ResourceFormat::DEPTH);

	if (m_isDepthStencil) {
		createDepthTextures();
	} else {
		createTextures();
	}
}

void DX12RenderableTexture::resize(int width, int height) {
	if (width == m_width && height == m_height) {
		return;
	}
	m_width = width;
	m_height = height;

	if (m_isDepthStencil) {
		createDepthTextures();
	} else {
		createTextures();
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderableTexture::getRtvCDH(int frameIndex) const {
	unsigned int i = (frameIndex == -1 || useOneResource) ? getSwapIndex() : frameIndex;
	return m_rtvHeapCDHs[i];
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderableTexture::getDsvCDH(int frameIndex) const {
	unsigned int i = (frameIndex == -1 || useOneResource) ? getSwapIndex() : frameIndex;
	return m_dsvHeapCDHs[i];
}

void DX12RenderableTexture::createTextures() {	
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	context->waitForGPU();
	for (unsigned int i = 0; i < m_numSwapBuffers; i++) {
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.Format = m_format;
		textureDesc.Width = m_width;
		textureDesc.Height = m_height;
		textureDesc.DepthOrArraySize = m_arraySize;
		textureDesc.MipLevels = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		if (m_usageFlags & USAGE_UNORDERED_ACCESS) {
			textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}
		textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		D3D12_CLEAR_VALUE clearValue = { textureDesc.Format, {m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.a} };

		state[i] = D3D12_RESOURCE_STATE_COMMON;
		// A texture rarely updates its data, if at all, so it is stored in a default heap
		ThrowIfFailed(context->getDevice()->CreateCommittedResource(&DX12Utils::sDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &textureDesc, state[i], &clearValue, IID_PPV_ARGS(&textureDefaultBuffers[i])));

		std::wstring stemp = std::wstring(m_name.begin(), m_name.end());
		textureDefaultBuffers[i]->SetName(stemp.c_str());

		// Create a shader resource view (descriptor that points to the texture and describes it)
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		if (m_arraySize > 1) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.ArraySize = m_arraySize;
			srvDesc.Texture2DArray.MipLevels = 1;
		} else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
		}
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		context->getDevice()->CreateShaderResourceView(textureDefaultBuffers[i].Get(), &srvDesc, srvHeapCDHs[i]);

		// Create a unordered access view
		if (m_usageFlags & USAGE_UNORDERED_ACCESS) {
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			if (m_arraySize > 1) {
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uavDesc.Texture2DArray.ArraySize = m_arraySize;
			} else {
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			}
			uavDesc.Format = textureDesc.Format;
			context->getDevice()->CreateUnorderedAccessView(textureDefaultBuffers[i].Get(), nullptr, &uavDesc, uavHeapCDHs[i]);
		}

		// Create a render target view
		context->getDevice()->CreateRenderTargetView(textureDefaultBuffers[i].Get(), nullptr, m_rtvHeapCDHs[i]);
	}
}

void DX12RenderableTexture::createDepthTextures() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	for (unsigned int i = 0; i < m_numSwapBuffers; i++) {

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		D3D12_RESOURCE_DESC bufferDesc{};
		bufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		bufferDesc.Width = Application::getInstance()->getWindow()->getWindowWidth();
		bufferDesc.Height = Application::getInstance()->getWindow()->getWindowHeight();
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 0;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		state[i] = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		context->getDevice()->CreateCommittedResource(
			&DX12Utils::sDefaultHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			state[i],
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&textureDefaultBuffers[i])
		);
		textureDefaultBuffers[i]->SetName(L"Depth/Stencil Resource Buffer");

		// Create DSV
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = bufferDesc.Format;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
		context->getDevice()->CreateDepthStencilView(textureDefaultBuffers[i].Get(), &depthStencilDesc, m_dsvHeapCDHs[i]);

		// Create a shader resource view for the depth stencil
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		context->getDevice()->CreateShaderResourceView(textureDefaultBuffers[i].Get(), &srvDesc, srvHeapCDHs[i]);
	}
}

unsigned int DX12RenderableTexture::getSwapIndex() const {
	return (useOneResource) ? 0 : context->getSwapIndex();
}
