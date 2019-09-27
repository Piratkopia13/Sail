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

void PhysicSystem::collisionUpdate(Entity* e, float dt) {
	TransformComponent* transform = e->getComponent<TransformComponent>();
	PhysicsComponent* physics = e->getComponent<PhysicsComponent>();

	physics->onGround = false;
	std::vector<int> groundIndices;

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
				if (physics->collisions[i].normal.y > 0.7f) {
					physics->onGround = true;
					bool newGround = true;
					for (unsigned int j = 0; j < groundIndices.size(); j++) {
						if (physics->collisions[i].normal == physics->collisions[groundIndices[j]].normal) {
							newGround = false;
						}
					}
					if (newGround) {
						//Save collision for friction calculation
						groundIndices.push_back(i);
					}
				}

				//Stop movement towards triangle
				float projectionSize = glm::dot(physics->velocity, -physics->collisions[i].normal);

				if (projectionSize > 0.0f) { //Is pushing against wall
					physics->velocity += physics->collisions[i].normal * projectionSize; //Limit movement towards wall
				}

				//Tight angle corner special case
				float dotProduct = glm::dot(physics->collisions[i].normal, glm::normalize(sumVec));
				if (dotProduct < 0.7072f && dotProduct > 0.0f) { //Colliding in a tight angle corner
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


	//----Drag----
	if (physics->onGround) { //Ground drag
		unsigned int nrOfGroundCollisions = groundIndices.size();
		for (unsigned int i = 0; i < nrOfGroundCollisions; i++) {
			glm::vec3 velAlongPlane = physics->velocity - physics->collisions[groundIndices[i]].normal * glm::dot(physics->collisions[groundIndices[i]].normal, physics->velocity);
			float sizeOfVel = glm::length(velAlongPlane);
			if (sizeOfVel > 0.0f) {
				float slowdown = glm::min((physics->drag / nrOfGroundCollisions) * dt, sizeOfVel);
				physics->velocity -= slowdown * glm::normalize(velAlongPlane);
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
}

void PhysicSystem::rayCastUpdate(Entity* e, float& dt) {
	PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
	TransformComponent* transform = e->getComponent<TransformComponent>();
	BoundingBox* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();

	if (std::abs(physics->velocity.x * dt) > std::abs(boundingBox->getHalfSize().x)
		|| std::abs(physics->velocity.y * dt) > std::abs(boundingBox->getHalfSize().y)
		|| std::abs(physics->velocity.z * dt) > std::abs(boundingBox->getHalfSize().z)) {

		//Object is moving at a speed that risks missing collisions
		//Ray cast to find upcoming collisions
		Octree::RayIntersectionInfo intersectionInfo;
		m_octree->getRayIntersection(boundingBox->getPosition(), physics->velocity, &intersectionInfo, e);
		float velocityAmp = glm::length(physics->velocity) * dt;
		glm::vec3 normalizedVel = glm::normalize(physics->velocity);

		//Find the distance to the front wall
		float xMovementAllowed = boundingBox->getHalfSize().x / glm::max(std::abs(normalizedVel.x), 0.01f);
		float yMovementAllowed = boundingBox->getHalfSize().y / glm::max(std::abs(normalizedVel.y), 0.01f);
		float zMovementAllowed = boundingBox->getHalfSize().z / glm::max(std::abs(normalizedVel.z), 0.01f);
		float maxMovementAllowed = glm::min(glm::min(xMovementAllowed, yMovementAllowed), zMovementAllowed) * 0.95f;

		if ((intersectionInfo.closestHit - maxMovementAllowed) <= velocityAmp && intersectionInfo.closestHit > 0.0f) { //Found upcoming collision
			//Calculate new dt
			float newDt = ((intersectionInfo.closestHit - maxMovementAllowed) / velocityAmp) * dt;

			if (newDt > 0.0001f) {
				//Move untill first overlap
				boundingBox->setPosition(boundingBox->getPosition() + physics->velocity * newDt);
				transform->translate(physics->velocity * newDt);

				dt -= newDt;

				//Collision update
				collisionUpdate(e, dt);

				physics->m_oldVelocity = physics->velocity;

				rayCastUpdate(e, dt);
			}
		}
	}
}

void PhysicSystem::update(float dt) {
	for (auto& e : entities) {
		PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
		TransformComponent* transform = e->getComponent<TransformComponent>();
		BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();

		physics->velocity += physics->constantAcceleration * dt;
		physics->velocity += physics->accelerationToAdd * dt;

		if (boundingBox) {
			rayCastUpdate(e, dt);
			collisionUpdate(e, dt);
		}

		transform->rotate(physics->constantRotation * dt);
		transform->translate((physics->m_oldVelocity + physics->velocity) * 0.5f * dt);
		physics->m_oldVelocity = physics->velocity;
		physics->accelerationToAdd = glm::vec3(0.0f);
	}
}
