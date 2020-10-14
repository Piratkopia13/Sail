#pragma once

#include "Material.h"

class Texture;
class Shader;

// Shared shader defines
#include "../Demo/res/shaders/variables.shared"

class PBRMaterial : public Material {
public:
	typedef std::shared_ptr<PBRMaterial> SPtr;

public:
	PBRMaterial();
	~PBRMaterial();

	virtual void setEnvironment(const Environment& environment) override;
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
	void setMetalnessTexture(const std::string& filename, bool useAbsolutePath = false);
	void setRoughnessTexture(const std::string& filename, bool useAbsolutePath = false);
	void setAoTexture(const std::string& filename, bool useAbsolutePath = false);


	/*	Returns a texture
		Default texture id is as follows
		0 - Albedo
		1 - Normal map
		2 - Metalness/Roughness/AO as r/g/b
	*/
	Texture* getTexture(unsigned int id) const;

	ShaderShared::PBRMaterial& getPBRSettings();

private:
	ShaderShared::PBRMaterial m_pbrSettings;
	Texture* m_brdfLutTexture;
	bool m_hasTransparency;	
	UINT m_numTextures;
};