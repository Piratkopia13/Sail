#include "pch.h"
#include "TextureData.h"
#include "loaders/TGALoader.h"
#include "loaders/STBImageLoader.h"

const std::string TextureData::DEFAULT_TEXTURE_LOCATION = "res/textures/";

TextureData::TextureData() {
	m_data.channels = 4;
	m_data.height = 0;
	m_data.width = 0;
	m_data.textureDataFloat = nullptr;
	m_data.textureData8bit = nullptr;
}
TextureData::TextureData(const std::string& filename, bool useAbsolutePath) {
	load(filename, useAbsolutePath);
}
TextureData::~TextureData() {
	Memory::SafeDeleteArr(m_data.textureData8bit);
	Memory::SafeDeleteArr(m_data.textureDataFloat);
}

void TextureData::load(const std::string& filename, bool useAbsolutePath) {
	std::string path = (useAbsolutePath) ? filename : DEFAULT_TEXTURE_LOCATION + filename;

	if (path.substr(path.length() - 3) == "hdr") {
		FileLoader::STBImageLoader(path, m_data);
	} else {
		FileLoader::TGALoader TGALoader(path, m_data);
	}
}

ResourceFormat::TEXTURE_FORMAT TextureData::getFormat() const {
	return m_data.format;
}

unsigned int TextureData::getWidth() const {
	return m_data.width;
}
unsigned int TextureData::getHeight() const {
	return m_data.height;
}

unsigned int TextureData::getBytesPerPixel() const {
	return (m_data.channels * m_data.bitsPerChannel) / 8;
}

unsigned char* TextureData::getTextureData8bit() const {
	return m_data.textureData8bit;
}

float* TextureData::getTextureDataFloat() const {
	return m_data.textureDataFloat;
}
glm::vec4 TextureData::getPixel(unsigned int x, unsigned int y) {

	assert(m_data.format == ResourceFormat::R8G8B8A8); // TODO: Add support for other formats

	if (x < 0 || x > m_data.width - 1) return glm::vec4(0.f);
	if (y < 0 || y > m_data.height - 1) return glm::vec4(0.f);

	return glm::vec4(	m_data.textureData8bit[y * m_data.width * m_data.channels + (x * m_data.channels)],
					m_data.textureData8bit[y * m_data.width * m_data.channels + (x * m_data.channels) + 1],
					m_data.textureData8bit[y * m_data.width * m_data.channels + (x * m_data.channels) + 2],
					m_data.textureData8bit[y * m_data.width * m_data.channels + (x * m_data.channels) + 3]);

}