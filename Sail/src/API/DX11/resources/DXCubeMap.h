#pragma once

#include <DDSTextureLoader.h>
#include <wrl.h>
#include "TextureData.h"

namespace {
	static const std::wstring DEFAULT_CUBEMAP_LOCATION = L"res/textures/";
}

class DXCubeMap {

public:
	// Create a texture with data loaded from file
	DXCubeMap(const std::wstring& filename);
	~DXCubeMap();

	ID3D11ShaderResourceView** getResourceView();
	ID3D11Texture3D* getTexture3D();

private:
	ID3D11Texture3D* m_texture;
	ID3D11ShaderResourceView* m_resourceView;

};