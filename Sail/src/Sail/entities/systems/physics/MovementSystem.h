#pragma once
#include "..//BaseComponentSystem.h"

class MovementSystem final : public BaseComponentSystem {
public:
	MovementSystem();
	~MovementSystem() = default;

	void update(float dt);
};