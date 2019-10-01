#pragma once
#include "..//BaseComponentSystem.h"

class Octree;
class GameDataTracker;

class PhysicSystem final : public BaseComponentSystem
{
public:
	PhysicSystem();
	~PhysicSystem();

	void provideOctree(Octree* octree);

	void update(float dt) override;

private:
	Octree* m_octree = nullptr;
	GameDataTracker* m_gameDataTracker = nullptr;
};