#include "pch.h"
#include "BotFactory.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/candles/CandleSystem.h"

BotFactory::BotFactory() {
	p_ECS = ECS::Instance();
}

BotFactory::~BotFactory() {

}

void BotFactory::createBots(std::vector<glm::vec3> spawnPositions) {
	for (size_t i = 0; i < spawnPositions.size(); i++) 	{

		auto e = p_ECS->createEntity("AiCharacter");
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(glm::vec3(5.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		e->addComponent<PhysicsComponent>();
		e->addComponent<AiComponent>();
		e->addComponent<GunComponent>(m_cubeModel.get(), m_boundingBoxModel.get());
		e->addChildEntity(createCandleEntity("AiCandle", lightModel, glm::vec3(0.f, 2.f, 0.f)));

	}

}

void BotFactory::createCharacterModel() {
	Model* characterModel = &Application::getInstance()->getResourceManager().getModel("character1.fbx", shader);
	characterModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");
}


