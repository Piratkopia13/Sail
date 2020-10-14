#pragma once
#include "../entities/Entity.h"
#include "sail/api/Mesh.h"

class Texture;

// The environment holds the skybox, radiance and irradiance cubemap textures
class Environment {
public:
	Environment(Scene* scene, const std::string& folderName = "rail");
	~Environment();

	void changeTo(const std::string& folderName);

	Texture* getIrradianceTexture() const;
	Texture* getRadianceTexture() const;

private:
	void init(const std::string& folderName);

private:
	Entity m_skyboxEntity;

	Texture* m_irradianceMapTexture;
	Texture* m_radianceMapTexture;
};

