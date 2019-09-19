#include "pch.h"
#include "OctreeAddRemoverSystem.h"

#include "..//../Physics/Physics.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/BoundingBoxComponent.h"

OctreeAddRemoverSystem::OctreeAddRemoverSystem() {
	requiredComponentTypes.push_back(TransformComponent::ID);
	requiredComponentTypes.push_back(BoundingBoxComponent::ID);
	//TODO: Add a component that is specifically for if the entity should be added to octree
}

OctreeAddRemoverSystem::~OctreeAddRemoverSystem() {

}

void OctreeAddRemoverSystem::provideOctree(Octree* octree) {
	m_octree = octree;
	m_octree->addEntities(&entities);
}

void OctreeAddRemoverSystem::addEntity(Entity::SPtr entity) {
	BaseComponentSystem::addEntity(entity);

	if (m_octree) {
		m_octree->addEntity(entity);
	}
}

void OctreeAddRemoverSystem::removeEntity(Entity::SPtr entity) {
	BaseComponentSystem::removeEntity(entity);

	if (m_octree) {
		m_octree->removeEntity(entity);
	}
}


void OctreeAddRemoverSystem::update(float dt) {
	m_octree->update();
}