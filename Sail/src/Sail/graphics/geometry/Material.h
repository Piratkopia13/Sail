#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include "sail/api/Texture.h"

class ShaderPipeline;

class Material {
public:
	typedef std::shared_ptr<Material> SPtr;
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
	Material(ShaderPipeline* ShaderPipeline);
	~Material();

	void bind();

	void setKa(float ka);
	void setKd(float kd);
	void setKs(float ks);
	void setShininess(float shininess);
	void setColor(const glm::vec4& color);

	void setDiffuseTexture(const std::string& filename);
	void setDiffuseTextureFromHandle(SailTexture srv);

	void setNormalTexture(const std::string& filename);
	void setNormalTextureFromHandle(SailTexture srv);

	void setSpecularTexture(const std::string& filename);
	void setSpecularTextureFromHandle(SailTexture srv);

	//void setTextures(ID3D11ShaderResourceView** srvs, UINT numTextures);

	/*	Returns an array of textures, numTextures get set
		Default texture order in array is as follows
		0 - Diffuse texture
		1 - Normal map
		2 - Specular map
	*/ 
	//ID3D11ShaderResourceView* const* getTextures(UINT& numTextures);

	//const glm::vec4& getColor() const;
	const PhongSettings& getPhongSettings() const;

	ShaderPipeline* getShader() const;

	//const bool* getTextureFlags() const;// TODO: remove

private:
	void getAndInsertTexture(const std::string& filename, int arraySlot);

private:
	ShaderPipeline* m_shader;

	PhongSettings m_phongSettings;
	SailTexture m_texHandles[3];

	UINT m_numTextures;

	//ID3D11ShaderResourceView** m_customSRVs;

};