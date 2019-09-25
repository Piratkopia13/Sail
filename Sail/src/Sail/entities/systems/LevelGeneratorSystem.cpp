#include "pch.h"
#include "LevelGeneratorSystem.h"
#include "..//Sail/src/Sail/entities/ECS.h"
#include "..//Sail/src/Sail/entities/components/Components.h"
#include "../..//graphics/Scene.h"
#include "..//components/MapComponent.h"
#include "..//Entity.h"
LevelGeneratorSystem::LevelGeneratorSystem():BaseComponentSystem() {
	requiredComponentTypes.push_back(MapComponent::ID);
}

LevelGeneratorSystem::~LevelGeneratorSystem() {
}

void LevelGeneratorSystem::update(float dt) {
}
void LevelGeneratorSystem::generateMap() {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();
			for (int i = 0; i < map->xsize; i++) {
				for (int j = 0; j < map->ysize; j++) {
					if (rand() % 2 == 0) {
						//1 is cross
						map->m_tiles.push_back(1);
					}
					else {
						//0 is flat
						map->m_tiles.push_back(0);
					}
				}
			}
	}
}
void LevelGeneratorSystem::createWorld(Scene* scene, Model* tile1, Model* tile2,Model* bb) {

	MapComponent* map;
	for (auto& f : entities) {
		map = f->getComponent<MapComponent>();
	}
	int random = rand() % 100 + 1;     // 1 to 100
	int tileSize = 10;
	int worldWidth = map->xsize;
	int worldDepth = map->ysize;

	for (int i = 0; i < worldWidth; i++) {
		for (int j = 0; j < worldDepth; j++) {
			auto e = ECS::Instance()->createEntity("");
			if (map->m_tiles[i*worldWidth+j]==1) {
				e->addComponent<ModelComponent>(tile1);
			}
			else {
				e->addComponent<ModelComponent>(tile2);
			}
			e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i+2), 0.f, tileSize * (j+2)));
			e->addComponent<CollidableComponent>();
			e->addComponent<BoundingBoxComponent>(bb);
			scene->addStaticEntity(e);
		}
	}
}
