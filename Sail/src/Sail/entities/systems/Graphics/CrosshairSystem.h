#pragma once

#include "../BaseComponentSystem.h"
#include "Sail/Application.h"

class Entity;
class CrosshairComponent;

/*
Crosshair is rendered in InGameGui.cpp
*/

class CrosshairSystem : public BaseComponentSystem {
public:
	CrosshairSystem();
	~CrosshairSystem();

	void update(float dt);

private:
	SettingStorage* m_settings = nullptr;

	void applySettings(CrosshairComponent* c);
	void alterCrosshair(Entity* e, float dt);
};