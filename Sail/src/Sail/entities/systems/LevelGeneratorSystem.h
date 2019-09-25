#pragma once

#include "BaseComponentSystem.h"

class Scene;
class Model;
class LevelGeneratorSystem : public BaseComponentSystem {
public:
	LevelGeneratorSystem();
	~LevelGeneratorSystem();

	void update(float dt) override;
	void generateMap();
	void createWorld(Scene* scene, Model* tile1,Model* tile2, Model* bb);

private:
};