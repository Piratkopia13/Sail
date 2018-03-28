#pragma once

#include "TextureData.h"

class DXTexture {

public:
	// Create a texture with data loaded from file
	DXTexture(const std::string& filename);
	// Createa an empty texture wit the D3D11_BIND_RENDER_TARGET flag set
	DXTexture(UINT width, UINT height, UINT aaSamples, UINT bindFlags = 0, UINT cpuAccessFlags = 0);
	// Create an empty texture with chosen parameters
	DXTexture(DXGI_FORMAT format, UINT width, UINT height, UINT samples = 1, UINT cpuAccessFlags = 0);
	~DXTexture();

	ID3D11ShaderResourceView** getResourceView();
	ID3D11Texture2D* getTexture2D();

private:
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_resourceView;

};