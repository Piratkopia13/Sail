#pragma once

#include "..//BaseComponentSystem.h"

class Scene;
class Model;
struct rect;

enum TileModel {
	ROOM_FLOOR,
	ROOM_WALL,
	ROOM_DOOR,
	CORRIDOR_FLOOR,
	CORRIDOR_WALL,
	CORRIDOR_DOOR,
	NUMBOFMODELS
};

enum Direction {
	NONE = 0,
	UP = 1,
	RIGHT = 2,
	DOWN = 4,
	LEFT = 8
};

class LevelGeneratorSystem final: public BaseComponentSystem {
public:
	LevelGeneratorSystem();
	~LevelGeneratorSystem();

	void update(float dt) override;
	void generateMap();
	//void createWorld(Model* tile1,Model* tile2,Model* tile3,Model* tile4, Model* tile5,Model* tile6, Model* bb);
	void createWorld(const std::vector<Model*>& tileModels, Model* bb);

private:
	int randomizeTileId(std::vector<int>* tiles);
	void findPossibleTiles(std::vector<int>* mapPointer,int posx, int posy);
	void splitChunk();
	void splitBlock();
	void matchRoom();
	int checkBorder(rect rekt);
	bool splitDirection(bool ns);
	void addDoors();
	void addTile(Direction dir, int doors, const std::vector<Model*>& tileModels, float tileSize, int tileOffset, int i, int j, Model* bb);
	bool hasDoor(Direction dir, int doors);
};