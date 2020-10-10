#include "pch.h"
#include "Texture.h"
#include "Sail/Application.h"

Texture::Texture(const std::string& filename)
	: m_name(filename)
	, texIsCubeMap(false)
	, readyToUse(false)
{ }

const std::string& Texture::getName() const {
	return m_name;
}

bool Texture::isCubeMap() const {
	return texIsCubeMap;
}

bool Texture::isReadyToUse() const {
	return readyToUse;
}

TextureData& Texture::getTextureData(const std::string& filename, bool useAbsolutePath) const {
	// Load the texture file it if is not loaded already
	auto& rm = Application::getInstance()->getResourceManager();
	if (!rm.hasTextureData(filename)) {
		rm.loadTextureData(filename, useAbsolutePath);
	}
	return rm.getTextureData(filename);
}