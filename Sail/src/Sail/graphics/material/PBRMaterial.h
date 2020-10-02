#pragma once

#include "Material.h"

class Texture;
class Shader;

class PBRMaterial : public Material {
public:
	typedef std::shared_ptr<PBRMaterial> SPtr;

public:
	// Matching shader struct
	struct PBRSettings {
		glm::vec4 modelColor;
		float metalnessScale;
		float roughnessScale;
		float aoIntensity;
		int albedoTexIndex;
		int normalTexIndex;
		int mraoTexIndex; // R/G/B = Metalness/Roughness/Ambient occlusion
		int radianceMapTexIndex;
		int irradianceMapTexIndex;
		int brdfLutTexIndex;
		glm::vec3 padding;
	};

public:
	PBRMaterial();
	~PBRMaterial();

	virtual void bind(Shader* shader, Environment* environment, void* cmdList = nullptr) override;
	virtual void setEnvironment(Environment* environment) override;
	virtual void setTextureIndex(unsigned int textureID, int index) override;
	virtual void* getData() override;
	virtual unsigned int getDataSize() const override;
	Shader* getShader(Renderer::Type rendererType) const override;

	void setMetalnessScale(float metalness);
	void setRoughnessScale(float roughness);
	void setAOIntensity(float intensity);
	void setColor(const glm::vec4& color);
	void enableTransparency(bool enable); // If set to true - meshes will be drawn with alpha blending enabled in a forward pass

	// An empty filename will remove the texture
	void setAlbedoTexture(const std::string& filename, bool useAbsolutePath = false);
	void setNormalTexture(const std::string& filename, bool useAbsolutePath = false);
	void setMetalnessRoughnessAOTexture(const std::string& filename, bool useAbsolutePath = false);

	/*	Returns a texture
		Default texture id is as follows
		0 - Albedo
		1 - Normal map
		2 - Metalness/Roughness/AO as r/g/b
	*/
	Texture* getTexture(unsigned int id) const;

	PBRSettings& getPBRSettings();

private:
	PBRSettings m_pbrSettings;
	Texture* m_brdfLutTexture;
	bool m_hasTransparency;	
	UINT m_numTextures;
};