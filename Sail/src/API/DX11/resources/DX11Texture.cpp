#include "pch.h"
#include "DX11Texture.h"
#include "Sail/Application.h"
#include "../DX11API.h"
#include "DDSTextureLoader/DDSTextureLoader11.cpp"

Texture* Texture::Create(const std::string& filename, bool useAbsolutePath) {
	return SAIL_NEW DX11Texture(filename, useAbsolutePath);
}

DX11Texture::DX11Texture(const std::string& filename, bool useAbsolutePath)
	: Texture(filename)
{
	auto api = Application::getInstance()->getAPI<DX11API>();

	if (filename.substr(filename.length() - 3) == "dds") {
		std::string path = (useAbsolutePath) ? filename : TextureData::DEFAULT_TEXTURE_LOCATION + filename;
		std::wstring wideFilename = std::wstring(path.begin(), path.end());

		DirectX::CreateDDSTextureFromFile(api->getDevice(), wideFilename.c_str(), (ID3D11Resource**)&m_texture, &m_resourceView);

	} else {
		TextureData& data = getTextureData(filename, useAbsolutePath);

		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.ArraySize = 1;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.Format = ConvertToDXGIFormat(data.getFormat());;
		texDesc.Width = data.getWidth();
		texDesc.Height = data.getHeight();
		texDesc.MipLevels = 6;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		// Create an empty Texture2D
		ThrowIfFailed(api->getDevice()->CreateTexture2D(&texDesc, nullptr, &m_texture));
		// Fill the texture with data
		UINT rowPitch = sizeof(unsigned char) * data.getWidth() * 4;
		api->getDeviceContext()->UpdateSubresource(m_texture, 0, nullptr, data.getData(), rowPitch, 0);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		// Create the ShaderResourceView
		ThrowIfFailed(api->getDevice()->CreateShaderResourceView(m_texture, &srvDesc, &m_resourceView));

		// Generate mipmaps for this texture.
		api->getDeviceContext()->GenerateMips(m_resourceView);
		
		// Delete texture data from the CPU
		Application::getInstance()->getResourceManager().releaseTextureData(filename);
	}


}

DX11Texture::DX11Texture(UINT width, UINT height, DXGI_FORMAT format, UINT aaSamples, UINT bindFlags, UINT cpuAccessFlags)
	: Texture("Custom texture")
{
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | bindFlags;
	texDesc.CPUAccessFlags = cpuAccessFlags;
	texDesc.Format = format;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = aaSamples;
	texDesc.Usage = D3D11_USAGE_DEFAULT;

	auto api = Application::getInstance()->getAPI<DX11API>();
	// Create the texture2D
	api->getDevice()->CreateTexture2D(&texDesc, nullptr, &m_texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = (aaSamples == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	// Create the ShaderResourceView
	api->getDevice()->CreateShaderResourceView(m_texture, &srvDesc, &m_resourceView);

}

DX11Texture::DX11Texture(DXGI_FORMAT format, UINT width, UINT height, UINT samples, UINT cpuAccessFlags)
	: Texture("Custom texture")
{
	auto api = Application::getInstance()->getAPI<DX11API>();

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
	api->getDevice()->CreateTexture2D(&texDesc, nullptr, &m_texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = (samples == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the ShaderResourceView
	api->getDevice()->CreateShaderResourceView(m_texture, &srvDesc, &m_resourceView);
}

DX11Texture::~DX11Texture() {
	Memory::SafeRelease(m_texture);
	Memory::SafeRelease(m_resourceView);
}

ID3D11ShaderResourceView* const DX11Texture::getSRV() const {
	return m_resourceView;
}

ID3D11Texture2D* DX11Texture::getTexture2D() {
	return m_texture;
}

DXGI_FORMAT DX11Texture::ConvertToDXGIFormat(ResourceFormat::TextureFormat format) {
	DXGI_FORMAT dxgiFormat;
	switch (format) {
	case ResourceFormat::R8:
		dxgiFormat = DXGI_FORMAT_R8_UNORM;
		break;
	case ResourceFormat::R8G8:
		dxgiFormat = DXGI_FORMAT_R8G8_UNORM;
		break;
	case ResourceFormat::R8G8B8A8:
		dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case ResourceFormat::R16_FLOAT:
		dxgiFormat = DXGI_FORMAT_R16_FLOAT;
		break;
	case ResourceFormat::R16G16_FLOAT:
		dxgiFormat = DXGI_FORMAT_R16G16_FLOAT;
		break;
	case ResourceFormat::R16G16B16A16_FLOAT:
		dxgiFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case ResourceFormat::R32G32B32A32_FLOAT:
		dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	}
	return dxgiFormat;
}
