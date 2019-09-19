#pragma once
#include "..//BaseComponentSystem.h"

class Octree;

class OctreeAddRemoverSystem final : public BaseComponentSystem
{
public:
	OctreeAddRemoverSystem();
	~OctreeAddRemoverSystem();

	void provideOctree(Octree* octree);

	void addEntity(Entity* entity) override;

	void removeEntity(Entity* entity) override;

	void update(float dt) override;

private:
	Octree* m_octree;
};