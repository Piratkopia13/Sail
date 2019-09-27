#pragma once
#include "..//BaseComponentSystem.h"

class Octree;

class PhysicSystem final : public BaseComponentSystem
{
public:
	PhysicSystem();
	~PhysicSystem();

	void provideOctree(Octree* octree);

	void update(float dt) override;

private:
	Octree* m_octree;

	void rayCastUpdate(Entity* e, float& dt);
	void collisionUpdate(Entity* e, float dt);
};