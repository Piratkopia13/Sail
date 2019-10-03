#include "pch.h"
#include "PhysicSystem.h"
#include "..//../Physics/Physics.h"
#include "..//..//Entity.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/PhysicsComponent.h"
#include "..//..//components/BoundingBoxComponent.h"
#include "Sail/utils/GameDataTracker.h"

#include "..//Sail/src/Sail/api/Input.h"
#include "..//Sail/src/Sail/KeyCodes.h"

PhysicSystem::PhysicSystem() : BaseComponentSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<PhysicsComponent>(true, true, true);

	// Assumed?
	registerComponent<BoundingBoxComponent>(false, true, true);

	m_octree = nullptr;
	m_gameDataTracker = &GameDataTracker::getInstance();
}

PhysicSystem::~PhysicSystem() {
}

void PhysicSystem::provideOctree(Octree* octree) {
	m_octree = octree;
}

const bool PhysicSystem::collisionUpdate(Entity* thisPhysicalObject, PhysicsComponent* physicsComp, const float& dt) {
	//Update collision data
	std::vector<Octree::CollisionInfo> collisions;

	m_octree->getCollisions(thisPhysicalObject, &collisions);

	return handleCollisions(thisPhysicalObject, physicsComp, collisions, dt);
}

const bool PhysicSystem::handleCollisions(Entity* e, PhysicsComponent* physicsComp, const std::vector<Octree::CollisionInfo>& collisions, const float& dt) {
	bool returnValue = false;

	physicsComp->onGround = false;
	std::vector<int> groundIndices;

	if (collisions.size() > 0) {
		//Get the combined normals
		glm::vec3 sumVec(0.0f);
		for (unsigned int i = 0; i < collisions.size(); i++) {
			sumVec += collisions[i].normal;

			//Add collision to current collisions
			physicsComp->collisions.push_back(collisions[i]);
		}

		for (unsigned int i = 0; i < collisions.size(); i++) {
			if (collisions[i].normal.y > 0.7f) {
				physicsComp->onGround = true;
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
			float projectionSize = glm::dot(physicsComp->velocity, -collisions[i].normal);

			if (projectionSize > 0.0f) { //Is pushing against wall
				returnValue = true;
				physicsComp->velocity += collisions[i].normal * projectionSize * (1.0f + physicsComp->bounciness); //Limit movement towards wall
			}

			//Tight angle corner special case
			float dotProduct = glm::dot(collisions[i].normal, glm::normalize(sumVec));
			if (dotProduct < 0.7072f && dotProduct > 0.0f) { //Colliding in a tight angle corner
				glm::vec3 normalToNormal = sumVec - glm::dot(sumVec, collisions[i].normal) * collisions[i].normal;
				normalToNormal = glm::normalize(normalToNormal);

				//Stop movement towards corner
				projectionSize = glm::dot(physicsComp->velocity, -normalToNormal);

				if (projectionSize > 0.0f) {
					returnValue = true;
					physicsComp->velocity += normalToNormal * projectionSize * (1.0f + physicsComp->bounciness);
				}
			}
		}
	}
	//------------------

	//----Drag----
	if (physicsComp->onGround) { //Ground drag
		unsigned int nrOfGroundCollisions = groundIndices.size();
		for (unsigned int i = 0; i < nrOfGroundCollisions; i++) {
			glm::vec3 velAlongPlane = physicsComp->velocity - collisions[groundIndices[i]].normal * glm::dot(collisions[groundIndices[i]].normal, physicsComp->velocity);
			float sizeOfVel = glm::length(velAlongPlane);
			if (sizeOfVel > 0.0f) {
				float slowdown = glm::min((physicsComp->drag / nrOfGroundCollisions) * dt, sizeOfVel);
				physicsComp->velocity -= slowdown * glm::normalize(velAlongPlane);
				returnValue = true;
			}
		}
	}
	else { //Air drag
		float saveY = physicsComp->velocity.y;
		physicsComp->velocity.y = 0;
		float vel = glm::length(physicsComp->velocity);

		if (vel > 0.0f) {
			vel = glm::max(vel - physicsComp->airDrag * dt, 0.0f);
			physicsComp->velocity = glm::normalize(physicsComp->velocity) * vel;
		}
		physicsComp->velocity.y = saveY;
	}
	//------------

	return returnValue;
}

const bool PhysicSystem::rayCastCheck(Entity* e, PhysicsComponent* physicsComp, BoundingBox* boundingBox, float& dt) {
	if (glm::abs(physicsComp->velocity.x * dt) > glm::abs(boundingBox->getHalfSize().x)
		|| glm::abs(physicsComp->velocity.y * dt) > glm::abs(boundingBox->getHalfSize().y)
		|| glm::abs(physicsComp->velocity.z * dt) > glm::abs(boundingBox->getHalfSize().z)) {
		//Object is moving at a speed that risks missing collisions
		return true;
	}
	return false;
}

void PhysicSystem::rayCastUpdate(Entity* e, PhysicsComponent* physicsComp, BoundingBox* boundingBox, TransformComponent* transform, float& dt) {

	float velocityAmp = glm::length(physicsComp->velocity) * dt;

	//Ray cast to find upcoming collisions, use padding for "swept sphere"
	Octree::RayIntersectionInfo intersectionInfo;
	m_octree->getRayIntersection(boundingBox->getPosition(), physicsComp->velocity, &intersectionInfo, e, physicsComp->padding);

	if (intersectionInfo.closestHit <= velocityAmp && intersectionInfo.closestHit >= 0.0f) { //Found upcoming collision
		//Calculate new dt
		float newDt = ((intersectionInfo.closestHit) / velocityAmp) * dt;

		//Move untill first overlap
		boundingBox->setPosition(boundingBox->getPosition() + physicsComp->velocity * newDt);
		transform->translate(physicsComp->velocity * newDt);

		dt -= newDt;

		//Collision update
		bool paddingTooBig = true;

		for (unsigned int i = 0; i < intersectionInfo.info.size(); i++) {
			if (Intersection::AabbWithTriangle(*boundingBox, intersectionInfo.info[i].positions[0], intersectionInfo.info[i].positions[1], intersectionInfo.info[i].positions[2])) {
				physicsComp->collisions.push_back(intersectionInfo.info[i]);

				//Stop movement towards triangle
				float projectionSize = glm::dot(physicsComp->velocity, -intersectionInfo.info[i].normal);

				if (projectionSize > 0.0f) { //Is pushing against wall
					physicsComp->velocity += intersectionInfo.info[i].normal * projectionSize * (1.0f + physicsComp->bounciness); //Limit movement towards wall
					paddingTooBig = false;
				}
			}
		}

		if (paddingTooBig) {
			physicsComp->padding *= 0.5f;
		}

		rayCastUpdate(e, physicsComp, boundingBox, transform, dt);
	}
}

void PhysicSystem::surfaceFromCollision(Entity* e, BoundingBox* boundingBox, TransformComponent* transform, const std::vector<Octree::CollisionInfo>& collisions) {
	glm::vec3 distance(0.0f);

	float stepSize = 0.005f;

	for (unsigned int i = 0; i < collisions.size(); i++) {
		//Find approx how far into the wall the character is
		bool colliding = true;
		while (colliding) {
			//Push the character out of the wall
			boundingBox->setPosition(boundingBox->getPosition() + collisions[i].normal * stepSize);

			glm::vec3 middle = (collisions[i].positions[0] + collisions[i].positions[1] + collisions[i].positions[2]) / 3.0f;

			glm::vec3 newPosition0 = middle + (collisions[i].positions[0] - middle) * 0.9f;
			glm::vec3 newPosition1 = middle + (collisions[i].positions[1] - middle) * 0.9f;
			glm::vec3 newPosition2 = middle + (collisions[i].positions[2] - middle) * 0.9f;

			bool currentCollision = Intersection::AabbWithTriangle(*boundingBox, newPosition0, newPosition1, newPosition2);
			if (!currentCollision) {
				colliding = false;

				//Go back to previous position to avoid loosing contact
				boundingBox->setPosition(boundingBox->getPosition() - collisions[i].normal * stepSize);
			}
			else {
				distance += collisions[i].normal * stepSize;
			}
		}
	}

	transform->translate(distance);
}

void PhysicSystem::update(float dt) {
	for (auto& e : entities) {
		PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
		TransformComponent* transform = e->getComponent<TransformComponent>();
		BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();

		physics->collisions.clear();

		if (physics->padding < 0.0f) {
			physics->padding = glm::length(boundingBox->getBoundingBox()->getHalfSize());
		}

		physics->velocity += physics->constantAcceleration * dt;
		physics->velocity += physics->accelerationToAdd * dt;

		if (physics->constantRotation != glm::vec3(0.0f)) {
			transform->rotate(physics->constantRotation * dt);
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

		float updateableDt = dt;

		if (boundingBox && m_octree) {
			collisionUpdate(e, physics, updateableDt);

			surfaceFromCollision(e, boundingBox->getBoundingBox(), transform, physics->collisions);

			if (rayCastCheck(e, physics, boundingBox->getBoundingBox(), updateableDt)) {
				//Object is moving fast, ray cast for collisions
				rayCastUpdate(e, physics, boundingBox->getBoundingBox(), transform, updateableDt);
				physics->m_oldVelocity = physics->velocity;
			}
		}

		glm::vec3 translation = (physics->m_oldVelocity + physics->velocity) * 0.5f * updateableDt;
		if (translation != glm::vec3(0.0f)) {
			transform->translate(translation);
			if (e->getName() == "player") {
				m_gameDataTracker->logDistanceWalked(translation);
			}
		}
		
		physics->m_oldVelocity = physics->velocity;
		physics->accelerationToAdd = glm::vec3(0.0f);
	}
}
