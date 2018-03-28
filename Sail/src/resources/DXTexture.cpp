#include "DXTexture.h"
#include "../api/Application.h"

DXTexture::DXTexture(const std::string& filename) {

	// Load the texture file it if is not loaded already
	if (!Application::getInstance()->getResourceManager().hasTextureData(filename)) {
		Application::getInstance()->getResourceManager().LoadTextureData(filename);
	}
	auto& data = Application::getInstance()->getResourceManager().getTextureData(filename);

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Width = data.getWidth();
	texDesc.Height = data.getHeight();
	texDesc.MipLevels = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create an empty Texture2D
	ThrowIfFailed(Application::getInstance()->getAPI()->getDevice()->CreateTexture2D(&texDesc, nullptr, &m_texture));
	// Fill the texture with data
	UINT rowPitch = sizeof(unsigned char) * data.getWidth() * 4;
	Application::getInstance()->getAPI()->getDeviceContext()->UpdateSubresource(m_texture, 0, nullptr, data.getTextureData(), rowPitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	// Create the ShaderResourceView
	ThrowIfFailed(Application::getInstance()->getAPI()->getDevice()->CreateShaderResourceView(m_texture, &srvDesc, &m_resourceView));

	// Generate mipmaps for this texture.
	Application::getInstance()->getAPI()->getDeviceContext()->GenerateMips(m_resourceView);

}

DXTexture::DXTexture(UINT width, UINT height, UINT aaSamples, UINT bindFlags, UINT cpuAccessFlags) {

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | bindFlags;
	texDesc.CPUAccessFlags = cpuAccessFlags;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = aaSamples;
	texDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the texture2D
	Application::getInstance()->getAPI()->getDevice()->CreateTexture2D(&texDesc, nullptr, &m_texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = (aaSamples == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	// Create the ShaderResourceView
	Application::getInstance()->getAPI()->getDevice()->CreateShaderResourceView(m_texture, &srvDesc, &m_resourceView);

}

DXTexture::DXTexture(DXGI_FORMAT format, UINT width, UINT height, UINT samples, UINT cpuAccessFlags) {

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = cpuAccessFlags;
	texDesc.Format = format;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = samples;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = (cpuAccessFlags == D3D11_CPU_ACCESS_READ) ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;

	// Create the texture2D
	Application::getInstance()->getAPI()->getDevice()->CreateTexture2D(&texDesc, nullptr, &m_texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = (samples == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the ShaderResourceView
	Application::getInstance()->getAPI()->getDevice()->CreateShaderResourceView(m_texture, &srvDesc, &m_resourceView);
}

DXTexture::~DXTexture() {
	Memory::safeRelease(m_texture);
	Memory::safeRelease(m_resourceView);
}

ID3D11ShaderResourceView** DXTexture::getResourceView() {
	return &m_resourceView;
}

ID3D11Texture2D* DXTexture::getTexture2D() {
	return m_texture;
}
