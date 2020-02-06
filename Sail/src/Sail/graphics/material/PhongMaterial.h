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
		int hasDiffuseTexture;
		int hasNormalTexture;
		int hasSpecularTexture;
	};

public:
	PhongMaterial();
	~PhongMaterial();

	virtual void bind(Shader* shader, void* cmdList = nullptr) override;

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

	//void setTextures(ID3D11ShaderResourceView** srvs, UINT numTextures);

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
	Texture* m_textures[3];

	unsigned int m_numTextures;

};