#include "pch.h"
#include "Environment.h"
#include "Sail/Application.h"
#include "geometry/factory/Cube.h"
#include "../entities/components/Components.h"
#include "material/TexturesMaterial.h"

Environment::Environment(Scene* scene, const std::string& folderName) {
	// Create a skybox

	auto mesh = MeshFactory::Cube::Create(glm::vec3(0.5f));

	m_skyboxEntity = scene->createEntity("Skybox");
	m_skyboxEntity.addComponent<SkyboxComponent>(); // Used as an identifier to allow the scene to render this before anything else

	m_skyboxEntity.addComponent<MeshComponent>(mesh);
	m_skyboxEntity.addComponent<TransformComponent>(glm::vec3(0.f));
	std::shared_ptr<TexturesMaterial> mat = std::make_shared<TexturesMaterial>();
	mat->setForwardShader(Shaders::CubemapShader);
	m_skyboxEntity.addComponent<MaterialComponent>(mat);

	init(folderName);
}

Environment::~Environment() { }

void Environment::changeTo(const std::string& folderName) {
	init(folderName);
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

	auto mat = m_skyboxEntity.getComponent<MaterialComponent>();
	mat.get()->getAs<TexturesMaterial>()->clearTextures();
	mat.get()->getAs<TexturesMaterial>()->addTexture(skyboxFilename);
}
