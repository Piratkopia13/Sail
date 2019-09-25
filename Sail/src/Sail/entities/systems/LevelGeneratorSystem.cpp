#include "pch.h"
#include "LevelGeneratorSystem.h"
#include "..//Sail/src/Sail/entities/ECS.h"
#include "..//Sail/src/Sail/entities/components/Components.h"
#include "../..//graphics/Scene.h"

LevelGeneratorSystem::LevelGeneratorSystem() {
}

LevelGeneratorSystem::~LevelGeneratorSystem() {
}

void LevelGeneratorSystem::update(float dt) {
}

void LevelGeneratorSystem::createWorld(Scene* scene, Model* model, Model* bb) {

	int tileSize = 5;
	int worldWidth = 20;
	int worldDepth = 20;

	int random = rand() % 100 + 1;     // 1 to 100
	
	for (int i = 0; i < worldWidth; i++) {
		for (int j = 0; j < worldDepth; j++) {
			auto e = ECS::Instance()->createEntity("");
			e->addComponent<ModelComponent>(model);
			if (rand() % 100 > 70) {
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * i, 0.f, tileSize * j));
			}
			else {
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * i, -5.f, tileSize * j));
			}
			
			e->addComponent<BoundingBoxComponent>(bb);
			e->addComponent<CollidableComponent>();
			scene->addStaticEntity(e);
		}
	}


}
