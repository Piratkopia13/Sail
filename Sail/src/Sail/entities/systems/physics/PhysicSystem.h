#pragma once
#include "..//BaseComponentSystem.h"
#include "..//..//Physics/Physics.h"
//class Octree;

class PhysicSystem final : public BaseComponentSystem
{
public:
	PhysicSystem();
	~PhysicSystem();

	void provideOctree(Octree* octree);

	void update(float dt) override;

private:
	Octree* m_octree;

	const bool rayCastCheck(Entity* e, float& dt);
	void rayCastUpdate(Entity* e, float& dt);
	const bool collisionUpdate(Entity* thisPhysicalObject, const float& dt);
	const bool handleCollisions(Entity* e, const std::vector<Octree::CollisionInfo>& collisions, const float& dt);
};