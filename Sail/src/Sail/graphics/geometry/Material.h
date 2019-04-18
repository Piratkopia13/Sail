#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>

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
	Material(ShaderPipeline* shaderSet);
	~Material();

	void bind();

	void setKa(float ka);
	void setKd(float kd);
	void setKs(float ks);
	void setShininess(float shininess);
	void setColor(const glm::vec4& color);

	void setDiffuseTexture(const std::string& filename);
	void setDiffuseTexture(ID3D11ShaderResourceView* srv);

	void setNormalTexture(const std::string& filename);
	void setNormalTexture(ID3D11ShaderResourceView* srv);

	void setSpecularTexture(const std::string& filename);
	void setSpecularTexture(ID3D11ShaderResourceView* srv);

	void setTextures(ID3D11ShaderResourceView** srvs, UINT numTextures);

	/*	Returns an array of textures, numTextures get set
		Default texture order in array is as follows
		0 - Diffuse texture
		1 - Normal map
		2 - Specular map
	*/ 
	ID3D11ShaderResourceView* const* getTextures(UINT& numTextures);

	//const glm::vec4& getColor() const;
	const PhongSettings& getPhongSettings() const;

	ShaderPipeline* getShader() const;

	//const bool* getTextureFlags() const;// TODO: remove

private:
	void getAndInsertTexture(const std::string& filename, int arraySlot);

private:
	ShaderPipeline* m_shader;

	PhongSettings m_phongSettings;

	ID3D11ShaderResourceView* m_srvs[3];
	UINT m_numTextures;

	ID3D11ShaderResourceView** m_customSRVs;

};