#pragma once
#include "Component.h"

// This component will eventually contain the health etc of the candles
class CandleComponent : public Component<CandleComponent> {
public:
	CandleComponent();
	virtual ~CandleComponent();

	void hitWithWater(float damage);
	void resetHitByWater();
	bool wasHitByWater() const;
	bool getIsAlive() const;
	bool* getPtrToIsAlive();
	void setIsAlive(bool alive);
	bool getDoActivate() const;
	void activate();
	void resetDoActivate();
	void addToDownTime(float time);
	void resetDownTime();
	bool isCarried() const;
	void setWasCarriedLastUpdate(const bool wasCarried);
	bool getWasCarriedLastUpdate() const;
	void toggleCarried();
	float getDownTime() const;
	bool getIsLit() const;
	void setIsLit(const bool isLit);
	int getNumRespawns() const;
	void incrementRespawns();
	void setOwner(int playerEntityID);
	int getOwner() const;
	int getDamageTakenLastHit() const;

private:
	bool m_wasHitByWater = false;
	float m_damageTakenLastHit = 0;
	bool m_isAlive = true;
	bool m_activate = true;
	bool m_carried = true;
	bool m_wasCarriedLastUpdate = true;

	/* Should probably be removed later */
	float m_downTime = 0.f;

	bool m_isLit = true;
	int m_respawns = 0;

	int m_playerEntityID = -1;
};