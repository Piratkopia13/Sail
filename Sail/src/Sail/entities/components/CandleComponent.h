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
	void putDown();
	void pickUp();
	bool isCarried() const;


private:
	bool m_wasHitByWater = false;
	bool m_carried = true;

};