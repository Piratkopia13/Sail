#include "pch.h"
#include "BotFactory.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/candles/CandleSystem.h"
#include "Sail/graphics/geometry/factory/CubeModel.h"

BotFactory::BotFactory() {
	p_ECS = ECS::Instance();
}

BotFactory::~BotFactory() {

}

void BotFactory::createBots(std::vector<glm::vec3> spawnPositions) {
	// Shader
#ifdef DISABLE_RT
	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet<MaterialShader>();
#else
	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet<GBufferOutShader>();
#endif

	// BoundingBox Model
	auto* wireframeShader = &Application::getInstance()->getResourceManager().getShaderSet<WireframeShader>();
	Model m_boundingBoxModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f), wireframeShader);
	
	// Cube Model
	Model m_cubeModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f), shader);

	// Character Model
	Model* characterModel = &Application::getInstance()->getResourceManager().getModel("character1.fbx", shader);
	characterModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");

	// Light Model
	Model* lightModel = &Application::getInstance()->getResourceManager().getModel("candleExported.fbx", shader);



	for (size_t i = 0; i < spawnPositions.size(); i++) 	{

		auto e = p_ECS->createEntity("AiCharacter");
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(glm::vec3(5.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel);
		e->addComponent<CollidableComponent>();
		e->addComponent<PhysicsComponent>();
		e->addComponent<AiComponent>();
		e->addComponent<GunComponent>(m_cubeModel, m_boundingBoxModel);
		e->addChildEntity(createCandleEntity("AiCandle", lightModel, glm::vec3(0.f, 2.f, 0.f)));

	}

}

void BotFactory::initialize() {
	if (m_isInitialized == false) {
		m_isInitialized = true;

		
	}
}

void BotFactory::createCharacterModel() {
	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet<GBufferOutShader>();
	Model* characterModel = &Application::getInstance()->getResourceManager().getModel("character1.fbx", shader);
	characterModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");
}


