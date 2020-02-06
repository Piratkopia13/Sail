#include "pch.h"
#include "TexturesMaterial.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/shader/Shader.h"

TexturesMaterial::TexturesMaterial()
	: Material(Material::TEXTURES)
{ }

TexturesMaterial::~TexturesMaterial() { }

void TexturesMaterial::bind(Shader* shader, void* cmdList) {
	ShaderPipeline* pipeline = shader->getPipeline();

	unsigned int i = 0;
	for (auto& texture : m_textures) {
		pipeline->setTexture("sys_tex"+std::to_string(i), texture, cmdList);
		i++;
	}
}

void TexturesMaterial::addTexture(const std::string& filename, bool useAbsolutePath) {
	m_textures.emplace_back(loadTexture(filename, useAbsolutePath));
}
