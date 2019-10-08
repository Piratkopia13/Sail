#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

#define INVINCIBLE_DURATION 1.0f
#define MAX_HEALTH 100.0f

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
	float m_health = MAX_HEALTH;
	bool m_isHit = false;
	float m_invincibleTimer = INVINCIBLE_DURATION;

	float testTimer = 0.0f;
};