#pragma once
#include "Component.h"

// This component will eventually contain the health etc of the candles
class CandleComponent : public Component<CandleComponent> {
public:
	CandleComponent();
	virtual ~CandleComponent();

	void hitWithWater(int damage);
	void resetHitByWater();
	bool wasHitByWater() const;
	bool getIsAlive() const;
	bool* getPtrToIsAlive();
	void setIsAlive(bool alive);
	bool getDoActivate() const;
	void setDoActivate();
	void resetDoActivate();
	void addToDownTime(float time);
	void resetDownTime();
	bool isCarried() const;
	void toggleCarried();
	float getDownTime() const;
	void setOwner(int playerEntityID);
	int getOwner() const;
	int getDamageTakenLastHit() const;

private:
	bool m_wasHitByWater = false;
	int m_damageTakenLastHit = 0;
	bool m_isAlive = true;
	bool m_activate = true;
	bool m_carried = true;

	/* Should probably be removed later */
	float m_downTime = 0.f;

	int m_playerEntityID = -1;
};