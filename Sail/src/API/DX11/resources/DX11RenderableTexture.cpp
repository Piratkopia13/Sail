#include "pch.h"
#include "../DX11API.h"
#include "DX11RenderableTexture.h"
#include "Sail/Application.h"

RenderableTexture* RenderableTexture::Create(unsigned int width, unsigned int height, const std::string& name, ResourceFormat::TextureFormat format, bool createDepthStencilView, bool createOnlyDSV, unsigned int arraySize, bool singleBuffer) {
	return SAIL_NEW DX11RenderableTexture(1, width, height, createDepthStencilView, createOnlyDSV, 0U, 0U);
}

DX11RenderableTexture::DX11RenderableTexture(UINT aaSamples, UINT width, UINT height, bool createDepthStencilView, bool createOnlyDSV, UINT bindFlags, UINT cpuAccessFlags)
	: m_width(width)
	, m_height(height)
	, m_hasDepthStencilView(createDepthStencilView)
	, m_dxColorTexture(nullptr)
	, m_dxDepthTexture(nullptr)
	, m_renderTargetView(nullptr)
	, m_aaSamples(aaSamples)
	, m_nonMSAAColorTexture2D(nullptr)
	, m_nonMSAAColorSRV(nullptr)
	, m_nonMSAADepthSRV(nullptr)
	, m_depthStencilView(nullptr)
	, m_onlyDSV(createOnlyDSV)
	, m_bindFlags(bindFlags)
	, m_cpuAccessFlags(cpuAccessFlags)
{
	createTextures();
}

DX11RenderableTexture::~DX11RenderableTexture() {
	Memory::SafeDelete(m_dxColorTexture);
	Memory::SafeDelete(m_dxDepthTexture);
	Memory::SafeRelease(m_renderTargetView);
	Memory::SafeRelease(m_depthStencilView);

	// Release the SRV if the MSAA sample count is > 1, since we created our own when this is true
	if (m_aaSamples > 1) {
		Memory::SafeRelease(m_nonMSAAColorTexture2D);
		Memory::SafeRelease(m_nonMSAAColorSRV); 
		Memory::SafeRelease(m_nonMSAADepthSRV);
	}
}

void DX11RenderableTexture::createTextures() {

	auto api = Application::getInstance()->getAPI<DX11API>();

	// Set up the viewport
	m_viewport.Width = static_cast<FLOAT>(m_width);
	m_viewport.Height = static_cast<FLOAT>(m_height);
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	// Color
	if (!m_onlyDSV) {
		Memory::SafeDelete(m_dxColorTexture);
		m_dxColorTexture = SAIL_NEW DX11Texture(m_width, m_height, m_aaSamples, m_bindFlags, m_cpuAccessFlags);

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		rtvDesc.ViewDimension = (m_aaSamples == 1) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
		rtvDesc.Texture2D.MipSlice = 0;

		// Delete the old
		Memory::SafeRelease(m_renderTargetView);
		// Create the new
		api->getDevice()->CreateRenderTargetView(m_dxColorTexture->getTexture2D(), &rtvDesc, &m_renderTargetView);

		if (m_aaSamples > 1) {

			// Create a non MSAA'd copy of the color texture for use in shaders
			D3D11_TEXTURE2D_DESC texDesc;
			m_dxColorTexture->getTexture2D()->GetDesc(&texDesc);
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | m_bindFlags;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.CPUAccessFlags = m_cpuAccessFlags;

			// Release the old Texture2D
			Memory::SafeRelease(m_nonMSAAColorTexture2D);
			// Create the texture2D
			api->getDevice()->CreateTexture2D(&texDesc, nullptr, &m_nonMSAAColorTexture2D);

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;

			// Release the old SRV
			Memory::SafeRelease(m_nonMSAAColorSRV);
			// Create the ShaderResourceView
			api->getDevice()->CreateShaderResourceView(m_nonMSAAColorTexture2D, &srvDesc, &m_nonMSAAColorSRV);

		}

	}

	// Depth
	if (m_hasDepthStencilView) {
		Memory::SafeDelete(m_dxDepthTexture);
		m_dxDepthTexture = SAIL_NEW DX11Texture(DXGI_FORMAT_R24G8_TYPELESS, m_width, m_height, m_aaSamples, m_cpuAccessFlags);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(dsvDesc));
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = (m_aaSamples == 1) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
		dsvDesc.Texture2D.MipSlice = 0;

		// Delete the old
		Memory::SafeRelease(m_depthStencilView);
		// Create the new
		api->getDevice()->CreateDepthStencilView(m_dxDepthTexture->getTexture2D(), &dsvDesc, &m_depthStencilView);
	}

	if (m_aaSamples <= 1) {
		if(!m_onlyDSV)
			m_nonMSAAColorSRV = m_dxColorTexture->getSRV();
		if (m_hasDepthStencilView)
			m_nonMSAADepthSRV = m_dxDepthTexture->getSRV();
	}

}

ID3D11ShaderResourceView** DX11RenderableTexture::getColorSRV() {
	return &m_nonMSAAColorSRV;
}

ID3D11ShaderResourceView** DX11RenderableTexture::getDepthSRV() {
	if (!m_hasDepthStencilView)
		Logger::Error("Tried to get DepthSRV from a DX11RenderableTexture that initialized without one created.");

	if (m_aaSamples > 1)
		Logger::Error("Tried to get DepthSRV from a DX11RenderableTexture that has MSAA enabled. This will not work in DirectX11.");
	return &m_nonMSAADepthSRV;
}
ID3D11RenderTargetView** DX11RenderableTexture::getRenderTargetView() {
	if (m_onlyDSV)
		Logger::Error("Tried to get RenderTargetView when no render target view existed.");
	return &m_renderTargetView;
}
ID3D11DepthStencilView** DX11RenderableTexture::getDepthStencilView() {
	if (m_aaSamples > 1)
		Logger::Error("Tried to get Depth Stencil View from a DX11RenderableTexture that has MSAA enabled. This will not work in DirectX11.");
	return &m_depthStencilView;
}

D3D11_VIEWPORT* DX11RenderableTexture::getViewPort() {
	return &m_viewport;
}

ID3D11Texture2D* DX11RenderableTexture::getTexture2D() {
	return m_dxColorTexture->getTexture2D();
}

ID3D11Texture2D* DX11RenderableTexture::getDepthTexture2D() {
	return m_dxDepthTexture->getTexture2D();
}

void DX11RenderableTexture::changeFormat(ResourceFormat::TextureFormat newFormat) {
	assert(false && "changeFormat() is not yet implemented for DX11");
}

void DX11RenderableTexture::begin(void* cmdList) {

// 	if (!m_hasDepthStencilView)
// 		Logger::Error("Tried to call to DX11RenderableTexture::begin on a DX11RenderableTexture that was created without a depth stencil view. Use the data from this class manually or change the constructor parameter to create a DSV.");

	auto api = Application::getInstance()->getAPI<DX11API>();
	// Set render target
	api->getDeviceContext()->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	// Bind viewport
	api->getDeviceContext()->RSSetViewports(1, &m_viewport);

}

void DX11RenderableTexture::end(void* cmdList) {
	auto api = Application::getInstance()->getAPI<DX11API>();
	// Create a non MSAA'd texture copy of the texture that was rendered to
	if (m_aaSamples > 1)
		api->getDeviceContext()->ResolveSubresource(m_nonMSAAColorTexture2D, 0, m_dxColorTexture->getTexture2D(), 0, DXGI_FORMAT_R32G32B32A32_FLOAT);

	// Revert render target to the back buffer
	api->renderToBackBuffer();
}

void DX11RenderableTexture::clear(const glm::vec4& color, void* cmdList) {

	auto api = Application::getInstance()->getAPI<DX11API>();
	api->getDeviceContext()->ClearRenderTargetView(m_renderTargetView, glm::value_ptr(color));
	if (m_hasDepthStencilView)
		api->getDeviceContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

}

void DX11RenderableTexture::resize(int width, int height) {
	m_width = width;
	m_height = height;
	createTextures();
}