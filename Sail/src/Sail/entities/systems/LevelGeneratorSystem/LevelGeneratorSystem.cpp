#include "pch.h"
#include "LevelGeneratorSystem.h"
#include "..//Sail/src/Sail/entities/ECS.h"
#include "..//Sail/src/Sail/entities/components/Components.h"
#include "..//..//..//graphics/Scene.h"
#include "..//..//components/MapComponent.h"
#include "..//..//Entity.h"
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

		srand(2);
		int seed =2;
		int centerx = (map->xsize)/ 2, centery = (map->ysize)/ 2;
		std::vector<int> avaliableTiles;

		//set edges
		{
			avaliableTiles.push_back(5);
			avaliableTiles.push_back(7);
			for (int i = 1; i < map->ysize - 1; i++) {
				map->tileArr[0][i] = randomizeTileId(&avaliableTiles);
			}
			avaliableTiles.clear();

			avaliableTiles.push_back(10);
			avaliableTiles.push_back(11);
			for (int i = 1; i < map->xsize - 1; i++) {
				map->tileArr[i][0] = randomizeTileId(&avaliableTiles);
			}
			avaliableTiles.clear();	
			avaliableTiles.push_back(5);
			avaliableTiles.push_back(13);
			for (int i = 1; i < map->ysize - 1; i++) {
				map->tileArr[map->xsize-1][i] = randomizeTileId(&avaliableTiles);
			}
			avaliableTiles.clear();
			avaliableTiles.push_back(10);
			avaliableTiles.push_back(14);
			for (int i = 1; i < map->xsize - 1; i++) {
				map->tileArr[i][map->ysize-1] = randomizeTileId(&avaliableTiles);
			}
			avaliableTiles.clear();
		}
		//set center room
		{
			//floor
			map->tileArr[centerx][centery] = 17;
			map->tileArr[centerx - 1][centery] = 0;
			map->tileArr[centerx - 1][centery - 1] = 0;
			map->tileArr[centerx][centery - 1] = 0;

			//walls
			map->tileArr[centerx - 2][centery - 2] = 3;
			map->tileArr[centerx - 2][centery - 1] = 12;
			map->tileArr[centerx - 2][centery] = 9;
			map->tileArr[centerx - 2][centery + 1] = 6;
			map->tileArr[centerx - 1][centery + 1] = 9;
			map->tileArr[centerx][centery + 1] = 3;
			map->tileArr[centerx + 1][centery + 1] = 12;
			map->tileArr[centerx + 1][centery] = 3;
			map->tileArr[centerx + 1][centery - 1] = 6;
			map->tileArr[centerx + 1][centery - 2] = 9;
			map->tileArr[centerx][centery - 2] = 6;
			map->tileArr[centerx - 1][centery - 2] = 12;

			//ends
			map->tileArr[centerx - 1][centery + 2] = 4;
			map->tileArr[centerx][centery + 2] = 4;
			map->tileArr[centerx + 2][centery] = 8;
			map->tileArr[centerx + 2][centery - 1] = 8;
			map->tileArr[centerx][centery - 3] = 1;
			map->tileArr[centerx - 1][centery - 3] = 1;
			map->tileArr[centerx - 3][centery - 1] = 2;
			map->tileArr[centerx - 3][centery] = 2;
		}
		//create ramp rooms
		{	
			//north room
			map->tileArr[centerx-2][map->ysize - 1] = 14;
			map->tileArr[centerx-1][map->ysize - 1] = 10;
			map->tileArr[centerx][map->ysize - 1] = 10;
			map->tileArr[centerx+1][map->ysize - 1] = 14;

			map->tileArr[centerx][map->ysize-3] = 0;
			map->tileArr[centerx][map->ysize-2] = 0;
			map->tileArr[centerx-1][map->ysize - 3] = 0;
			map->tileArr[centerx-1][map->ysize - 2] = 0;
			
			map->tileArr[centerx-2][map->ysize - 4] = 3;
			map->tileArr[centerx-1][map->ysize - 4] = 12;
			map->tileArr[centerx][map->ysize - 4] = 6;
			map->tileArr[centerx+1][map->ysize - 4] = 9;

			//south room
			map->tileArr[centerx-2][0] = 11;
			map->tileArr[centerx-1][0] = 10;
			map->tileArr[centerx][0] = 10;
			map->tileArr[centerx+1][0] = 11;

			map->tileArr[centerx][2] = 0;
			map->tileArr[centerx][1] = 0;
			map->tileArr[centerx - 1][2] = 0;
			map->tileArr[centerx - 1][1] = 0;
			
			map->tileArr[centerx - 2][3] = 6;
			map->tileArr[centerx - 1][3] = 9;
			map->tileArr[centerx][3] = 3;
			map->tileArr[centerx + 1][3] = 12;

			//east room
			map->tileArr[map->xsize-1][centery- 2] = 13;
			map->tileArr[map->xsize - 1][centery - 1] = 5;
			map->tileArr[map->xsize - 1][centery] = 5;
			map->tileArr[map->xsize - 1][centery+1] = 13;

			map->tileArr[map->xsize - 3][centery] = 0;
			map->tileArr[map->xsize - 2][centery] = 0;
			map->tileArr[map->xsize - 3][centery-1] = 0;
			map->tileArr[map->xsize - 2][centery -1] = 0;

			map->tileArr[map->xsize - 4][centery - 2] = 3;
			map->tileArr[map->xsize - 4][centery - 1] = 12;
			map->tileArr[map->xsize - 4][centery ] = 9;
			map->tileArr[map->xsize - 4][centery +1] = 6;

			//west room
			map->tileArr[0][centery - 2] = 7;
			map->tileArr[0][centery - 1] = 5;
			map->tileArr[0][centery] = 5;
			map->tileArr[0][centery + 1] = 7;

			map->tileArr[2][centery] = 0;
			map->tileArr[1][centery] = 0;
			map->tileArr[2][centery - 1] = 0;
			map->tileArr[1][centery - 1] = 0;

			map->tileArr[3][centery - 2] = 9;
			map->tileArr[3][centery - 1] = 6;
			map->tileArr[3][centery] = 3;
			map->tileArr[3][centery + 1] = 12;
		}
		//create "corridors"
		{
			for (int i = 0; i < map->ysize; i++) {
				if (map->tileArr[centerx][i] == -1) {
					map->tileArr[centerx][i] = 0;
					findPossibleTiles(&avaliableTiles, centerx-1,i);
					map->tileArr[centerx-1][i] = randomizeTileId(&avaliableTiles);
					avaliableTiles.clear();
					findPossibleTiles(&avaliableTiles, centerx , i);
					map->tileArr[centerx][i] = randomizeTileId(&avaliableTiles);
					avaliableTiles.clear();
					if (map->tileArr[centerx][i] == 0 || map->tileArr[centerx - 1][i] == 0) {
						map->tileArr[centerx][i] = -1;
						map->tileArr[centerx - 1][i] = -1;
						i--;
					}
				}
			}
			for (int i = 0; i < map->xsize; i++) {
				if (map->tileArr[i][centery] == -1) {
					map->tileArr[i][centery] = 0;
					findPossibleTiles(&avaliableTiles, i, centery-1);
					map->tileArr[i][centery-1] = randomizeTileId(&avaliableTiles);
					avaliableTiles.clear();
					findPossibleTiles(&avaliableTiles, i, centery);
					map->tileArr[i][centery] = randomizeTileId(&avaliableTiles);
					avaliableTiles.clear();
					if (map->tileArr[i][centery] == 0 || map->tileArr[i][centery-1] == 0) {
						map->tileArr[i][centery] = -1;
						map->tileArr[i][centery - 1] = -1;
						i--;
					}

				}
			}
		}

		//generateCorridors();

		//set all other tiles
		for (int i = 0; i < map->xsize; i++) {
			for (int j = 0; j < map->ysize; j++) {
				if (map->tileArr[i][j] == -1) {
					findPossibleTiles(&avaliableTiles, i, j);
					map->tileArr[i][j] = randomizeTileId(&avaliableTiles);// rand() % 16;
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
	int tileSize = 7;
	int worldWidth = map->xsize;
	int worldDepth = map->ysize;

	for (int i = 0; i < worldWidth; i++) {
		for (int j = 0; j < worldDepth; j++) {
			auto e = ECS::Instance()->createEntity("");
			int tileId = map->tileArr[i][j];
			if (tileId<16 && tileId>-1) {
				if (tileId == 0) {
					e->addComponent<ModelComponent>(tileFlat);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20));
				}
				else if (tileId == 1) {
					e->addComponent<ModelComponent>(tileEnd);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20), glm::vec3(0.f, glm::radians(270.f), 0.f));
				}
				else if (tileId == 2) {
					e->addComponent<ModelComponent>(tileEnd);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20));
				}
				else if (tileId == 3) {
					e->addComponent<ModelComponent>(tileCorner);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20));
				}
				else if (tileId == 4) {
					e->addComponent<ModelComponent>(tileEnd);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20), glm::vec3(0.f, glm::radians(90.f), 0.f));
				}
				else if (tileId == 5) {
					e->addComponent<ModelComponent>(tileStraight);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20), glm::vec3(0.f, glm::radians(90.f), 0.f));
				}
				else if (tileId == 6) {
					e->addComponent<ModelComponent>(tileCorner);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20), glm::vec3(0.f, glm::radians(90.f), 0.f));
				}
				else if (tileId == 7) {
					e->addComponent<ModelComponent>(tileT);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20), glm::vec3(0.f, glm::radians(90.f), 0.f));
				}
				else if (tileId == 8) {
					e->addComponent<ModelComponent>(tileEnd);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20), glm::vec3(0.f, glm::radians(180.f), 0.f));
				}
				else if (tileId == 9) {
					e->addComponent<ModelComponent>(tileCorner);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20), glm::vec3(0.f, glm::radians(270.f), 0.f));
				}
				else if (tileId == 10) {
					e->addComponent<ModelComponent>(tileStraight);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20));
				}
				else if (tileId == 11) {
					e->addComponent<ModelComponent>(tileT);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20));
				}
				else if (tileId == 12) {
					e->addComponent<ModelComponent>(tileCorner);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20), glm::vec3(0.f, glm::radians(180.f), 0.f));
				}
				else if (tileId == 13) {
					e->addComponent<ModelComponent>(tileT);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20), glm::vec3(0.f, glm::radians(270.f), 0.f));
				}
				else if (tileId == 14) {
					e->addComponent<ModelComponent>(tileT);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20), glm::vec3(0.f, glm::radians(180.f), 0.f));
				}
				else if (tileId == 15) {
					e->addComponent<ModelComponent>(tileCross);
					e->addComponent<TransformComponent>(glm::vec3(tileSize * i + 20, 0.f, tileSize * j + 20));
				}
				e->getComponent<TransformComponent>()->setScale(glm::vec3(.7f, 4.0f, .7f));
				e->addComponent<BoundingBoxComponent>(bb);
				e->addComponent<CollidableComponent>();
				scene->addEntity(e);
			}
		}
	}
}

int LevelGeneratorSystem::randomizeTileId(std::vector<int>* tiles) {
	if (tiles->size() > 0) {
		//srand(seed);
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
				//less t and cross
				if (i !=7 &&i!=11&&i!=13&&i!=14&&i!=15) {
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
				}
				//more flat
				if (i == 0) {
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
				}
				//more straight
				if(i==5||i==10){
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
					mapPointer->emplace_back(i);
				}
			}
			match = 0;
		}
	}
}

void LevelGeneratorSystem::generateCorridors() {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();
		int pos = 0;
		bool ns = true;
		int corridors = 0;
		float size = map->xsize * map->ysize;
		while ((corridors / size) < 0.1f) {
			pos = rand()%map->xsize;
			for (int i = 0; i < map->xsize; i++) {
				if (ns) {
					if (map->tileArr[pos][i] == -1) {
						map->tileArr[pos][i] = 0;
						corridors++;
					}
				}
				else {
					if (map->tileArr[i][pos] == -1) {
						map->tileArr[i][pos] =0;
						corridors++;
					}
				}
			}
			ns = !ns;
		}
	}
}

void LevelGeneratorSystem::splitChunk() {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();
		//map->chunk
	}
}