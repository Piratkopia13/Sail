#pragma once
#include "../BaseComponentSystem.h"

class LightSetup;
class PerspectiveCamera;

class SpotLightSystem final : public BaseComponentSystem {
public:
	SpotLightSystem();
	~SpotLightSystem();

	//void stop() override;
	void updateLights(LightSetup* lightSetup, float alpha, float dt);
	void toggleONOFF();
	void enableHazardLights(std::vector<int> activeRooms);
};