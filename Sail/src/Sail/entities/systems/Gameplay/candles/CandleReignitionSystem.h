#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"
#include "Sail/events/EventReceiver.h"

class CandleReignitionSystem final : public BaseComponentSystem, public EventReceiver {
public:
	CandleReignitionSystem();
	~CandleReignitionSystem();

	void update(float dt) override;

private:
	bool onEvent(const Event& event) override;

private:
	// TODO: Replace using game settings when that is implemented
	float m_candleForceRespawnTimer = 5.0f;
};