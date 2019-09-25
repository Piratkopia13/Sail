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
			//	map->m_tiles.push_back(rand() % 5);
				map->tileArr[i][j] = rand() % 12;
			}
		}
	}
}
void LevelGeneratorSystem::createWorld(Scene* scene, Model* tileFlat, Model* tileCross,Model* tileCorner,Model* tileStraight, Model* tileT, Model* bb) {

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
			//int tileId = map->m_tiles[i * worldWidth + j];
			int tileId = map->tileArr[i][j];
			if (tileId==0) {
				e->addComponent<ModelComponent>(tileFlat);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}
			else if(tileId==1){
				e->addComponent<ModelComponent>(tileCross);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}
			else if (tileId == 2) {
				e->addComponent<ModelComponent>(tileStraight);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}
			else if (tileId == 3) {
				e->addComponent<ModelComponent>(tileStraight);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)),glm::vec3(0.f,glm::radians(90.f),0.f));
			}
			else if (tileId == 4) {
				e->addComponent<ModelComponent>(tileCorner);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}
			else if (tileId == 5) {
				e->addComponent<ModelComponent>(tileCorner);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(90.f), 0.f));
			}
			else if (tileId == 6) {
				e->addComponent<ModelComponent>(tileCorner);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(180.f), 0.f));
			}
			else if (tileId == 7) {
				e->addComponent<ModelComponent>(tileCorner);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(270.f), 0.f));
			}
			else if (tileId == 8) {
				e->addComponent<ModelComponent>(tileT);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}
			else if (tileId == 9) {
				e->addComponent<ModelComponent>(tileT);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(90.f), 0.f));
			}
			else if (tileId == 10) {
				e->addComponent<ModelComponent>(tileT);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(180.f), 0.f));
			}
			else {
				e->addComponent<ModelComponent>(tileT);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(270.f), 0.f));
			}
			e->addComponent<BoundingBoxComponent>(bb);
			e->addComponent<CollidableComponent>();
			scene->addStaticEntity(e);
		}
	}
}
