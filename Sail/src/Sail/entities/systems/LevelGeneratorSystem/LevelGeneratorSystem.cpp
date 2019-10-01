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
		srand(map->seed);
		std::vector<int> avaliableTiles;

		//create floor to split for map generation
		rect floor;
		floor.posx = 0;
		floor.posy = 0;
		floor.sizex = map->xsize;
		floor.sizey = map->ysize;
		map->chunks.emplace(floor);

		//create blocks and hallways
		splitChunk();

		//add hallways to type-layer
		while (!map->hallways.empty()) {
			rect tile;
			tile = map->hallways.front();
			map->hallways.pop();
			for (int i = 0; i < tile.sizex; i++) {
				for (int j = 0; j < tile.sizey; j++) {
					map->tileArr[tile.posx + i][tile.posy + j][1] = 0;
				}
			}
		}

		//create rooms from blocks
		splitBlock();

		//add rooms with individual type to type-layer
		int roomCounter = 1;
		while (!map->rooms.empty()) {
			rect tile;
			tile = map->rooms.front();
			map->rooms.pop();
			for (int i = 0; i < tile.sizex; i++) {
				for (int j = 0; j < tile.sizey; j++) {
					map->tileArr[tile.posx + i][tile.posy + j][1] = roomCounter;
					map->matched.emplace(tile);
				}
			}
			roomCounter++;
		}
		matchRoom();
	}
}

void LevelGeneratorSystem::createWorld(Scene* scene, Model* tileFlat, Model* tileCross,Model* tileCorner,Model* tileStraight, Model* tileT,Model* tileEnd, Model* bb) {

	MapComponent* map;
	for (auto& f : entities) {
		map = f->getComponent<MapComponent>();
	}
	int tileSize = 10;
	int worldWidth = map->xsize;
	int worldDepth = map->ysize;

	for (int i = 0; i < worldWidth; i++) {
		for (int j = 0; j < worldDepth; j++) {
			auto e = ECS::Instance()->createEntity("");
			int tileId = map->tileArr[i][j][0];
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
				float height = 1.0f;
				if (map->tileArr[i][j][1] != 0) {
					height = 3.0f;
				}
				e->getComponent<TransformComponent>()->setScale(glm::vec3(1.f, height, 1.f));
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
		tiles[0]=map->tileArr[posx][posy + 1][0];
		tiles[1]= map->tileArr[posx+1][posy][0];
		tiles[2] = map->tileArr[posx][posy-1][0];
		tiles[3] = map->tileArr[posx - 1][posy][0];

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

void LevelGeneratorSystem::splitChunk() {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();
		bool ns = true;
		float hallwayArea = 0.f;
		while (hallwayArea / map->totalArea < map->hallwayThreshold && !map->chunks.empty()) {
			rect rekt,a,b,hall;
			rekt = map->chunks.front();
			map->chunks.pop();
			if (ns) {
				if (rekt.sizex > map->minSplitSize) {
					int newSize = rand() % (rekt.sizex - map->minSplitSize) + map->minSplitSize/2;
					a.posx = rekt.posx;
					a.posy = rekt.posy;
					a.sizex = newSize;
					a.sizey = rekt.sizey;
					map->chunks.emplace(a);
					b.posy = rekt.posy;
					b.posx = rekt.posx + newSize + 1;
					b.sizex = rekt.sizex - 1 - newSize;
					b.sizey = rekt.sizey;
					map->chunks.emplace(b);
					hall.posx = rekt.posx + newSize;
					hall.posy = rekt.posy;
					hall.sizex = 1;
					hall.sizey = rekt.sizey;
					map->hallways.emplace(hall);
					hallwayArea += hall.sizex * hall.sizey;
				}
				else if(rekt.sizey <= map->minSplitSize){
					map->blocks.emplace(rekt);
					ns = !ns;
				}
				else {
					map->chunks.emplace(rekt);
				}
				ns = !ns;
			}
			else {
				if (rekt.sizey > map->minSplitSize) {
					int newSize = rand() % (rekt.sizey - map->minSplitSize) + map->minSplitSize/2;
					a.posx = rekt.posx;
					a.posy = rekt.posy;
					a.sizex = rekt.sizex;
					a.sizey = newSize;
					map->chunks.emplace(a);
					b.posy = rekt.posy + newSize + 1;
					b.posx = rekt.posx;
					b.sizex = rekt.sizex;
					b.sizey = rekt.sizey - 1 - newSize;
					map->chunks.emplace(b);
					hall.posx = rekt.posx ;
					hall.posy = rekt.posy + newSize;
					hall.sizex = rekt.sizex;
					hall.sizey =1;
					map->hallways.emplace(hall);
					hallwayArea += hall.sizex * hall.sizey;
				}
				else if (rekt.sizex <= map->minSplitSize) {
					map->blocks.emplace(rekt);
					ns = !ns;
				}
				else {
					map->chunks.emplace(rekt);
				}
				ns = !ns;

			}
		}
		while (!map->chunks.empty()) {
			rect rekt = map->chunks.front();
			map->chunks.pop();
			map->blocks.emplace(rekt);
		}
	}
}

void LevelGeneratorSystem::splitBlock() {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();

		while (!map->blocks.empty()) {
			rect rekt, a, b;
			rekt = map->blocks.front();
			map->blocks.pop();
			if (rand() % 100 > map->roomSplitStop) {
				if (rekt.sizex <= map->minRoomSize && rekt.sizey <= map->minRoomSize) {
					map->rooms.emplace(rekt);
				}
				else if ((rekt.sizex > rekt.sizey ||rekt.ns==2)&&rekt.ns<3) {
					if (rekt.sizex > 3) {
						int newSize = rand() % (rekt.sizex - 2) + 1;
						a.posy = rekt.posy;
						a.posx = rekt.posx;
						a.sizex = newSize;
						a.sizey = rekt.sizey;
						b.posy = rekt.posy;
						b.posx = rekt.posx + newSize;
						b.sizex = rekt.sizex - newSize;
						b.sizey = rekt.sizey;
						if (checkBorder(a) && checkBorder(b)) {
							map->blocks.emplace(a);
							map->blocks.emplace(b);
						}
						else {
							rekt.ns += 1;
							map->blocks.emplace(rekt);
						}
					}
					else if (rekt.sizex == 3) {
						int newSize = 1+rand()%2;
						a.posy = rekt.posy;
						a.posx = rekt.posx;
						a.sizex = newSize;
						a.sizey = rekt.sizey;
						b.posy = rekt.posy;
						b.posx = rekt.posx + newSize;
						b.sizex = rekt.sizex - newSize;
						b.sizey = rekt.sizey;
						if (checkBorder(a) && checkBorder(b)) {
							map->blocks.emplace(a);
							map->blocks.emplace(b);
						}
						else {
							rekt.ns += 1;
							map->blocks.emplace(rekt);
						}
					}
					else if (rekt.sizex == 2) {
						int newSize = 1;
						a.posy = rekt.posy;
						a.posx = rekt.posx;
						a.sizex = newSize;
						a.sizey = rekt.sizey;
						b.posy = rekt.posy;
						b.posx = rekt.posx + newSize;
						b.sizex = rekt.sizex - newSize;
						b.sizey = rekt.sizey;
						if (checkBorder(a) && checkBorder(b)) {
							map->blocks.emplace(a);
							map->blocks.emplace(b);
						}
						else {
							rekt.ns += 1;
							map->blocks.emplace(rekt);
						}
					}
				}
				else if(rekt.ns==0||rekt.ns==1 ){
					if (rekt.sizey > 3) {
						int newSize = rand() % (rekt.sizey - 2)+1;
						a.posy = rekt.posy;
						a.posx = rekt.posx;
						a.sizey = newSize;
						a.sizex = rekt.sizex;
						b.posy = rekt.posy + newSize;
						b.posx = rekt.posx;
						b.sizex = rekt.sizex;
						b.sizey = rekt.sizey - newSize;
						if (checkBorder(a) && checkBorder(b)) {
							map->blocks.emplace(a);
							map->blocks.emplace(b);
						}
						else {
							rekt.ns += 2;
							map->blocks.emplace(rekt);
						}
					}
					else if (rekt.sizey == 3) {
						int newSize = 1+rand()%2;
						a.posy = rekt.posy;
						a.posx = rekt.posx;
						a.sizey = newSize;
						a.sizex = rekt.sizex;
						b.posy = rekt.posy + newSize;
						b.posx = rekt.posx;
						b.sizex = rekt.sizex;
						b.sizey = rekt.sizey - newSize;
						if (checkBorder(a) && checkBorder(b)) {
							map->blocks.emplace(a);
							map->blocks.emplace(b);
						}
						else {
							rekt.ns += 2;
							map->blocks.emplace(rekt);
						}
					}
					else if (rekt.sizey == 2) {
						int newSize = 1;
						a.posy = rekt.posy;
						a.posx = rekt.posx;
						a.sizey = newSize;
						a.sizex = rekt.sizex;
						b.posy = rekt.posy + newSize;
						b.posx = rekt.posx;
						b.sizex = rekt.sizex;
						b.sizey = rekt.sizey - newSize;
						if (checkBorder(a) && checkBorder(b)) {
							map->blocks.emplace(a);
							map->blocks.emplace(b);
						}
						else {
							rekt.ns += 2;
							map->blocks.emplace(rekt);
						}
					}
				}
				else {
					map->rooms.emplace(rekt);
				}
			}
			else {
				map->rooms.emplace(rekt);
			}
		}
	}
}

void LevelGeneratorSystem::matchRoom() {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();
		int tiles[4];
		for (int i = 0; i < map->xsize; i++) {
			for (int j = 0; j < map->ysize; j++) {
				//check typeID around current position to find tileID 
				if (i - 1 < 0) {
					tiles[0] = -1;
				}
				else {
					tiles[0] = map->tileArr[i - 1][j][1];
				}
				if (j + 1 > map->ysize) {
					tiles[1] = -1;
				}
				else {
					tiles[1] = map->tileArr[i][j + 1][1];
				}
				if (i + 1 > map->xsize) {
					tiles[2] = -1;
				}
				else {
					tiles[2] = map->tileArr[i + 1][j][1];
				}
				if (j - 1 < 0) {
					tiles[3] = -1;
				}
				else {
					tiles[3] = map->tileArr[i][j - 1][1];
				}

				//match and add value for walls
				if (tiles[0] != map->tileArr[i][j][1]) {
					map->tileArr[i][j][0] += 8;
				}
				if (tiles[1] != map->tileArr[i][j][1]) {
					map->tileArr[i][j][0] += 1;
				}
				if (tiles[2] != map->tileArr[i][j][1]) {
					map->tileArr[i][j][0] += 2;
				}
				if (tiles[3] != map->tileArr[i][j][1]) {
					map->tileArr[i][j][0] += 4;
				}
				if (map->tileArr[i][j][1] == -1) {
					map->tileArr[i][j][0] = -1;
				}
			}
		}
	
	}
}

bool LevelGeneratorSystem::checkBorder(rect rekt) {
	bool top=false,bottom=false,left=false,right=false;
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();
		for (int i = 0; i < rekt.sizex; i++) {
			if (rekt.posx + i >= 0 && rekt.posx + i < map->xsize) {
				if (rekt.posy > 0) {
					if (map->tileArr[rekt.posx + i][rekt.posy - 1][1] == 0) {
						bottom = true;
					}
				}
				if (rekt.posy + rekt.sizey + 1 < map->ysize) {
					if (map->tileArr[rekt.posx + i][rekt.posy + rekt.sizey ][1] == 0) {
						top = true;
					}
				}
			}
		}

		for (int i = 0; i < rekt.sizey; i++) {
			if (rekt.posy + i >= 0 && rekt.posy + i < map->ysize) {
				if (rekt.posx > 0) {
					if (map->tileArr[rekt.posx - 1][rekt.posy + i][1] == 0) {
						left = true;
					}
				}
				if (rekt.posx + rekt.sizex + 1 < map->xsize) {
					if (map->tileArr[rekt.posx + rekt.sizex][rekt.posy + i][1] == 0) {
						right = true;
					}
				}
			}
		}
	}
	return (top||bottom||left||right);
}