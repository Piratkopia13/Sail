#pragma once
#include "..//BaseComponentSystem.h"

class PhysicSystem final : public BaseComponentSystem
{
public:
	PhysicSystem();
	~PhysicSystem();

	void update(float dt) override;

private:
};