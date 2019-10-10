#pragma once

#include "..//BaseComponentSystem.h"

class Scene;
class Model;
struct rect;
class LevelGeneratorSystem final: public BaseComponentSystem {
public:
	LevelGeneratorSystem();
	~LevelGeneratorSystem();

	void update(float dt) override;
	void generateMap();
	void createWorld(Model* tile1,Model* tile2,Model* tile3,Model* tile4, Model* tile5,Model* tile6, Model* bb);
	//void createWorld(std::vector<Model*> tiles, Model* bb);

private:
	int randomizeTileId(std::vector<int>* tiles);
	void findPossibleTiles(std::vector<int>* mapPointer,int posx, int posy);
	void splitChunk();
	void splitBlock();
	void matchRoom();
	int checkBorder(rect rekt);
	bool splitDirection(bool ns);
	void addDoors();
};