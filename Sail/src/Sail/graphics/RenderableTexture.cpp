#include "pch.h"
#include "RenderableTexture.h"
#include "Sail/Application.h"

RenderableTexture::RenderableTexture(UINT aaSamples, UINT width, UINT height, bool createDepthStencilView, bool createOnlyDSV, UINT bindFlags, UINT cpuAccessFlags)
	: m_width(width)
	, m_height(height)
	, m_hasDepthStencilView(createDepthStencilView)
	, m_dxColorTexture(nullptr)
	, m_dxDepthTexture(nullptr)
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

RenderableTexture::~RenderableTexture() {
	Memory::safeDelete(m_dxColorTexture);
	Memory::safeDelete(m_dxDepthTexture);
	Memory::safeRelease(m_renderTargetView);
	Memory::safeRelease(m_depthStencilView);

	// Release the SRV if the MSAA sample count is > 1, since we created our own when this is true
	if (m_aaSamples > 1) {
		Memory::safeRelease(m_nonMSAAColorTexture2D);
		Memory::safeRelease(m_nonMSAAColorSRV); 
		Memory::safeRelease(m_nonMSAADepthSRV);
	}
}

void RenderableTexture::createTextures() {

	// Set up the viewport
	m_viewport.Width = static_cast<FLOAT>(m_width);
	m_viewport.Height = static_cast<FLOAT>(m_height);
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	// Color
	if (!m_onlyDSV) {
		Memory::safeDelete(m_dxColorTexture);
		m_dxColorTexture = new DXTexture(m_width, m_height, m_aaSamples, m_bindFlags, m_cpuAccessFlags);

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		rtvDesc.ViewDimension = (m_aaSamples == 1) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
		rtvDesc.Texture2D.MipSlice = 0;

		// Delete the old
		Memory::safeRelease(m_renderTargetView);
		// Create the new
		Application::getInstance()->getAPI()->getDevice()->CreateRenderTargetView(m_dxColorTexture->getTexture2D(), &rtvDesc, &m_renderTargetView);
	}

	// Depth
	if (m_hasDepthStencilView) {
		Memory::safeDelete(m_dxDepthTexture);
		m_dxDepthTexture = new DXTexture(DXGI_FORMAT_R24G8_TYPELESS, m_width, m_height, m_aaSamples, m_cpuAccessFlags);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(dsvDesc));
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = (m_aaSamples == 1) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
		dsvDesc.Texture2D.MipSlice = 0;

		// Delete the old
		Memory::safeRelease(m_depthStencilView);
		// Create the new
		Application::getInstance()->getAPI()->getDevice()->CreateDepthStencilView(m_dxDepthTexture->getTexture2D(), &dsvDesc, &m_depthStencilView);
	}

	if (m_aaSamples > 1) {

		// Color

		// Create a non MSAA'd copy of the color texture for use in shaders
		D3D11_TEXTURE2D_DESC texDesc;
		m_dxColorTexture->getTexture2D()->GetDesc(&texDesc);
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | m_bindFlags;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.CPUAccessFlags = m_cpuAccessFlags;

		// Release the old Texture2D
		Memory::safeRelease(m_nonMSAAColorTexture2D);
		// Create the texture2D
		Application::getInstance()->getAPI()->getDevice()->CreateTexture2D(&texDesc, nullptr, &m_nonMSAAColorTexture2D);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		// Release the old SRV
		Memory::safeRelease(m_nonMSAAColorSRV);
		// Create the ShaderResourceView
		Application::getInstance()->getAPI()->getDevice()->CreateShaderResourceView(m_nonMSAAColorTexture2D, &srvDesc, &m_nonMSAAColorSRV);

	}
	else {
		if(!m_onlyDSV)
			m_nonMSAAColorSRV = *m_dxColorTexture->getResourceView();
		if (m_hasDepthStencilView)
			m_nonMSAADepthSRV = *m_dxDepthTexture->getResourceView();
	}

}

ID3D11ShaderResourceView** RenderableTexture::getColorSRV() {
	return &m_nonMSAAColorSRV;
}

ID3D11ShaderResourceView** RenderableTexture::getDepthSRV() {
	if (!m_hasDepthStencilView)
		Logger::Error("Tried to get DepthSRV from a RenderableTexture that initialized without one created.");

	if (m_aaSamples > 1)
		Logger::Error("Tried to get DepthSRV from a RenderableTexture that has MSAA enabled. This will not work in DirectX11.");
	return &m_nonMSAADepthSRV;
}
ID3D11RenderTargetView** RenderableTexture::getRenderTargetView() {
	if (m_onlyDSV)
		Logger::Error("Tried to get RenderTargetView when no render target view existed.");
	return &m_renderTargetView;
}
ID3D11DepthStencilView** RenderableTexture::getDepthStencilView() {
	if (m_aaSamples > 1)
		Logger::Error("Tried to get Depth Stencil View from a RenderableTexture that has MSAA enabled. This will not work in DirectX11.");
	return &m_depthStencilView;
}

D3D11_VIEWPORT* RenderableTexture::getViewPort() {
	return &m_viewport;
}

ID3D11Texture2D* RenderableTexture::getTexture2D() {
	return m_dxColorTexture->getTexture2D();
}

ID3D11Texture2D* RenderableTexture::getDepthTexture2D() {
	return m_dxDepthTexture->getTexture2D();
}

void RenderableTexture::begin() {

// 	if (!m_hasDepthStencilView)
// 		Logger::Error("Tried to call to RenderableTexture::begin on a RenderableTexture that was created without a depth stencil view. Use the data from this class manually or change the constructor parameter to create a DSV.");

	auto dxManager = Application::getInstance()->getAPI();
	// Set render target
	dxManager->getDeviceContext()->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	// Bind viewport
	dxManager->getDeviceContext()->RSSetViewports(1, &m_viewport);

}

void RenderableTexture::end() {
	// Create a non MSAA'd texture copy of the texture that was rendered to
	if (m_aaSamples > 1)
		Application::getInstance()->getAPI()->getDeviceContext()->ResolveSubresource(m_nonMSAAColorTexture2D, 0, m_dxColorTexture->getTexture2D(), 0, DXGI_FORMAT_R32G32B32A32_FLOAT);

	// Revert render target to the back buffer
	Application::getInstance()->getAPI()->renderToBackBuffer();
}

void RenderableTexture::clear(const DirectX::XMVECTORF32& color) {

	auto dxManager = Application::getInstance()->getAPI();
	dxManager->getDeviceContext()->ClearRenderTargetView(m_renderTargetView, color);
	if (m_hasDepthStencilView)
		dxManager->getDeviceContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

}

void RenderableTexture::resize(int width, int height) {
	m_width = width;
	m_height = height;
	createTextures();
}