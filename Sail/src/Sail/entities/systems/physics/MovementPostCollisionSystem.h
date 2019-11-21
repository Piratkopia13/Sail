#pragma once
#include "..//BaseComponentSystem.h"

class MovementPostCollisionSystem final : public BaseComponentSystem {
public:
	MovementPostCollisionSystem();
	~MovementPostCollisionSystem() = default;

	void update(float dt);
private:

};
