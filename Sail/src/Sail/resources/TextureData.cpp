#include "pch.h"
#include "TextureData.h"

const std::string TextureData::DEFAULT_TEXTURE_LOCATION = "res/textures/";

TextureData::TextureData() {
	m_data.channels = 4;
	m_data.height = 0;
	m_data.width = 0;
	m_data.textureData = nullptr;
}
TextureData::TextureData(const std::string& filename, bool useAbsolutePath) {
	load(filename, useAbsolutePath);
}
TextureData::~TextureData() {
	Memory::SafeDeleteArr(m_data.textureData);
}

void TextureData::load(const std::string& filename, bool useAbsolutePath) {
	std::string path = (useAbsolutePath) ? filename : DEFAULT_TEXTURE_LOCATION + filename;
	FileLoader::TGALoader TGALoader(path, m_data);
}

unsigned int TextureData::getWidth() const {
	return m_data.width;
}
unsigned int TextureData::getHeight() const {
	return m_data.height;
}

unsigned int TextureData::getBytesPerPixel() const {
	// TODO: change bitsPerChannel to match the loaded image
	unsigned int bitsPerChannel = 8;
	return (m_data.channels * bitsPerChannel) / 8;
}

unsigned char* TextureData::getTextureData() const {
	return m_data.textureData;
}
glm::vec4 TextureData::getPixel(unsigned int x, unsigned int y) {

	if (x < 0 || x > m_data.width - 1) return glm::vec4(0.f);
	if (y < 0 || y > m_data.height - 1) return glm::vec4(0.f);

	return glm::vec4(	m_data.textureData[y * m_data.width * m_data.channels + (x * m_data.channels)],
					m_data.textureData[y * m_data.width * m_data.channels + (x * m_data.channels) + 1],
					m_data.textureData[y * m_data.width * m_data.channels + (x * m_data.channels) + 2],
					m_data.textureData[y * m_data.width * m_data.channels + (x * m_data.channels) + 3]);

}