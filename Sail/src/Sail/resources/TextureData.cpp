#include "pch.h"
#include "TextureData.h"
#include "loaders/TGAImageLoader.h"
#include "loaders/STBImageLoader.h"
#include "loaders/DDSKTXImageLoader.h"

const std::string TextureData::DEFAULT_TEXTURE_LOCATION = "res/textures/";

TextureData::TextureData() {
	m_data.channels = 4;
	m_data.height = 0;
	m_data.width = 0;
	m_data.data = nullptr;
}
TextureData::TextureData(const std::string& filename, bool useAbsolutePath) {
	load(filename, useAbsolutePath);
}
TextureData::~TextureData() {
	Memory::SafeDeleteArr(m_data.data);
}

void TextureData::load(const std::string& filename, bool useAbsolutePath) {
	std::string path = (useAbsolutePath) ? filename : DEFAULT_TEXTURE_LOCATION + filename;

	auto ext = path.substr(path.length() - 3);
	if (ext == "hdr" || ext == "jpg" || ext == "png") {
		FileLoader::STBImageLoader(path, m_data);
	} else if (ext == "dds" || ext == "ktx") {
		FileLoader::DDSKTXImageLoader(path, m_data);
	} else if (ext == "tga") {
		FileLoader::TGAImageLoader TGALoader(path, m_data);
	} else {
		Logger::Error("Tried to load unsupported texture format: " + ext);
	}
}

ResourceFormat::TextureFormat TextureData::getFormat() const {
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

void* TextureData::getData() const {
	return m_data.data;
}

glm::vec4 TextureData::getPixel(unsigned int x, unsigned int y) {

	assert(m_data.format == ResourceFormat::R8G8B8A8); // TODO: Add support for other formats

	if (x < 0 || x > m_data.width - 1) return glm::vec4(0.f);
	if (y < 0 || y > m_data.height - 1) return glm::vec4(0.f);

	unsigned char* dataAsUChar = static_cast<unsigned char*>(m_data.data);
	return glm::vec4(	dataAsUChar[y * m_data.width * m_data.channels + (x * m_data.channels)],
						dataAsUChar[y * m_data.width * m_data.channels + (x * m_data.channels) + 1],
						dataAsUChar[y * m_data.width * m_data.channels + (x * m_data.channels) + 2],
						dataAsUChar[y * m_data.width * m_data.channels + (x * m_data.channels) + 3]);

}

bool TextureData::isCubeMap() const {
	return m_data.isCubeMap;
}

bool TextureData::isSRGB() const {
	return m_data.isSRGB;
}

unsigned int TextureData::getAllocatedMemorySize() const {
	return m_data.byteSize;
}
