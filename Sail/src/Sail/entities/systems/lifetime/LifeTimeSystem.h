#pragma once
#include "..//BaseComponentSystem.h"

class LifeTimeSystem : public BaseComponentSystem {
public:
	LifeTimeSystem();
	~LifeTimeSystem();
	void update(float dt);
};