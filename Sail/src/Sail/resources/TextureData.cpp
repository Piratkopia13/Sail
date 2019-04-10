#include "pch.h"
#include "TextureData.h"

TextureData::TextureData() {
	m_data.channels = 4;
	m_data.height = 0;
	m_data.width = 0;
	m_data.textureData = nullptr;
}
TextureData::TextureData(const std::string& filename) {
	load(filename);
}
TextureData::~TextureData() {
	Memory::safeDeleteArr(m_data.textureData);
}

void TextureData::load(const std::string& filename) {
	FileLoader::TGALoader TGALoader(DEFAULT_TEXTURE_LOCATION + filename, m_data);
}

unsigned int TextureData::getWidth() {
	return m_data.width;
}
unsigned int TextureData::getHeight() {
	return m_data.height;
}
unsigned char* TextureData::getTextureData() {
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