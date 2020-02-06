#pragma once

#include <string>
#include "ResourceFormat.h"

class TextureData {
public:
	static const std::string DEFAULT_TEXTURE_LOCATION;

public:
	TextureData();
	TextureData(const std::string& filename, bool useAbsolutePath = false);
	~TextureData();

	void load(const std::string& filename, bool useAbsolutePath = false);

	ResourceFormat::TEXTURE_FORMAT getFormat() const;
	unsigned int getWidth() const;
	unsigned int getHeight() const;
	unsigned int getBytesPerPixel() const;
	unsigned char* getTextureData8bit() const;
	float* getTextureDataFloat() const;
	glm::vec4 getPixel(unsigned int x, unsigned int y);

private:
	ResourceFormat::TextureData m_data;

};