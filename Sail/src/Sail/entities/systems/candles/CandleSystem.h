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

	void setPlayerEntityID(int entityID, Entity* entityPtr);

	void lightCandle(const std::string& name);

	void update(float dt) override;

	void putDownCandle(Entity* e);

	void init(GameState* gameStatePtr);

private:
	int m_playerEntityID;
	Entity* m_playerEntityPtr;
	GameState* m_gameStatePtr;

	// TODO: Replace using game settings when that is implemented
	float m_candleForceRespawnTimer = 2.0f;
	// TODO: Replace using game settings when that is implemented
	int m_maxNumRespawns = 0;
	// TO DO: Move or something
	int m_livingCandles = 0;

	float testTimer = 0.0f;

	void setGameStatePtr(GameState* ptr) { m_gameStatePtr = ptr; }
};