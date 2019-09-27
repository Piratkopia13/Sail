#pragma once
#include "Component.h"

// This component will eventually contain the health etc of the candles
class CandleComponent : public Component<CandleComponent> {
public:
	CandleComponent();
	virtual ~CandleComponent();

	void hitWithWater();
	void resetHitByWater();
	bool wasHitByWater() const;
	bool getIsAlive() const;
	void setIsAlive(bool alive);
	bool getDoActivate() const;
	void resetDoActivate();

	/* Should probably be removed later */
	void addToDownTime(float time);
	void resetDownTime();
	float getDownTime() const;

private:
	bool m_wasHitByWater = false;
	bool m_isAlive = true;
	bool m_activate = true;

	/* Should probably be removed later */
	float m_downTime = 0.f;
};