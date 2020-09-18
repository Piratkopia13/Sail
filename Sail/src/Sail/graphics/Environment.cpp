#include "pch.h"
#include "Environment.h"
#include "Sail/Application.h"
#include "geometry/factory/CubeModel.h"
#include "../entities/components/Components.h"
#include "material/TexturesMaterial.h"

Environment::Environment(const std::string& folderName) {
	// Create a skybox

	m_skyboxModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f));

	m_skyboxEntity = Entity::Create("Skybox");
	m_skyboxEntity->addComponent<ModelComponent>(m_skyboxModel);
	m_skyboxEntity->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
	auto& mat = m_skyboxEntity->addComponent<MaterialComponent<TexturesMaterial>>();
	//mat->get()->setForwardShader(Shaders::CubemapShader);

	init(folderName);
}

Environment::~Environment() {

}

void Environment::changeTo(const std::string& folderName) {
	init(folderName);
}

Entity::SPtr& Environment::getSkyboxEntity() {
	return m_skyboxEntity;
}

Texture* Environment::getIrradianceTexture() const {
	return m_irradianceMapTexture;
}

Texture* Environment::getRadianceTexture() const {
	return m_radianceMapTexture;
}

void Environment::init(const std::string& folderName) {
	auto* app = Application::getInstance();
	auto& resman = app->getResourceManager();

	std::string irradianceFilename = "environments/" + folderName + "/iem.dds";
	std::string radianceFilename = "environments/" + folderName + "/pmrem.dds";
	std::string skyboxFilename = "environments/" + folderName + "/skybox.dds";

	resman.loadTexture(irradianceFilename);
	m_irradianceMapTexture = &resman.getTexture(irradianceFilename);

	resman.loadTexture(radianceFilename);
	m_radianceMapTexture = &resman.getTexture(radianceFilename);

	auto mat = m_skyboxEntity->getComponent<MaterialComponent<Material>>();
	mat->get()->getAs<TexturesMaterial>()->clearTextures();
	mat->get()->getAs<TexturesMaterial>()->addTexture(skyboxFilename);
}
