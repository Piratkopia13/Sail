#pragma once
#include "../entities/Entity.h"
#include "sail/api/Mesh.h"

class Texture;

// The environment holds the skybox, radiance and irradiance cubemap textures
class Environment {
public:
	Environment(const std::string& folderName = "studio");
	~Environment();

	void changeTo(const std::string& folderName);

	Entity::SPtr& getSkyboxEntity();
	Texture* getIrradianceTexture() const;
	Texture* getRadianceTexture() const;

private:
	void init(const std::string& folderName);

private:
	std::shared_ptr<Mesh> m_skyboxMesh;
	Entity::SPtr m_skyboxEntity;

	Texture* m_irradianceMapTexture;
	Texture* m_radianceMapTexture;
};

