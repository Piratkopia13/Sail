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

	ResourceFormat::TextureFormat getFormat() const;
	unsigned int getWidth() const;
	unsigned int getHeight() const;
	unsigned int getBytesPerPixel() const;
	void* getData() const;
	glm::vec4 getPixel(unsigned int x, unsigned int y);
	bool isCubeMap() const;
	bool isSRGB() const;
	int getMipLevels() const;
	const std::vector<glm::int2>& getMipExtents() const;
	const std::vector<unsigned int>& getMipOffsets() const;

	unsigned int getAllocatedMemorySize() const;

private:
	ResourceFormat::TextureData m_data;

};