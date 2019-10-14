#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class LightSetup;
class PerspectiveCamera;

class LightSystem final : public BaseComponentSystem {
public:
	LightSystem();
	~LightSystem();

	void updateLights(LightSetup* lightSetup);

#ifdef _DEBUG
	void setDebugLightListEntity(std::string name);
	void addPointLightToDebugEntity(LightSetup* lightSetup, PerspectiveCamera* cam);
	void removePointLightFromDebugEntity();
private:
	Entity* m_debugLightListEntity = nullptr;
#endif

};