#include "pch.h"
#include "Material.h"
#include "Sail/Application.h"
#include "PhongMaterial.h"
#include "PBRMaterial.h"
#include "TexturesMaterial.h"

Material::Material(Type type) : m_type(type) { }

Material::~Material() { }

Material::Type Material::getType() const {
	return m_type;
}

Texture* Material::loadTexture(const std::string& filename, bool useAbsolutePath) {
	Texture* t = nullptr;
	if (!filename.empty()) {
		auto& resMan = Application::getInstance()->getResourceManager();
		if (!resMan.hasTexture(filename)) {
			Logger::Warning("Texture (" + filename + ") was not yet loaded, loading now.");
			resMan.loadTexture(filename, useAbsolutePath);
		}
		t = &resMan.getTexture(filename);
	}
	return t;
}
