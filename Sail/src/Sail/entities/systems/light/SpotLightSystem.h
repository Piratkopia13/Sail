#pragma once
#include "../BaseComponentSystem.h"

class LightSetup;
class PerspectiveCamera;

class SpotLightSystem final : public BaseComponentSystem {
public:
	SpotLightSystem();
	~SpotLightSystem();

	void updateLights(LightSetup* lightSetup, float alpha);
};