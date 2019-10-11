#include "pch.h"
#include "OctreeAddRemoverSystem.h"

#include "..//../Physics/Physics.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/BoundingBoxComponent.h"
#include "..//..//components/CollidableComponent.h"
#include "..//..//components/ModelComponent.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/graphics/camera/Camera.h"

OctreeAddRemoverSystem::OctreeAddRemoverSystem() 
	: m_doCulling(false)
	, m_cullCamera(nullptr)
{
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
	//BaseComponentSystem::removeEntity(entity);
	entity->queueDestruction();

	if (m_octree) {
		m_octree->removeEntity(entity);
	}
}


void OctreeAddRemoverSystem::update(float dt) {
	m_octree->update();
	
	if (m_doCulling) {
		// Let the renderer know that all entities should not be rendered - will be set to true in cull method call if they are visible
		for (auto& entity : entities) {
			auto* modelComponent = entity->getComponent<ModelComponent>();
			if (modelComponent) {
				for (unsigned int j = 0; j < modelComponent->getModel()->getNumberOfMeshes(); j++) {
					modelComponent->getModel()->getMesh(j)->setIsVisibleOnScreen(false);
				}
			}
		}
		m_octree->frustumCulledDraw(*m_cullCamera);
	}
}

void OctreeAddRemoverSystem::setCulling(bool activated, Camera* camera) {
	m_doCulling = activated;
	m_cullCamera = camera;
}
