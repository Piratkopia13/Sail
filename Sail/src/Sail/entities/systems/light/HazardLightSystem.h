#pragma once
#include "../BaseComponentSystem.h"

class LightSetup;
class PerspectiveCamera;

class HazardLightSystem final : public BaseComponentSystem {
public:
	HazardLightSystem();
	~HazardLightSystem();

	const std::vector<Entity*>* getHazardLightEntities();
	void updateLights(LightSetup* lightSetup, float alpha, float dt);
	void toggleONOFF();
	void enableHazardLights(std::vector<int> activeRooms);
};