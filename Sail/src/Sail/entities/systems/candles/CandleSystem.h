#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

// TODO: Replace using game settings when that is implemented
#define INVINCIBLE_DURATION 1.0f
// TODO: Replace using game settings when that is implemented
#define MAX_HEALTH 20.f

class CameraController;
class LightSetup;
class PerspectiveCamera;

class CandleSystem final : public BaseComponentSystem {
public:
	CandleSystem();
	~CandleSystem();

	void setPlayerEntityID(int entityID);

	void lightCandle(const std::string& name);

	void update(float dt) override;

	void putDownCandle(Entity* e);

private:
	int m_playerEntityID;
	// TODO: Replace using game settings when that is implemented
	float m_candleForceRespawnTimer = 15.f;
	// TODO: Replace using game settings when that is implemented
	int m_maxNumRespawns = 2;

	float testTimer = 0.0f;
};