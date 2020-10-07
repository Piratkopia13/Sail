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

	virtual void* getData() override;
	virtual unsigned int getDataSize() const override;
	Shader* getShader(Renderer::Type rendererType) const override;
	void setForwardShader(Shaders::ShaderIdentifier shaderId);

	void addTexture(RenderableTexture* texture);
	void addTexture(Texture* texture);
	void addTexture(const std::string& filename, bool useAbsolutePath = false); // An empty filename will remove the texture
	void clearTextures();

private:
	Shader* m_forwardShader;

};