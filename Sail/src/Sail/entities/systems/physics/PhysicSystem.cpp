#include "pch.h"
#include "PhysicSystem.h"
#include "..//../Physics/Physics.h"
#include "..//..//Entity.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/PhysicsComponent.h"
#include "..//..//components/BoundingBoxComponent.h"

#include "..//Sail/src/Sail/api/Input.h"
#include "..//Sail/src/Sail/KeyCodes.h"

PhysicSystem::PhysicSystem() : BaseComponentSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<PhysicsComponent>(true, true, true);

	// Assumed?
	registerComponent<BoundingBoxComponent>(false, true, true);

	m_octree = nullptr;
}

PhysicSystem::~PhysicSystem() {
}

void PhysicSystem::provideOctree(Octree* octree) {
	m_octree = octree;
}

const bool PhysicSystem::collisionUpdate(Entity* thisPhysicalObject, const float& dt) {
	//Update collision data
	std::vector<Octree::CollisionInfo> collisions;

	m_octree->getCollisions(thisPhysicalObject, &collisions);

	return handleCollisions(thisPhysicalObject, collisions, dt);
}

const bool PhysicSystem::handleCollisions(Entity* e, const std::vector<Octree::CollisionInfo>& collisions, const float& dt) {
	bool returnValue = false;

	TransformComponent* transform = e->getComponent<TransformComponent>();
	PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
	BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();

	physics->onGround = false;
	std::vector<int> groundIndices;

	if (collisions.size() > 0) {
		//Get the combined normals
		glm::vec3 sumVec(0.0f);
		for (unsigned int i = 0; i < collisions.size(); i++) {
			sumVec += collisions[i].normal;

			//Add collision to current collisions
			physics->collisions.push_back(collisions[i]);
		}

		for (unsigned int i = 0; i < collisions.size(); i++) {
			if (collisions[i].normal.y > 0.7f) {
				physics->onGround = true;
				bool newGround = true;
				for (unsigned int j = 0; j < groundIndices.size(); j++) {
					if (collisions[i].normal == collisions[groundIndices[j]].normal) {
						newGround = false;
					}
				}
				if (newGround) {
					//Save collision for friction calculation
					groundIndices.push_back(i);
				}
			}

			//Stop movement towards triangle
			float projectionSize = glm::dot(physics->velocity, -collisions[i].normal);

			if (projectionSize > 0.0f) { //Is pushing against wall
				returnValue = true;
				physics->velocity += collisions[i].normal * projectionSize * (1.0f + physics->bounciness); //Limit movement towards wall
			}

			//Tight angle corner special case
			float dotProduct = glm::dot(collisions[i].normal, glm::normalize(sumVec));
			if (dotProduct < 0.7072f && dotProduct > 0.0f) { //Colliding in a tight angle corner
				glm::vec3 normalToNormal = sumVec - glm::dot(sumVec, collisions[i].normal) * collisions[i].normal;
				normalToNormal = glm::normalize(normalToNormal);

				//Stop movement towards corner
				projectionSize = glm::dot(physics->velocity, -normalToNormal);

				if (projectionSize > 0.0f) {
					returnValue = true;
					physics->velocity += normalToNormal * projectionSize * (1.0f + physics->bounciness);
				}
			}
		}
	}
	//------------------

	//----Drag----
	if (physics->onGround) { //Ground drag
		unsigned int nrOfGroundCollisions = groundIndices.size();
		for (unsigned int i = 0; i < nrOfGroundCollisions; i++) {
			glm::vec3 velAlongPlane = physics->velocity - collisions[groundIndices[i]].normal * glm::dot(collisions[groundIndices[i]].normal, physics->velocity);
			float sizeOfVel = glm::length(velAlongPlane);
			if (sizeOfVel > 0.0f) {
				float slowdown = glm::min((physics->drag / nrOfGroundCollisions) * dt, sizeOfVel);
				physics->velocity -= slowdown * glm::normalize(velAlongPlane);
				returnValue = true;
			}
		}
	}
	else { //Air drag
		float saveY = physics->velocity.y;
		physics->velocity.y = 0;
		float vel = glm::length(physics->velocity);

		if (vel > 0.0f) {
			vel = glm::max(vel - physics->airDrag * dt, 0.0f);
			physics->velocity = glm::normalize(physics->velocity) * vel;
		}
		physics->velocity.y = saveY;
	}
	//------------

	return returnValue;
}

void PhysicSystem::rayCastUpdate(Entity* e, float& padding, const float originalPadding, float& dt) {
	PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
	TransformComponent* transform = e->getComponent<TransformComponent>();
	BoundingBox* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();

	float velocityAmp = glm::length(physics->velocity) * dt;

	//Problem I am facing is this becomes false for the small padding in the end of a frame. And the next frame the padding is so big that it has already passed the wall.
	if (std::abs(physics->velocity.x * dt) > std::abs(boundingBox->getHalfSize().x)
		|| std::abs(physics->velocity.y * dt) > std::abs(boundingBox->getHalfSize().y)
		|| std::abs(physics->velocity.z * dt) > std::abs(boundingBox->getHalfSize().z)) {

		//Object is moving at a speed that risks missing collisions
		//Ray cast to find upcoming collisions, use padding for "swept sphere"
		Octree::RayIntersectionInfo intersectionInfo;
		m_octree->getRayIntersection(boundingBox->getPosition(), physics->velocity, &intersectionInfo, e, padding);

		if ((intersectionInfo.closestHit) <= velocityAmp + originalPadding && intersectionInfo.closestHit >= 0.0f) { //Found upcoming collision
			//Calculate new dt
			float newDt = ((intersectionInfo.closestHit) / velocityAmp) * dt;

			//Move untill first overlap
			boundingBox->setPosition(boundingBox->getPosition() + physics->velocity * newDt);
			transform->translate(physics->velocity * newDt);

			dt -= newDt;

			//Collision update
			if (collisionUpdate(e, dt)) {
				//Velocity changed due to collision
				physics->velocity = glm::vec3(0.0f);
				physics->constantAcceleration = glm::vec3(0.0f);
				padding = originalPadding;
			}
			else {
				padding *= 0.9f;
			}

			physics->m_oldVelocity = physics->velocity;

			rayCastUpdate(e, padding, originalPadding, dt);
		}
	}
}

void PhysicSystem::update(float dt) {
	for (auto& e : entities) {
		PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
		TransformComponent* transform = e->getComponent<TransformComponent>();
		BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();

		physics->collisions.clear();

		physics->velocity += physics->constantAcceleration * dt;
		physics->velocity += physics->accelerationToAdd * dt;

		if (boundingBox && m_octree) {
			if (e->getName() == "projectile" && Input::IsKeyPressed(SAIL_KEY_L)) {
				std::cout << "Break\n";
			}

			m_lastRayIntersect = nullptr;
			//Get ray padding for swept sphere
			float padding = glm::length(boundingBox->getBoundingBox()->getHalfSize());
			if (e->getName() == "projectile") {
				std::cout << "\n";
			}
			rayCastUpdate(e, padding, padding, dt);
			if (collisionUpdate(e, dt) && e->getName() == "projectile") {
				physics->velocity = glm::vec3(0.0f);
				physics->constantAcceleration = glm::vec3(0.0f);
			}
		}

		//----Max Speed Limiter----
		float saveY = physics->velocity.y;
		physics->velocity.y = 0;
		float vel = glm::length(physics->velocity);
		if (vel > 0.0f) {
			//Limit max speed
			if (vel > physics->maxSpeed) {
				vel = physics->maxSpeed;
			}
			else if (vel < 0.05f) {
				vel = 0.0f;
			}

			physics->velocity = vel * glm::normalize(physics->velocity);
		}
		physics->velocity.y = saveY;
		//-------------------------

		transform->rotate(physics->constantRotation * dt);
		transform->translate((physics->m_oldVelocity + physics->velocity) * 0.5f * dt);
		physics->m_oldVelocity = physics->velocity;
		physics->accelerationToAdd = glm::vec3(0.0f);
	}
}
