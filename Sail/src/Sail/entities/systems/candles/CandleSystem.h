#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class CameraController;
class LightSetup;
class PerspectiveCamera;

class CandleSystem final : public BaseComponentSystem {
public:
	CandleSystem();
	virtual ~CandleSystem();

	void setPlayerEntityID(int entityID);

	void lightCandle(std::string name);

	void update(float dt) override;
private:
	int m_playerEntityID;
};