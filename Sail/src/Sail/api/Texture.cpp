#include "pch.h"
#include "Texture.h"
#include "Sail/Application.h"

TextureData& Texture::getTextureData(const std::string& filename) const {
	// Load the texture file it if is not loaded already
	if (!Application::getInstance()->getResourceManager().hasTextureData(filename)) {
		Application::getInstance()->getResourceManager().loadTextureData(filename);
	}
	return Application::getInstance()->getResourceManager().getTextureData(filename);
}
