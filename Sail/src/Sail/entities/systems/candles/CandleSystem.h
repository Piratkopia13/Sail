#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"
#include "../SPLASH/src/game/states/GameState.h"

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

	void setGameStatePtr(GameState* ptr) { m_gameStatePtr = ptr; }
	void resetGameStatePtr() { m_gameStatePtr = nullptr; }

private:
	int m_playerEntityID;
	GameState* m_gameStatePtr;

	// TODO: Replace using game settings when that is implemented
	float m_candleForceRespawnTimer = 2.0f;
	// TODO: Replace using game settings when that is implemented
	int m_maxNumRespawns = 1;

	float testTimer = 0.0f;
};