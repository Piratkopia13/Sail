#pragma once
#include "Component.h"

#include "Sail/netcode/NetcodeTypes.h"

class Entity;

// TODO: Replace with game settings
constexpr float MAX_HEALTH = 20.f


// TODO: Remove as many functions as possible
// This component will eventually contain the health etc of the candles
class CandleComponent : public Component<CandleComponent> {
public:
	CandleComponent() {}
	virtual ~CandleComponent() {}

	void hitWithWater(float damage, Netcode::PlayerID shooterID) {
		damageTakenLastHit = damage;
		wasHitByPlayerID = shooterID;
	}
	
public:
	Entity* ptrToOwner = nullptr;

	bool hitByLocalPlayer = false;
	bool wasHitByMeThisTick = false;
	bool wasHitByWater = false;
	bool isAlive = true;
	bool isCarried = true;
	bool wasCarriedLastUpdate = true;
	bool isLit = true;
	bool userReignition = false;

	/* Should probably be removed later */
	float downTime = 0.f;
	float invincibleTimer;
	float damageTakenLastHit = 0;
	// TODO: Replace using game settings when that is implemented
	float health = MAX_HEALTH;

	int respawns = 0;

	Netcode::PlayerID playerEntityID;
	Netcode::PlayerID wasHitByPlayerID = 0;
};