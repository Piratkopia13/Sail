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
		//set corners
		{
			map->tileArr[0][0] = 3;
			map->tileArr[0][map->ysize-1] =6;
			map->tileArr[map->xsize-1][map->ysize-1] = 12;
			map->tileArr[map->xsize-1][0] = 9;
		}
		std::vector<int> avaliableTiles;
		//set edges
		{
			avaliableTiles.push_back(5);
			avaliableTiles.push_back(7);
			for (int i = 1; i < map->ysize - 1; i++) {
				map->tileArr[0][i] = randomizeTileId(&avaliableTiles,0);
			}
			avaliableTiles.clear();

			avaliableTiles.push_back(10);
			avaliableTiles.push_back(11);
			for (int i = 1; i < map->xsize - 1; i++) {
				map->tileArr[i][0] = randomizeTileId(&avaliableTiles, 0);
			}
			avaliableTiles.clear();	
			avaliableTiles.push_back(5);
			avaliableTiles.push_back(13);
			for (int i = 1; i < map->ysize - 1; i++) {
				map->tileArr[map->xsize-1][i] = randomizeTileId(&avaliableTiles, 0);
			}
			avaliableTiles.clear();
			avaliableTiles.push_back(10);
			avaliableTiles.push_back(14);
			for (int i = 1; i < map->xsize - 1; i++) {
				map->tileArr[i][map->ysize-1] = randomizeTileId(&avaliableTiles, 0);
			}
			avaliableTiles.clear();
		}
		//set center room
		{
			int centerx = map->xsize / 2, centery = map->ysize / 2;
			//floor
			map->tileArr[centerx][centery] = 0;
			map->tileArr[centerx+1][centery] = 0;
			map->tileArr[centerx+1][centery-1] = 0;
			map->tileArr[centerx][centery-1] =0 ;
			map->tileArr[centerx-1][centery-1] = 0;
			map->tileArr[centerx-1][centery] =0 ;
			map->tileArr[centerx-1][centery+1] =0 ;
			map->tileArr[centerx][centery+1] = 0;
			map->tileArr[centerx+1][centery+1] = 0;
			map->tileArr[centerx][centery-2] = 0;
			map->tileArr[centerx][centery+2] = 0;
			map->tileArr[centerx-2][centery] = 0;
			map->tileArr[centerx+2][centery] = 0;

			//walls
			map->tileArr[centerx-2][centery+2] = 6;
			map->tileArr[centerx-1][centery+2] = 9;
			map->tileArr[centerx+1][centery+2] = 3;
			map->tileArr[centerx+2][centery+2] = 12;
			map->tileArr[centerx-2][centery-2] = 3;
			map->tileArr[centerx-1][centery-2] = 12;
			map->tileArr[centerx+1][centery-2] = 6;
			map->tileArr[centerx+2][centery-2] = 9;
			map->tileArr[centerx-2][centery+1] = 9;
			map->tileArr[centerx+2][centery+1] = 3;
			map->tileArr[centerx - 2][centery - 1] = 12;
			map->tileArr[centerx+2][centery-1] = 6;



		}
		for (int i = 0; i < map->xsize; i++) {
			for (int j = 0; j < map->ysize; j++) {
				if (map->tileArr[i][j] == -1) {
					findPossibleTiles(&avaliableTiles, i, j);
					map->tileArr[i][j] = randomizeTileId(&avaliableTiles,0);// rand() % 16;
					avaliableTiles.clear();
				}
			}
		}
	}
}
void LevelGeneratorSystem::createWorld(Scene* scene, Model* tileFlat, Model* tileCross,Model* tileCorner,Model* tileStraight, Model* tileT,Model* tileEnd, Model* bb) {

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
			int tileId = map->tileArr[i][j];
			if (tileId==0) {
				e->addComponent<ModelComponent>(tileFlat);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}
			else if(tileId==1){
				e->addComponent<ModelComponent>(tileEnd);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(270.f), 0.f));
			}
			else if (tileId == 2) {
				e->addComponent<ModelComponent>(tileEnd);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}
			else if (tileId == 3) {
				e->addComponent<ModelComponent>(tileCorner);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}
			else if (tileId == 4) {
				e->addComponent<ModelComponent>(tileEnd);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(90.f), 0.f));
			}
			else if (tileId == 5) {
				e->addComponent<ModelComponent>(tileStraight);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)),glm::vec3(0.f,glm::radians(90.f),0.f));
			}
			else if (tileId == 6) {
				e->addComponent<ModelComponent>(tileCorner);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(90.f), 0.f));
			}
			else if (tileId == 7) {
				e->addComponent<ModelComponent>(tileT);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(90.f), 0.f));
			}
			else if (tileId == 8) {
				e->addComponent<ModelComponent>(tileEnd);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(180.f), 0.f));
			}
			else if (tileId == 9) {
				e->addComponent<ModelComponent>(tileCorner);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(270.f), 0.f));
			}
			else if (tileId == 10) {
				e->addComponent<ModelComponent>(tileStraight);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}
			else if(tileId==11){
				e->addComponent<ModelComponent>(tileT);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}
			else if (tileId == 12) {
				e->addComponent<ModelComponent>(tileCorner);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(180.f), 0.f));
			}
			else if (tileId == 13) {
				e->addComponent<ModelComponent>(tileT);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(270.f), 0.f));
			}
			else if (tileId == 14) {
				e->addComponent<ModelComponent>(tileT);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)), glm::vec3(0.f, glm::radians(180.f), 0.f));
			}
			else {
				e->addComponent<ModelComponent>(tileCross);
				e->addComponent<StaticMatrixComponent>(glm::vec3(tileSize * (i + 2), 0.f, tileSize * (j + 2)));
			}

			e->
			e->addComponent<BoundingBoxComponent>(bb);
			e->addComponent<CollidableComponent>();
			scene->addStaticEntity(e);
		}
	}
}

int LevelGeneratorSystem::randomizeTileId(std::vector<int>* tiles, int seed) {
	if (tiles->size() > 0) {
		return tiles->operator[](rand() % tiles->size());
	}
	else {
		return -1;
	}
}

void LevelGeneratorSystem::findPossibleTiles(std::vector<int>* mapPointer, int posx, int posy) {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();
		
		int tiles[4];
		tiles[0]=map->tileArr[posx][posy + 1];
		tiles[1]= map->tileArr[posx+1][posy];
		tiles[2] = map->tileArr[posx][posy-1];
		tiles[3] = map->tileArr[posx - 1][posy];

		int nrOfRules = 0;
		std::vector<int> temp;
		int match = 0;
		for (int i = 0; i < 4; i++) {
			if (tiles[i] != -1) {
				nrOfRules++;
			}
		}
		for (int i = 0; i < 16; i++) {
			if (tiles[0] != -1) {
				if (((tiles[0] % 8) / 4) == (i % 2)) {
					temp.emplace_back(i);
				}
			}
			if (tiles[1] != -1) {
				if ((tiles[1] / 8)==((i % 4) / 2)) {
					temp.emplace_back(i);
				}
			}
			if (tiles[2] != -1) {
				if (((i%8)/4)==(tiles[2]%2)) {
					temp.emplace_back(i);
				}
			}
			if (tiles[3] != -1) {
				if (((tiles[3]% 4) / 2) == (i/ 8)) {
					temp.emplace_back(i);
				}
			}
		}
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < temp.size(); j++) {
				if (i == temp[j]) {
					match++;
				}
			}
			if (match >= nrOfRules) {
				mapPointer->emplace_back(i);
				if (i == 0) {
					mapPointer->emplace_back(i);
				}
			}
			match = 0;
		}
	}
}

int LevelGeneratorSystem::createTileId(int posx, int posy) {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();

	}
	return 0;
}