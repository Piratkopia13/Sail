#pragma once
#include "../BaseComponentSystem.h"
#ifdef _DEBUG
#include <string>
#endif

class LightSetup;
class PerspectiveCamera;
class Entity;

class LightListSystem final : public BaseComponentSystem {
public:
	LightListSystem();
	~LightListSystem();

	void updateLights(LightSetup* lightSetup);

#ifdef _DEBUG
	void setDebugLightListEntity(std::string name);
	void addPointLightToDebugEntity(LightSetup* lightSetup, PerspectiveCamera* cam);
	void removePointLightFromDebugEntity();
private:
	Entity* m_debugLightListEntity = nullptr;
#endif

};