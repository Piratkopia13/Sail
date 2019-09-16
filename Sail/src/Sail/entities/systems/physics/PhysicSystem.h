#pragma once
#include "..//ComponentSystem.h"

class PhysicSystem final : public ComponentSystem<PhysicSystem>
{
public:
	PhysicSystem();
	~PhysicSystem();

	void update(float dt) override;

private:
};