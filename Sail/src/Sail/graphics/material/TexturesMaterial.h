#pragma once

#include "Material.h"
#include <vector>
#include "../shader/Shaders.h"

class Texture;
class Shader;

class TexturesMaterial : public Material {
public:
	TexturesMaterial();
	~TexturesMaterial();

	virtual void bind(Shader* shader, Environment* environment, void* cmdList = nullptr) override;
	virtual void* getData() override;
	virtual unsigned int getDataSize() const override;
	Shader* getShader(Renderer::Type rendererType) const override;
	void setForwardShader(Shaders::ShaderIdentifier shaderId);

	// An empty filename will remove the texture
	void addTexture(const std::string& filename, bool useAbsolutePath = false);
	void clearTextures();

private:
	unsigned int m_numTextures;
	Shader* m_forwardShader;

};