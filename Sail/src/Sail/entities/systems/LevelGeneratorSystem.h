#pragma once

#include "BaseComponentSystem.h"
#include "..//Entity.h"

class Scene;
class Model;
class LevelGeneratorSystem : public BaseComponentSystem {
public:
	LevelGeneratorSystem();
	~LevelGeneratorSystem();

	void update(float dt) override;
	void createWorld(Scene* scene, Model* model);
private:
};