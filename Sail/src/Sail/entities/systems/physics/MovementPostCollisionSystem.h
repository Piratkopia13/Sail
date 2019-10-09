#pragma once
#include "..//BaseComponentSystem.h"

class MovementPostCollisionSystem final : public BaseComponentSystem {
public:
	MovementPostCollisionSystem();
	~MovementPostCollisionSystem();

	void update(float dt) override;
private:

};