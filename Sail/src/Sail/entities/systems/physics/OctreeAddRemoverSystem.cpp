#include "pch.h"
#include "OctreeAddRemoverSystem.h"

#include "..//../Physics/Physics.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/BoundingBoxComponent.h"
#include "..//..//components/CollidableComponent.h"

OctreeAddRemoverSystem::OctreeAddRemoverSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<BoundingBoxComponent>(true, true, true);
	registerComponent<CollidableComponent>(true, true, true);
}

OctreeAddRemoverSystem::~OctreeAddRemoverSystem() {

}

void OctreeAddRemoverSystem::provideOctree(Octree* octree) {
	m_octree = octree;
	m_octree->addEntities(&entities);
}

bool OctreeAddRemoverSystem::addEntity(Entity* entity) {
	if (BaseComponentSystem::addEntity(entity) && m_octree) {
			m_octree->addEntity(entity);
			return true;
	}
	return false;
}

void OctreeAddRemoverSystem::removeEntity(Entity* entity) {
	BaseComponentSystem::removeEntity(entity);

	if (m_octree) {
		m_octree->removeEntity(entity);
	}
}


void OctreeAddRemoverSystem::update(float dt) {
	m_octree->update();
}