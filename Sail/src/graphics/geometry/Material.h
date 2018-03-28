#pragma once

#include <string>
#include <d3d11.h>
#include <SimpleMath.h>
#include "../../resources/DXTexture.h"
#include "../../api/Application.h"

class Material {

public:
	// Matching shader struct
	struct PhongSettings {
		DirectX::SimpleMath::Vector4 modelColor;
		float ka;
		float kd;
		float ks;
		float shininess;
		int hasDiffuseTexture;
		int hasNormalTexture;
		int hasSpecularTexture;
	};

public:
	Material(ShaderSet* shaderSet);
	~Material();

	void bind();

	void setKa(float ka);
	void setKd(float kd);
	void setKs(float ks);
	void setShininess(float shininess);
	void setColor(const DirectX::SimpleMath::Vector4& color);

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

	//const DirectX::SimpleMath::Vector4& getColor() const;
	const PhongSettings& getPhongSettings() const;

	ShaderSet* getShader() const;

	//const bool* getTextureFlags() const;// TODO: remove

private:
	void getAndInsertTexture(const std::string& filename, int arraySlot);

private:
	ShaderSet* m_shader;

	PhongSettings m_phongSettings;

	ID3D11ShaderResourceView* m_srvs[3];
	UINT m_numTextures;

	ID3D11ShaderResourceView** m_customSRVs;

};