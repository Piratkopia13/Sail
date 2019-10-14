#pragma once
#include "../BaseComponentSystem.h"

class LightSetup;
class PerspectiveCamera;

class LightSystem final : public BaseComponentSystem {
public:
	LightSystem();
	~LightSystem();

	void updateLights(LightSetup* lightSetup);
};