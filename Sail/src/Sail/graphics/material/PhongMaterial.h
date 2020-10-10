#pragma once

#include "Material.h"

class Texture;
class Shader;

class PhongMaterial : public Material {
public:
	// Matching shader struct
	struct PhongSettings {
		glm::vec4 modelColor;
		float ka;
		float kd;
		float ks;
		float shininess;
		int diffuseTexIndex;
		int normalTexIndex;
		int specularTexIndex;
		float padding;
	};

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

	PhongSettings& getPhongSettings();

private:
	PhongSettings m_phongSettings;

	unsigned int m_numTextures;

};