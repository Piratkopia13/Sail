#include "pch.h"
#include "OctreeAddRemoverSystem.h"

#include "..//../Physics/Physics.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/BoundingBoxComponent.h"
#include "..//..//components/CollidableComponent.h"

OctreeAddRemoverSystem::OctreeAddRemoverSystem() {
	requiredComponentTypes.push_back(BoundingBoxComponent::ID);
	requiredComponentTypes.push_back(CollidableComponent::ID);
}

OctreeAddRemoverSystem::~OctreeAddRemoverSystem() {

}

void OctreeAddRemoverSystem::provideOctree(Octree* octree) {
	m_octree = octree;
	m_octree->addEntities(&entities);
}

bool OctreeAddRemoverSystem::addEntity(Entity* entity) {
	if (BaseComponentSystem::addEntity(entity)) {
		if (m_octree) {
			m_octree->addEntity(entity);
			return true;
		}
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