#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <string>
#include "loaders/TGALoader.h"
#include "ResourceFormat.h"

namespace {
	static const std::string DEFAULT_TEXTURE_LOCATION = "res/textures/";
}

class TextureData {

public:
	TextureData();
	TextureData(const std::string& filename);
	~TextureData();

	void load(const std::string& filename);

	unsigned int getWidth();
	unsigned int getHeight();
	unsigned char* getTextureData();
	DirectX::SimpleMath::Vector4 getPixel(unsigned int x, unsigned int y);

private:
	ResourceFormat::TextureData m_data;

};