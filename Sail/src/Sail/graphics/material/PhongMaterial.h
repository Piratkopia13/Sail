#pragma once

#include "Material.h"

class Texture;
class Shader;

// Shared shader defines
#include "../Demo/res/shaders/variables.shared"

class PhongMaterial : public Material {

public:
	PhongMaterial();
	~PhongMaterial();

	virtual void setTextureIndex(unsigned int textureID, int index) override;
	virtual void* getData() override;
	virtual unsigned int getDataSize() const override;
	Shader* getShader(Renderer::Type rendererType) const override;

	void setKa(float ka);
	void setKd(float kd);
	void setKs(float ks);
	void setShininess(float shininess);
	void setColor(const glm::vec4& color);

	// An empty filename will remove the texture
	void setDiffuseTexture(const std::string& filename, bool useAbsolutePath = false);
	void setDiffuseTextureFromHandle(Texture* srv);

	// An empty filename will remove the texture
	void setNormalTexture(const std::string& filename, bool useAbsolutePath = false);
	void setNormalTextureFromHandle(Texture* srv);

	// An empty filename will remove the texture
	void setSpecularTexture(const std::string& filename, bool useAbsolutePath = false);
	void setSpecularTextureFromHandle(Texture* srv);

	/*	Returns a texture
		Default texture id is as follows
		0 - Diffuse texture
		1 - Normal map
		2 - Specular map
	*/ 
	Texture* getTexture(unsigned int id) const;

	ShaderShared::PhongMaterial& getPhongSettings();

private:
	ShaderShared::PhongMaterial m_phongSettings;

	unsigned int m_numTextures;

};