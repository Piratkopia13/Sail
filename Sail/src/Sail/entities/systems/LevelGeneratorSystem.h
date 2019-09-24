#pragma once

#include "BaseComponentSystem.h"


class LevelGeneratorSystem : public BaseComponentSystem {
public:
	LevelGeneratorSystem();
	~LevelGeneratorSystem();

	void createTiles();

	void update(float dt) override;

private:
};