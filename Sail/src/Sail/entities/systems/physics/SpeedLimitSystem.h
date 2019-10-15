#pragma once
#include "..//BaseComponentSystem.h"

class SpeedLimitSystem final : public BaseComponentSystem {
public:
	SpeedLimitSystem();
	~SpeedLimitSystem();

	void update();
};