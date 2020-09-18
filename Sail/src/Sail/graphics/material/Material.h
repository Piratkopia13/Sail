#pragma once

#include <memory>
#include <string>
#include "Sail/api/Renderer.h"

class Shader;
class PhongMaterial;
class PBRMaterial;
class TexturesMaterial;
class Texture;
class Environment;

class Material {
public:
	typedef std::shared_ptr<Material> SPtr;
public:
	enum Type {
		NONE = 0,
		PHONG, PBR,
		TEXTURES, OUTLINE,
		CUSTOM
	};

	Material(Type type);
	~Material();

	virtual void bind(Shader* shader, Environment* environment, void* cmdList = nullptr) = 0;
	
	virtual void setEnvironment(Environment* environment) { };
	virtual void setTextureIndex(unsigned int textureID, unsigned int index) { };

	virtual void* getData() = 0;
	virtual unsigned int getDataSize() const = 0;

	// Get the shader to use this material with the given renderer, may return nullptr when the renderer is not supported
	virtual Shader* getShader(Renderer::Type rendererType) const = 0;

	const std::vector<Texture*>& getTextures() const;

	Type getType() const;

	template <typename T>
	T* getAs() {
		return dynamic_cast<T*>(this);
	}

protected:
	Texture* loadTexture(const std::string& filename, bool useAbsolutePath = false);
	std::vector<Texture*> textures;

private:
	Type m_type;
	
};
