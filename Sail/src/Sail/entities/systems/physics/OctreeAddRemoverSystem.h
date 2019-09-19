#pragma once
#include "..//BaseComponentSystem.h"

class Octree;

class OctreeAddRemoverSystem final : public BaseComponentSystem
{
public:
	OctreeAddRemoverSystem();
	~OctreeAddRemoverSystem();

	void provideOctree(Octree* octree);

	void addEntity(Entity::SPtr entity) override;

	void removeEntity(Entity::SPtr entity) override;

	void update(float dt) override;

private:
	Octree* m_octree;
};