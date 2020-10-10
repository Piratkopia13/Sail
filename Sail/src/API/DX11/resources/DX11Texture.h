#pragma once

#include <d3d11.h>
#include "Sail/resources/TextureData.h"
#include "Sail/api/Texture.h"

class DX11Texture : public Texture {

public:
	// Create a texture with data loaded from file
	DX11Texture(const std::string& filename, bool useAbsolutePath = false);
	// Create an empty texture wit the D3D11_BIND_RENDER_TARGET flag set
	DX11Texture(UINT width, UINT height, DXGI_FORMAT format, UINT aaSamples, UINT bindFlags = 0, UINT cpuAccessFlags = 0);
	// Create an empty texture with chosen parameters
	DX11Texture(DXGI_FORMAT format, UINT width, UINT height, UINT samples = 1, UINT cpuAccessFlags = 0);
	~DX11Texture();

	ID3D11ShaderResourceView* const getSRV() const;
	ID3D11Texture2D* getTexture2D();

	static DXGI_FORMAT ConvertToDXGIFormat(ResourceFormat::TextureFormat format);

private:
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_resourceView;

};