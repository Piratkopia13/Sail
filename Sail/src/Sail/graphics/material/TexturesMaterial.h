#pragma once

#include "Material.h"
#include <vector>

class Texture;
class Shader;

class TexturesMaterial : public Material {
public:
	TexturesMaterial();
	~TexturesMaterial();

	virtual void bind(Shader* shader, Environment* environment, void* cmdList = nullptr) override;

	// An empty filename will remove the texture
	void addTexture(const std::string& filename, bool useAbsolutePath = false);
	void clearTextures();

private:
	std::vector<Texture*> m_textures;
	unsigned int m_numTextures;

};