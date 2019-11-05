#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class CandleHealthSystem final : public BaseComponentSystem {
public:
	CandleHealthSystem();
	~CandleHealthSystem();

	void update(float dt) override;

private:
	// TODO: Replace using game settings when that is implemented
	int m_maxNumRespawns = 2;

};