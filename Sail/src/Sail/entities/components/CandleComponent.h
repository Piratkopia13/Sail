#pragma once
#include "Component.h"

#include "Sail/netcode/NetcodeTypes.h"

class Entity;

#define MAX_HEALTH 20.f


// TODO: Remove as many functions as possible
// This component will eventually contain the health etc of the candles
class CandleComponent : public Component<CandleComponent> {
public:
	CandleComponent() {}
	virtual ~CandleComponent() {}

	void hitWithWater(float damage, Netcode::PlayerID shooterID) {
		/*if (invincibleTimer <= 0.f) {
			if (health > 0.0f) {
				invincibleTimer = 0.4f;
				health -= damage;
				damageTakenLastHit = damage;
				wasHitByWater = true;

				if (health <= 0.0f) {
					wasHitByPlayerID = shooterID;
					isLit = false;
				}
			}
		}*/
		damageTakenLastHit = damage;
		wasHitByPlayerID = shooterID;
	}
	
public:
	Entity* ptrToOwner = nullptr;

	bool hitByLocalPlayer = false;
	bool wasHitByWater = false;
	bool isAlive = true;
	bool isCarried = true;
	bool wasCarriedLastUpdate = true;
	bool isLit = true;

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