#pragma once

#include "../BaseComponentSystem.h"

class Entity;

/*
Crosshair is rendered in InGameGui

*/

class CrosshairSystem : public BaseComponentSystem {
public:
	CrosshairSystem();
	~CrosshairSystem();

	void update(float dt);

private:

	void alterCrosshair(Entity* e, float dt);

};