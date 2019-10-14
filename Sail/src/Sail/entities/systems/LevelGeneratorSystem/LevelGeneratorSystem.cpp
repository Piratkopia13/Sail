#include "pch.h"
#include "LevelGeneratorSystem.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/components/MapComponent.h"
//#include "..//..//Entity.h"
LevelGeneratorSystem::LevelGeneratorSystem():BaseComponentSystem() {
	registerComponent<MapComponent>(true,true,true);
	registerComponent<ModelComponent>(false, false, true);
	registerComponent<TransformComponent>(false, true, true);
	registerComponent<BoundingBoxComponent>(false, false, true);
	registerComponent<CollidableComponent>(false, false, true);
}

LevelGeneratorSystem::~LevelGeneratorSystem() {
}

void LevelGeneratorSystem::update(float dt) {
}

//generates all necessary data for the world
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
				}
			}
			map->matched.emplace(tile);
			roomCounter++;
		}

		//adds doors to the layout
		addDoors();

		//creates tilemap for the level
		matchRoom();
	}
}

void LevelGeneratorSystem::createWorld(const std::vector<Model*>& tileModels, Model* bb) {
	
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();

		float tileSize = 5.0f; //how big a tile should be. Model has size 10, anything smaller scales everything down.
		int worldWidth = map->xsize;
		int worldDepth = map->ysize;
		int tileOffset = 20; //offset from origo in game

		//traverse all positions to find which tile should be there
		for (int i = 0; i < worldWidth; i++) {
			for (int j = 0; j < worldDepth; j++) {
				int tileId = map->tileArr[i][j][0];
				int doors = map->tileArr[i][j][2];
				if (tileId<16 && tileId>-1) {
					/*
					Adding tile type:
					x         x




					x         x
					*/
					if (tileId == 0) {
						addTile(Direction::NONE, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x---------x




					x         x
					*/
					else if (tileId == 1) {
						addTile(Direction::UP, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x         x
					          |
					          |
					          |
					          |
					x         x
					*/
					else if (tileId == 2) {
						addTile(Direction::RIGHT, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x---------x
					          |
					          |
					          |
					          |
					x         x
					*/
					else if (tileId == 3) {
						addTile(Direction::UP, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::RIGHT, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x         x




					x---------x
					*/
					else if (tileId == 4) {
						addTile(Direction::DOWN, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x---------x




					x---------x
					*/
					else if (tileId == 5) {
						addTile(Direction::UP, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::DOWN, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x         x
							  |
							  |
							  |
							  |
					x---------x
					*/
					else if (tileId == 6) {
						addTile(Direction::RIGHT, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::DOWN, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x---------x
							  |
							  |
							  |
							  |
					x---------x
					*/
					else if (tileId == 7) {
						addTile(Direction::UP, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::DOWN, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::RIGHT, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x         x
					|
					|
					|
					|
					x         x
					*/
					else if (tileId == 8) {
						addTile(Direction::LEFT, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x---------x
					|
					|
					|
					|
					x         x

					*/
					else if (tileId == 9) {
						addTile(Direction::UP, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::LEFT, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x         x
					|		  |
					|		  |
					|		  |
					|		  |
					x         x

					*/
					else if (tileId == 10) {
						addTile(Direction::RIGHT, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::LEFT, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x---------x
					|		  |
					|		  |
					|		  |
					|		  |
					x         x

					*/
					else if (tileId == 11) {						
						addTile(Direction::RIGHT, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::LEFT, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::UP, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x         x
					|
					|
					|
					|
					x---------x

					*/
					else if (tileId == 12) {
						addTile(Direction::DOWN, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::LEFT, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x---------x
					|
					|
					|
					|
					x---------x

					*/
					else if (tileId == 13) {
						addTile(Direction::UP, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::DOWN, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::LEFT, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x         x
					|		  |
					|		  |
					|		  |
					|		  |
					x---------x

					*/
					else if (tileId == 14) {
						addTile(Direction::RIGHT, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::DOWN, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::LEFT, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
					/*
					Adding tile type:
					x---------x
					|		  |
					|		  |
					|		  |
					|		  |
					x---------x

					*/
					else if (tileId == 15) {
						addTile(Direction::UP, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::RIGHT, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::DOWN, doors, tileModels, tileSize, tileOffset, i, j, bb);
						addTile(Direction::LEFT, doors, tileModels, tileSize, tileOffset, i, j, bb);
					}
				}
			}
		}
	}
}

//chooses a random tile from all possible tiles that fit
int LevelGeneratorSystem::randomizeTileId(std::vector<int>* tiles) {
	if (tiles->size() > 0) {
		//srand(seed);
		return tiles->operator[](rand() % tiles->size());
	}
	else {
		return -1;
	}
}

//finds a matching tile in a labyrinth implementation
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

//splits chunks to make hallways and blocks
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

//splits blocks to make rooms
void LevelGeneratorSystem::splitBlock() {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();

		while (!map->blocks.empty()) {
			bool isSplit = false,ns=true;
			rect rekt = map->blocks.front();
			if (rand() % 100 > map->roomSplitStop||rekt.sizex*rekt.sizey>map->roomMaxSize) {
				if (rekt.sizex > rekt.sizey) {
					ns = true;
				}
				else {
					ns = false;
				}
				isSplit = splitDirection(ns);
				if (isSplit) {
					map->blocks.pop();
				}
			}
			else {
				map->rooms.emplace(rekt);
				map->blocks.pop();
			}
		}
	}
}

//traverses every tile and matches it to the "areas" next to it, to see if there should be a wall.
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
				if (j + 1 >= map->ysize) {
					tiles[1] = -1;
				}
				else {
					tiles[1] = map->tileArr[i][j + 1][1];
				}
				if (i + 1 >= map->xsize) {
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

//checks the borders of a rect to see if it borders a corridor.
//Returns an int which holds all directions in which to find a corridor
int LevelGeneratorSystem::checkBorder(rect rekt) {
	bool top = false; 
	bool bottom = false;
	bool left = false;
	bool right = false;
	int corridor = 0;
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
	if (bottom) {
		corridor += 4;
	}
	if (top) {
		corridor += 1;
	}
	if (right) {
		corridor += 2;
	}
	if (left) {
		corridor += 8;
	}
	return corridor;
}

//Splits a block in a direction while making sure both new blocks are next to a corridor.
//If they're not, it tries to split the block the other way. If that also fails, the block is added to the rooms.
bool LevelGeneratorSystem::splitDirection(bool ns) {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();
		rect rekt, a, b;
		rekt = map->blocks.front();
		if (ns) {
			if (rekt.sizex >= map->minRoomSize * 2) {
				int newSize = 0;
				if (rekt.sizex == 2 * map->minRoomSize) {
					newSize = map->minRoomSize;
				}
				else if (rekt.sizex == 2 * map->minRoomSize + 1) {
					newSize = rand() % 2 + map->minRoomSize;
				}
				else {
					newSize = rand() % (rekt.sizex - 2 * map->minRoomSize) + map->minRoomSize;
				}
				a.posx = rekt.posx;
				a.posy = rekt.posy;
				a.sizex = newSize;
				a.sizey = rekt.sizey;
				b.posx = rekt.posx + newSize;
				b.posy = rekt.posy;
				b.sizex = rekt.sizex - newSize;
				b.sizey = rekt.sizey;
				if (checkBorder(a)>0 && checkBorder(b)>0) {
					map->blocks.emplace(a);
					map->blocks.emplace(b);
					return true;
				}
				else {
					if (rekt.sizey >= map->minRoomSize * 2) {
						newSize = 0;
						if (rekt.sizey == 2 * map->minRoomSize) {
							newSize = map->minRoomSize;
						}
						else if (rekt.sizey == 2 * map->minRoomSize + 1) {
							newSize = rand() % 2 + map->minRoomSize;
						}
						else {
							newSize = rand() % (rekt.sizey - 2 * map->minRoomSize) + map->minRoomSize;
						}
						a.posx = rekt.posx;
						a.posy = rekt.posy;
						a.sizex = rekt.sizex;
						a.sizey = newSize;
						b.posx = rekt.posx;
						b.posy = rekt.posy+newSize;
						b.sizex = rekt.sizex;
						b.sizey = rekt.sizey - newSize;
						if (checkBorder(a)>0 && checkBorder(b)>0) {
							map->blocks.emplace(a);
							map->blocks.emplace(b);
							return true;
						}
						else {
							map->rooms.emplace(rekt);
							return true;
						}
					}
					else {
						map->rooms.emplace(rekt);
						return true;
					}
				}

			}
			else {
				if (rekt.sizey >= map->minRoomSize * 2) {
					int newSize = 0;
					if (rekt.sizey == 2 * map->minRoomSize) {
						newSize = map->minRoomSize;
					}
					else if (rekt.sizey == 2 * map->minRoomSize + 1) {
						newSize = rand() % 2 + map->minRoomSize;
					}
					else {
						newSize = rand() % (rekt.sizey - 2 * map->minRoomSize) + map->minRoomSize;
					}
					a.posx = rekt.posx;
					a.posy = rekt.posy;
					a.sizex = rekt.sizex;
					a.sizey = newSize;
					b.posx = rekt.posx;
					b.posy = rekt.posy + newSize;
					b.sizex = rekt.sizex;
					b.sizey = rekt.sizey - newSize;
					if (checkBorder(a)>0 && checkBorder(b)>0) {
						map->blocks.emplace(a);
						map->blocks.emplace(b);
						return true;
					}
					else {
						map->rooms.emplace(rekt);
						return true;
					}
				}
				else {
					map->rooms.emplace(rekt);
					return true;
				}

			}
		}
		if (!ns) {
			if (rekt.sizey >= map->minRoomSize * 2) {
				int newSize = 0;
				if (rekt.sizey == 2*map->minRoomSize) {
					newSize = map->minRoomSize;
				}
				else if (rekt.sizey == 2*map->minRoomSize+1) {
					newSize = rand() % 2 + map->minRoomSize;
				}
				else {
					newSize = rand() % (rekt.sizey - 2 * map->minRoomSize) + map->minRoomSize;
				}
				a.posx = rekt.posx;
				a.posy = rekt.posy;
				a.sizex = rekt.sizex;
				a.sizey = newSize;
				b.posx = rekt.posx;
				b.posy = rekt.posy + newSize;
				b.sizex = rekt.sizex ;
				b.sizey = rekt.sizey - newSize;
				if (checkBorder(a)>0 && checkBorder(b)>0) {
					map->blocks.emplace(a);
					map->blocks.emplace(b);
					return true;
				}
				else {
					if (rekt.sizex >= map->minRoomSize * 2) {
						newSize = 0;
						if (rekt.sizex == 2 * map->minRoomSize) {
							newSize = map->minRoomSize;
						}
						else if (rekt.sizex == 2 * map->minRoomSize + 1) {
							newSize = rand() % 2 + map->minRoomSize;
						}
						else {
							newSize = rand() % (rekt.sizex - 2 * map->minRoomSize) + map->minRoomSize;
						}
						a.posx = rekt.posx;
						a.posy = rekt.posy;
						a.sizex = newSize;
						a.sizey = rekt.sizey;
						b.posx = rekt.posx+newSize;
						b.posy = rekt.posy;
						b.sizex = rekt.sizex-newSize;
						b.sizey = rekt.sizey;
						if (checkBorder(a)>0 && checkBorder(b)>0) {
							map->blocks.emplace(a);
							map->blocks.emplace(b);
							return true;
						}
						else {
							map->rooms.emplace(rekt);
							return true;
						}
					}
					else {
						map->rooms.emplace(rekt);
						return true;
					}
				}

			}
			else {
				if (rekt.sizex >= map->minRoomSize * 2) {
					int newSize = 0;
					if (rekt.sizex == 2 * map->minRoomSize) {
						newSize = map->minRoomSize;
					}
					else if (rekt.sizex == 2 * map->minRoomSize + 1) {
						newSize = rand() % 2 + map->minRoomSize;
					}
					else {
						newSize = rand() % (rekt.sizex - 2 * map->minRoomSize) + map->minRoomSize;
					}
					a.posx = rekt.posx;
					a.posy = rekt.posy;
					a.sizex = newSize;
					a.sizey = rekt.sizey;
					b.posx = rekt.posx+newSize;
					b.posy = rekt.posy;
					b.sizex = rekt.sizex-newSize;
					b.sizey = rekt.sizey;
					if (checkBorder(a)>0 && checkBorder(b)>0) {
						map->blocks.emplace(a);
						map->blocks.emplace(b);
						return true;
					}
					else {
						map->rooms.emplace(rekt);
						return true;
					}
				}
				else {
					map->rooms.emplace(rekt);
					return true;
				}

			}

		}
	}
	return false;
}

void LevelGeneratorSystem::addDoors() {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();
		
		size_t maxSize = map->matched.size();

		//add one door to each room, which leads to a corridor
		for (int c = 0; c < maxSize; c++) {
			rect currentTile = map->matched.front();
			std::vector<rect> possibleDoors;
			if (currentTile.doors == 0) {
				for (int i = 0; i < currentTile.sizex; i++) {
					for (int j = 0; j < currentTile.sizey; j++) {
						rect doorTile;
						doorTile.posx = currentTile.posx + i;
						doorTile.posy = currentTile.posy + j;
						doorTile.sizex = 1;
						doorTile.sizey = 1;
						doorTile.sizex = checkBorder(doorTile);
						if (doorTile.sizex > 0) {
							if (hasDoor(Direction::LEFT, doorTile.sizex)) {
								doorTile.sizey = Direction::LEFT;
								possibleDoors.emplace_back(doorTile);
							}
							if (hasDoor(Direction::DOWN, doorTile.sizex)) {
								doorTile.sizey = Direction::DOWN;
								possibleDoors.emplace_back(doorTile);
							}
							if (hasDoor(Direction::RIGHT, doorTile.sizex)) {
								doorTile.sizey = Direction::RIGHT;
								possibleDoors.emplace_back(doorTile);
							}
							if (hasDoor(Direction::UP, doorTile.sizex)) {
								doorTile.sizey = Direction::UP;
								possibleDoors.emplace_back(doorTile);
							}
						}
					}
				}
				// Pick one of the possible doors at random
				if (possibleDoors.size() > 0) {
					rect door = possibleDoors[rand() % possibleDoors.size()];
					possibleDoors.clear();
					if (door.sizey == Direction::UP) {
						map->tileArr[door.posx][door.posy][2] += Direction::UP;
						map->tileArr[door.posx][door.posy + 1][2] += Direction::DOWN;
						currentTile.doors += Direction::UP;
					}
					else if (door.sizey == Direction::RIGHT) {
						map->tileArr[door.posx][door.posy][2] += Direction::RIGHT;
						map->tileArr[door.posx+1][door.posy][2] += Direction::LEFT;
						currentTile.doors += Direction::RIGHT;
					}
					else if (door.sizey == Direction::DOWN) {
						map->tileArr[door.posx][door.posy][2] += Direction::DOWN;
						map->tileArr[door.posx][door.posy-1][2] += Direction::UP;
						currentTile.doors += Direction::DOWN;
					}
					else if (door.sizey == Direction::LEFT) {
						map->tileArr[door.posx][door.posy][2] += Direction::LEFT;
						map->tileArr[door.posx - 1][door.posy][2] += Direction::RIGHT;
						currentTile.doors += Direction::LEFT;
					}
				}
			}
			map->matched.pop();
			map->matched.emplace(currentTile);
		}

		// Adds a second door to each room
		for (int c = 0; c < maxSize; c++) {
			std::vector<rect> possibleDoors;
			rect currentTile = map->matched.front();
			int doorCounter = 0;
			bool up = false, right = false, down = false, left = false;
			// Check each room which walls have doors, and how many
			for (int i = 0; i < currentTile.sizex; i++) {
				for (int j = 0; j < currentTile.sizey; j++) {
					int doors = map->tileArr[currentTile.posx + i][currentTile.posy + j][2];
					if (hasDoor(Direction::UP, doors)) {
						up = true;
						doorCounter++;
					}
					if(hasDoor(Direction::RIGHT, doors)){
						right = true;
						doorCounter++;
					}
					if (hasDoor(Direction::DOWN, doors)) {
						down = true;
						doorCounter++;
					}
					if (hasDoor(Direction::LEFT, doors)) {
						left = true;
						doorCounter++;
					}
				}
			}
			// If the room does not have atleast 2 doors, we add one more on a wall that does not have a door already
			if (doorCounter < 2) {
				if (currentTile.posx - 1 >= 0 && !left) {
					for (int i = 0; i < currentTile.sizey; i++) {
						rect doorTile;
						doorTile.posx = currentTile.posx;
						doorTile.posy = currentTile.posy + i;
						doorTile.sizex = 1;
						doorTile.sizey = 8;
						possibleDoors.emplace_back(doorTile);
					}
				}
				if (currentTile.posx + currentTile.sizex + 1 < map->xsize && !right) {
					for (int i = 0; i < currentTile.sizey; i++) {
						rect doorTile;
						doorTile.posx = currentTile.posx + currentTile.sizex - 1;
						doorTile.posy = currentTile.posy + i;
						doorTile.sizex = 1;
						doorTile.sizey = 2;
						possibleDoors.emplace_back(doorTile);
					}
				}
				if (currentTile.posy - 1 >= 0 && !down) {
					for (int i = 0; i < currentTile.sizex; i++) {
						rect doorTile;
						doorTile.posx = currentTile.posx + i;
						doorTile.posy = currentTile.posy;
						doorTile.sizex = 1;
						doorTile.sizey = 4;
						possibleDoors.emplace_back(doorTile);
					}
				}
				if (currentTile.posy + currentTile.sizey + 1 < map->ysize && !up) {
					for (int i = 0; i < currentTile.sizex; i++) {
						rect doorTile;
						doorTile.posx = currentTile.posx + i;
						doorTile.posy = currentTile.posy + currentTile.sizey - 1;
						doorTile.sizex = 1;
						doorTile.sizey = 1;
						possibleDoors.emplace_back(doorTile);
					}
				}
				// Pick one of the possible doors at random
				if (possibleDoors.size() > 0) {
					rect door = possibleDoors[rand() % possibleDoors.size()];
					possibleDoors.clear();
					if (door.sizey == Direction::UP) {
						map->tileArr[door.posx][door.posy][2] += Direction::UP;
						map->tileArr[door.posx][door.posy + 1][2] += Direction::DOWN;
						currentTile.doors += Direction::UP;
					}
					if (door.sizey == Direction::RIGHT) {
						map->tileArr[door.posx][door.posy][2] += Direction::RIGHT;
						map->tileArr[door.posx + 1][door.posy][2] += Direction::LEFT;
						currentTile.doors += Direction::RIGHT;
					}
					if (door.sizey == Direction::DOWN) {
						map->tileArr[door.posx][door.posy][2] += Direction::DOWN;
						map->tileArr[door.posx][door.posy - 1][2] += Direction::UP;
						currentTile.doors += Direction::DOWN;
					}
					if (door.sizey == Direction::LEFT) {
						map->tileArr[door.posx][door.posy][2] += Direction::LEFT;
						map->tileArr[door.posx - 1][door.posy][2] += Direction::RIGHT;
						currentTile.doors += Direction::LEFT;
					}
				}
			}
			map->matched.pop();
			map->matched.push(currentTile);
		}
	}
}

bool LevelGeneratorSystem::hasDoor(Direction dir, int doors) {
	if (dir == Direction::UP && doors % 2 == Direction::UP) {
		return true;
	}
	else if(dir == Direction::DOWN && doors % 8 >= Direction::DOWN){
		return true;
	}
	else if (dir == Direction::RIGHT && doors % 4 >= Direction::RIGHT) {
		return true;
	}
	else if (dir == Direction::LEFT && doors >= Direction::LEFT) {
		return true;
	}
	return false;
}

void LevelGeneratorSystem::addTile(Direction dir, int doors, const std::vector<Model*>& tileModels, float tileSize, int tileOffset, int i, int j, Model* bb) {

	auto tileEntity = ECS::Instance()->createEntity("");
	if (dir == Direction::RIGHT) {
		if (hasDoor(Direction::RIGHT, doors)) {
			tileEntity->addComponent<ModelComponent>(tileModels[TileModel::ROOM_DOOR]);
			tileEntity->addComponent<TransformComponent>(glm::vec3(tileSize * i + tileOffset, 0.f, tileSize * j + tileOffset));
		}
		else {
			tileEntity->addComponent<ModelComponent>(tileModels[TileModel::ROOM_WALL]);
			tileEntity->addComponent<TransformComponent>(glm::vec3(tileSize * i + tileOffset, 0.f, tileSize * j + tileOffset));
		}
	}
	else if (dir == Direction::UP) {
		if (hasDoor(Direction::UP, doors)) {
			tileEntity->addComponent<ModelComponent>(tileModels[TileModel::ROOM_DOOR]);
			tileEntity->addComponent<TransformComponent>(glm::vec3(tileSize * i + tileOffset, 0.f, tileSize * j + tileOffset), glm::vec3(0.f, glm::radians(270.f), 0.f));
		}
		else {
			tileEntity->addComponent<ModelComponent>(tileModels[TileModel::ROOM_WALL]);
			tileEntity->addComponent<TransformComponent>(glm::vec3(tileSize * i + tileOffset, 0.f, tileSize * j + tileOffset), glm::vec3(0.f, glm::radians(270.f), 0.f));
		}
	}
	else if (dir == Direction::DOWN) {
		if (hasDoor(Direction::DOWN, doors)) {
			tileEntity->addComponent<ModelComponent>(tileModels[TileModel::ROOM_DOOR]);
			tileEntity->addComponent<TransformComponent>(glm::vec3(tileSize * i + tileOffset, 0.f, tileSize * j + tileOffset), glm::vec3(0.f, glm::radians(90.f), 0.f));
		}
		else {
			tileEntity->addComponent<ModelComponent>(tileModels[TileModel::ROOM_WALL]);
			tileEntity->addComponent<TransformComponent>(glm::vec3(tileSize * i + tileOffset, 0.f, tileSize * j + tileOffset), glm::vec3(0.f, glm::radians(90.f), 0.f));
		}
	}
	else if (dir == Direction::LEFT) {
		if (hasDoor(Direction::LEFT, doors)) {
			tileEntity->addComponent<ModelComponent>(tileModels[TileModel::ROOM_DOOR]);
			tileEntity->addComponent<TransformComponent>(glm::vec3(tileSize * i + tileOffset, 0.f, tileSize * j + tileOffset), glm::vec3(0.f, glm::radians(180.f), 0.f));
		}
		else {
			tileEntity->addComponent<ModelComponent>(tileModels[TileModel::ROOM_WALL]);
			tileEntity->addComponent<TransformComponent>(glm::vec3(tileSize * i + tileOffset, 0.f, tileSize * j + tileOffset), glm::vec3(0.f, glm::radians(180.f), 0.f));
		}
	}
	else
	{
		tileEntity->addComponent<ModelComponent>(tileModels[TileModel::ROOM_FLOOR]);
		tileEntity->addComponent<TransformComponent>(glm::vec3(tileSize * i + tileOffset, 0.f, tileSize * j + tileOffset));
	}
	tileEntity->getComponent<TransformComponent>()->setScale(glm::vec3(tileSize / 10.f, 1.0f, tileSize / 10.f));
	tileEntity->addComponent<BoundingBoxComponent>(bb);
	tileEntity->addComponent<CollidableComponent>();
	tileEntity->addComponent<CullingComponent>();
}