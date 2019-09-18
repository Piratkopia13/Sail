#include "pch.h"
#include "PhysicSystem.h"
#include "..//../Physics/Physics.h"
#include "..//..//Entity.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/PhysicsComponent.h"
#include "..//..//components/BoundingBoxComponent.h"

PhysicSystem::PhysicSystem() : BaseComponentSystem() {
	requiredComponentTypes.push_back(TransformComponent::ID);
	requiredComponentTypes.push_back(PhysicsComponent::ID);
	m_octree = nullptr;
}

PhysicSystem::~PhysicSystem() {
}

void PhysicSystem::provideOctree(Octree* octree) {
	m_octree = octree;
}

void PhysicSystem::update(float dt) {
	for (auto& e : entities) {
		TransformComponent* transform = e->getComponent<TransformComponent>();
		PhysicsComponent* physics = e->getComponent<PhysicsComponent>();

		physics->velocity += physics->acceleration * dt;

		//----Collisions----
		BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();
		if (m_octree && boundingBox) {
			//Update collision data
			physics->collisions.clear();
			m_octree->getCollisions(e, &physics->collisions);

			if (physics->collisions.size() > 0) {
				//Get the combined normals
				glm::vec3 sumVec(0.0f);
				for (unsigned int i = 0; i < physics->collisions.size(); i++) {
					sumVec += physics->collisions[i].normal;
				}

				for (unsigned int i = 0; i < physics->collisions.size(); i++) {
					//Stop movement towards triangle
					float projectionSize = glm::dot(physics->velocity, -physics->collisions[i].normal);

					if (projectionSize > 0.0f) { //Is pushing against wall
						physics->velocity += physics->collisions[i].normal * projectionSize; //Limit movement towards wall
					}

					//Tight angle corner special case
					float dotProduct = glm::dot(physics->collisions[i].normal, glm::normalize(sumVec));
					if (dotProduct < 0.98f && dotProduct > 0.0f) { //Colliding in a tight angle corner
						glm::vec3 normalToNormal = sumVec - glm::dot(sumVec, physics->collisions[i].normal) * physics->collisions[i].normal;
						normalToNormal = glm::normalize(normalToNormal);

						//Stop movement towards corner
						projectionSize = glm::dot(physics->velocity, -normalToNormal);

						if (projectionSize > 0.0f) {
							physics->velocity += normalToNormal * projectionSize;
						}
					}
				}
			}
		}
		//------------------
		transform->rotate(physics->rotation * dt);
		transform->translate(physics->velocity * dt);
	}
}
