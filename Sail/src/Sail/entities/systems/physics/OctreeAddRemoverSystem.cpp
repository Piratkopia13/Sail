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

void OctreeAddRemoverSystem::addEntity(Entity* entity) {
	BaseComponentSystem::addEntity(entity);

	if (m_octree) {
		m_octree->addEntity(entity);
	}
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