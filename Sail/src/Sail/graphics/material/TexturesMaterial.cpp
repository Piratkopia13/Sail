#include "pch.h"
#include "TexturesMaterial.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"

TexturesMaterial::TexturesMaterial()
	: Material(Material::TEXTURES)
	, m_forwardShader(nullptr)
{ }

TexturesMaterial::~TexturesMaterial() { }

void TexturesMaterial::bind(Shader* shader, Environment* environment, void* cmdList) {
	unsigned int i = 0;
	for (auto& texture : textures) {
		shader->setTexture("sys_tex"+std::to_string(i), texture, cmdList);
		i++;
	}
}

void* TexturesMaterial::getData() {
	assert(false && "Not implemented");
	return 0;
}

unsigned int TexturesMaterial::getDataSize() const {
	assert(false && "Not implemented");
	return 0;
}

Shader* TexturesMaterial::getShader(Renderer::Type rendererType) const {
	switch (rendererType) {
	case Renderer::FORWARD:
		return m_forwardShader;
		break;
	default:
		return nullptr;
		break;
	}
}

void TexturesMaterial::setForwardShader(Shaders::ShaderIdentifier shaderId) {
	m_forwardShader = &Application::getInstance()->getResourceManager().getShaderSet(shaderId);
}

void TexturesMaterial::addTexture(const std::string& filename, bool useAbsolutePath) {
	textures.emplace_back(loadTexture(filename, useAbsolutePath));
}

void TexturesMaterial::addTexture(Texture* texture) {
	textures.emplace_back(texture);
}

void TexturesMaterial::addTexture(RenderableTexture* texture) {
	renderableTextures.emplace_back(texture);
}

void TexturesMaterial::clearTextures() {
	textures.clear();
	renderableTextures.clear();
}
