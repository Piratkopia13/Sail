#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class LightSetup;
class PerspectiveCamera;

class CandleSystem final : public BaseComponentSystem {
public:
	CandleSystem();
	virtual ~CandleSystem();

	void setPlayerCandle(Entity* candle);

	void addLightToCandle(std::string name);

	void update(float dt) override;

private:
	Entity* m_playerCandle = nullptr;

};