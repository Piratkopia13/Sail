#include "pch.h"
#include "TextureData.h"
#include "loaders/TGAImageLoader.h"
#include "loaders/STBImageLoader.h"
#include "loaders/DDSKTXImageLoader.h"
#include "ResourceManager.h"

TextureData::TextureData() {
	m_data.channels = 4;
	m_data.height = 0;
	m_data.width = 0;
	m_data.data = nullptr;
}
TextureData::~TextureData() {
	Memory::SafeDeleteArr(m_data.data);
}

bool TextureData::load(const std::string& filename, bool useAbsolutePath) {
	std::string path = (useAbsolutePath) ? filename : ResourceManager::DEFAULT_TEXTURE_LOCATION + filename;

	auto ext = path.substr(path.length() - 3);
	if (ext == "dds" || ext == "ktx") {
		return FileLoader::DDSKTXImageLoader(path, m_data);
	} else {
		// Try loading using STB, note that this might fail on unsupported formats and should be handled
		return FileLoader::STBImageLoader(path, m_data);
	}

	return true;
}

uint32_t TextureData::getBytesPerPixel() const {
	return (m_data.channels * m_data.bitsPerChannel) / 8;
}

ResourceFormat::TextureData& TextureData::getData() {
	return m_data;
}

glm::vec4 TextureData::getPixel(uint32_t x, uint32_t y) {
	assert(m_data.format == ResourceFormat::R8G8B8A8); // TODO: Add support for other formats

	if (x < 0 || x > m_data.width - 1) return glm::vec4(0.f);
	if (y < 0 || y > m_data.height - 1) return glm::vec4(0.f);

	unsigned char* dataAsUChar = static_cast<unsigned char*>(m_data.data);
	return glm::vec4(	dataAsUChar[y * m_data.width * m_data.channels + (x * m_data.channels)],
						dataAsUChar[y * m_data.width * m_data.channels + (x * m_data.channels) + 1],
						dataAsUChar[y * m_data.width * m_data.channels + (x * m_data.channels) + 2],
						dataAsUChar[y * m_data.width * m_data.channels + (x * m_data.channels) + 3]);
}