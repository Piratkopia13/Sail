#pragma once

#include <string>
#include "ResourceFormat.h"

class TextureData {
public:
	TextureData();
	~TextureData();
	bool load(const std::string& filename, bool useAbsolutePath = false);

	glm::vec4 getPixel(uint32_t x, uint32_t y); // Only works for rgba8 textures
	uint32_t getBytesPerPixel() const;
	ResourceFormat::TextureData& getData();

private:
	ResourceFormat::TextureData m_data;
};