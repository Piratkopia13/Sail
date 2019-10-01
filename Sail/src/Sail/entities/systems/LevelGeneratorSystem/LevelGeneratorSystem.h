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
	void createWorld(Scene* scene, Model* tile1,Model* tile2,Model* tile3,Model* tile4, Model* tile5,Model* tile6, Model* bb);

private:
	int randomizeTileId(std::vector<int>* tiles);
	void findPossibleTiles(std::vector<int>* mapPointer,int posx, int posy);
	void splitChunk();
	void splitBlock();
	void matchRoom();
	bool checkBorder(rect rekt);
};