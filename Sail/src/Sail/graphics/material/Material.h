#pragma once

#include <memory>
#include <string>

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
		TEXTURES, OUTLINE
	};

	Material(Type type);;
	~Material();;

	virtual void bind(Shader* shader, Environment* environment, void* cmdList = nullptr) = 0;

	Type getType() const;

	template <typename T>
	T* getAs() {
		return dynamic_cast<T*>(this);
	}

protected:
	Texture* loadTexture(const std::string& filename, bool useAbsolutePath = false);

private:
	Type m_type;
	
};
