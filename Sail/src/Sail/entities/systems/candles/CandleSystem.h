#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

#define INVINCIBLE_DURATION 1.0f
#define MAX_HEALTH 10

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
private:
	int m_playerEntityID;
	int m_health = 10;
	bool m_isHit = false;
	float m_invincibleTimer = INVINCIBLE_DURATION;

	float testTimer = 0.0f;
};