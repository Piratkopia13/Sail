#pragma once

#include <memory>
#include <string>

class Shader;
class PhongMaterial;
class PBRMaterial;
class Texture;

class Material {
public:
	typedef std::shared_ptr<Material> SPtr;
public:
	enum Type {
		None = 0,
		PHONG, PBR
	};

	Material(Type type);;
	~Material();;

	virtual void bind(Shader* shader, void* cmdList = nullptr) = 0;

	Type getType() const;

	PhongMaterial* asPhong();
	PBRMaterial* asPBR();

protected:
	Texture* loadTexture(const std::string& filename, bool useAbsolutePath = false);

private:
	Type m_type;
	
};
