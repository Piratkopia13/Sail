#include "pch.h"
#include "../DX11API.h"
#include "DX11RenderableTexture.h"
#include "Sail/Application.h"
#include "DX11Texture.h"

RenderableTexture* RenderableTexture::Create(uint32_t width, uint32_t height, UsageFlags usage, const std::string& name,
	ResourceFormat::TextureFormat format, bool singleBuffer, uint32_t arraySize, const glm::vec4& clearColor)
{
	return SAIL_NEW DX11RenderableTexture(width, height, usage, format, singleBuffer, arraySize, clearColor);
}

DX11RenderableTexture::DX11RenderableTexture(uint32_t width, uint32_t height, UsageFlags usage, ResourceFormat::TextureFormat format, bool singleBuffer, bool arraySize, const glm::vec4& clearColor)
	: m_width(width)
	, m_height(height)
	, m_texture(nullptr)
	, m_rtv(nullptr)
	, m_aaSamples(1) // TODO: support multi sampling
	//, m_nonMSAAColorTexture2D(nullptr)
	//, m_nonMSAAColorSRV(nullptr)
	//, m_uav(nullptr)
	//, m_nonMSAADepthSRV(nullptr)
	, m_dsv(nullptr)
{
	m_format = DX11Texture::ConvertToDXGIFormat(format);

	m_isDepthStencil = (format == ResourceFormat::DEPTH);

	createTextures();
}

DX11RenderableTexture::~DX11RenderableTexture() {
	Memory::SafeDelete(m_texture);
	Memory::SafeRelease(m_rtv);
	Memory::SafeRelease(m_dsv);
	//Memory::SafeRelease(m_uav);

	//// Release the SRV if the MSAA sample count is > 1, since we created our own when this is true
	//if (m_aaSamples > 1) {
	//	Memory::SafeRelease(m_nonMSAAColorTexture2D);
	//	Memory::SafeRelease(m_nonMSAAColorSRV); 
	//	Memory::SafeRelease(m_nonMSAADepthSRV);
	//}
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
	if (!m_isDepthStencil) {
		Memory::SafeDelete(m_texture);
		UINT bindFlags = D3D11_BIND_RENDER_TARGET;
		if (m_usageFlags & USAGE_UNORDERED_ACCESS) {
			bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}
		UINT cpuFlags = 0;
		m_texture = SAIL_NEW DX11Texture(m_width, m_height, m_format, m_aaSamples, bindFlags, cpuFlags);

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = m_format;
		rtvDesc.ViewDimension = (m_aaSamples == 1) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
		rtvDesc.Texture2D.MipSlice = 0;

		// Delete the old
		Memory::SafeRelease(m_rtv);
		// Create the new
		api->getDevice()->CreateRenderTargetView(m_texture->getTexture2D(), &rtvDesc, &m_rtv);

		// Delete the old uav
		Memory::SafeRelease(m_uav);
		// Create the new uav
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		api->getDevice()->CreateUnorderedAccessView(m_texture->getTexture2D(), &uavDesc, &m_uav);

		//if (m_aaSamples > 1) {

		//	// Create a non MSAA'd copy of the color texture for use in shaders
		//	D3D11_TEXTURE2D_DESC texDesc;
		//	m_texture->getTexture2D()->GetDesc(&texDesc);
		//	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | m_bindFlags;
		//	texDesc.SampleDesc.Count = 1;
		//	texDesc.SampleDesc.Quality = 0;
		//	texDesc.CPUAccessFlags = m_cpuAccessFlags;

		//	// Release the old Texture2D
		//	Memory::SafeRelease(m_nonMSAAColorTexture2D);
		//	// Create the texture2D
		//	api->getDevice()->CreateTexture2D(&texDesc, nullptr, &m_nonMSAAColorTexture2D);

		//	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		//	ZeroMemory(&srvDesc, sizeof(srvDesc));
		//	srvDesc.Format = m_format;
		//	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		//	srvDesc.Texture2D.MostDetailedMip = 0;
		//	srvDesc.Texture2D.MipLevels = 1;

		//	// Release the old SRV
		//	Memory::SafeRelease(m_nonMSAAColorSRV);
		//	// Create the ShaderResourceView
		//	api->getDevice()->CreateShaderResourceView(m_nonMSAAColorTexture2D, &srvDesc, &m_nonMSAAColorSRV);
		//}
	} else {
		// Depth
		Memory::SafeDelete(m_texture);
		UINT cpuFlags = 0;
		m_texture = SAIL_NEW DX11Texture(DXGI_FORMAT_R24G8_TYPELESS, m_width, m_height, m_aaSamples, cpuFlags);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(dsvDesc));
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = (m_aaSamples == 1) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
		dsvDesc.Texture2D.MipSlice = 0;

		// Delete the old
		Memory::SafeRelease(m_dsv);
		// Create the new
		api->getDevice()->CreateDepthStencilView(m_texture->getTexture2D(), &dsvDesc, &m_dsv);
	}

	//if (m_aaSamples <= 1) {
	//	if(!m_onlyDSV)
	//		m_nonMSAAColorSRV = m_texture->getSRV();
	//	if (m_hasDepthStencilView)
	//		m_nonMSAADepthSRV = m_dxDepthTexture->getSRV();
	//}

}

ID3D11ShaderResourceView* DX11RenderableTexture::getSRV() {
	return m_srv;
}

ID3D11UnorderedAccessView* DX11RenderableTexture::getUAV() {
	return m_uav;
}

ID3D11RenderTargetView* DX11RenderableTexture::getRTV() {
	if (m_isDepthStencil)
		Logger::Error("Tried to get RenderTargetView when no render target view existed.");
	return m_rtv;
}
ID3D11DepthStencilView* DX11RenderableTexture::getDSV() {
	if (m_aaSamples > 1)
		Logger::Error("Tried to get Depth Stencil View from a DX11RenderableTexture that has MSAA enabled. This will not work in DirectX11.");
	if (!m_isDepthStencil)
		Logger::Error("Tried to get DSV on a color target.");
	return m_dsv;
}

D3D11_VIEWPORT* DX11RenderableTexture::getViewPort() {
	return &m_viewport;
}

ID3D11Texture2D* DX11RenderableTexture::getTexture2D() {
	return m_texture->getTexture2D();
}

void DX11RenderableTexture::changeFormat(ResourceFormat::TextureFormat newFormat) {
	assert(false && "changeFormat() is not yet implemented for DX11");
}

void DX11RenderableTexture::begin(void* cmdList) {

// 	if (!m_hasDepthStencilView)
// 		Logger::Error("Tried to call to DX11RenderableTexture::begin on a DX11RenderableTexture that was created without a depth stencil view. Use the data from this class manually or change the constructor parameter to create a DSV.");

	auto api = Application::getInstance()->getAPI<DX11API>();
	// Set render target
	api->getDeviceContext()->OMSetRenderTargets(1, &m_rtv, m_dsv); // Both of these do not exist at the same time - fix this
	// Bind viewport
	api->getDeviceContext()->RSSetViewports(1, &m_viewport);

}

void DX11RenderableTexture::end(void* cmdList) {
	auto api = Application::getInstance()->getAPI<DX11API>();
	// Create a non MSAA'd texture copy of the texture that was rendered to
	/*if (m_aaSamples > 1)
		api->getDeviceContext()->ResolveSubresource(m_nonMSAAColorTexture2D, 0, m_texture->getTexture2D(), 0, m_format);*/

	// Revert render target to the back buffer
	api->renderToBackBuffer();
}

void DX11RenderableTexture::clear(const glm::vec4& color, void* cmdList) {
	auto api = Application::getInstance()->getAPI<DX11API>();
	if (m_isDepthStencil)
		api->getDeviceContext()->ClearDepthStencilView(m_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	else 
		api->getDeviceContext()->ClearRenderTargetView(m_rtv, glm::value_ptr(color));
}

void DX11RenderableTexture::resize(int width, int height) {
	if (width == m_width && height == m_height) {
		return;
	}
	m_width = width;
	m_height = height;
	createTextures();
}