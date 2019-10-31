#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class CandleReignitionSystem final : public BaseComponentSystem {
public:
	CandleReignitionSystem();
	~CandleReignitionSystem();

	void update(float dt) override;

private:
	// TODO: Replace using game settings when that is implemented
	float m_candleForceRespawnTimer = 5.0f;

};