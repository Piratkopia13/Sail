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
	int offset = 17;

	int random = rand() % 100 + 1;     // 1 to 100
	
	for (int i = 0; i < worldWidth; i++) {
		for (int j = 0; j < worldDepth; j++) {
			auto e = ECS::Instance()->createEntity("");
			e->addComponent<ModelComponent>(model);
			if (rand() % 100 > 70) {
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * i + offset, 0.f, tileSize * j + offset));
			}
			else {
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * i + offset, -5.f, tileSize * j + offset));
			}
			
			e->addComponent<BoundingBoxComponent>(bb);
			e->addComponent<CollidableComponent>();
			scene->addStaticEntity(e);
		}
	}


}
