#include "pch.h"
#include "TexturesMaterial.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/api/shader/Shader.h"

TexturesMaterial::TexturesMaterial()
	: Material(Material::TEXTURES)
{ }

TexturesMaterial::~TexturesMaterial() { }

void TexturesMaterial::bind(Shader* shader, Environment* environment, void* cmdList) {
	unsigned int i = 0;
	for (auto& texture : m_textures) {
		shader->setTexture("sys_tex"+std::to_string(i), texture, cmdList);
		i++;
	}
}

void TexturesMaterial::addTexture(const std::string& filename, bool useAbsolutePath) {
	m_textures.emplace_back(loadTexture(filename, useAbsolutePath));
}

void TexturesMaterial::clearTextures() {
	m_textures.clear();
}
