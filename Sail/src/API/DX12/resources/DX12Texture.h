#pragma once

#include "Sail/resources/TextureData.h"
#include "Sail/api/Texture.h"

class DX12Texture : public Texture {
public:
	DX12Texture(const std::string& filename);
	~DX12Texture();

	virtual SailTexture* getHandle() override;

};