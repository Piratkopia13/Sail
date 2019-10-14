#pragma once
#include "..//BaseComponentSystem.h"

class MovementSystem final : public BaseComponentSystem {
public:
	MovementSystem();
	~MovementSystem();

	void update(float dt);
};