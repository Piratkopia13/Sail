#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"
#include "../SPLASH/src/game/states/GameState.h"

// TODO: Replace using game settings when that is implemented


class CameraController;
class LightSetup;
class PerspectiveCamera;
class Octree;

class CandleSystem final : public BaseComponentSystem {
public:
	CandleSystem();
	~CandleSystem();

	void setPlayerEntityID(int entityID, Entity* entityPtr);

	void update(float dt) override;

	void putDownCandle(Entity* e);

	void init(GameState* gameStatePtr, Octree* octree);

private:
	int m_playerEntityID;
	Entity* m_playerEntityPtr;
	GameState* m_gameStatePtr;
	Octree* m_octree;

	// TODO: Replace using game settings when that is implemented
	float m_candleForceRespawnTimer = 2.0f;
	// TODO: Replace using game settings when that is implemented
	//int m_maxNumRespawns = 2;
	int m_maxNumRespawns = 1;  // TODO: REMOVE
	// TO DO: Move or something
	int m_livingCandles = 0;

	void setGameStatePtr(GameState* ptr) { m_gameStatePtr = ptr; }
};