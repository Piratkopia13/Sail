#include "pch.h"
#include "CollisionSystem.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/MovementComponent.h"
#include "..//..//components/CollisionComponent.h"
#include "..//..//components/BoundingBoxComponent.h"
#include "..//..//components/CollisionSpheresComponent.h"
#include "..//..//Physics/Intersection.h"

CollisionSystem::CollisionSystem() {
	registerComponent<MovementComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<CollisionComponent>(true, true, true);
	registerComponent<BoundingBoxComponent>(true, true, true);
	registerComponent<CollisionSpheresComponent>(false, true, false);

	m_octree = nullptr;
}

CollisionSystem::~CollisionSystem() {
}

void CollisionSystem::provideOctree(Octree* octree) {
	m_octree = octree;
}

void CollisionSystem::update(float dt) {
	int counter = 0;
	for (auto& e : entities) {
		auto movement = e->getComponent<MovementComponent>();
		auto collision = e->getComponent<CollisionComponent>();
		auto boundingBox = e->getComponent<BoundingBoxComponent>();
		auto csc = e->getComponent<CollisionSpheresComponent>();

		if (e->getName() == "projectile") {
			if (boundingBox->getBoundingBox()->getPosition().y < -20.0f || boundingBox->getBoundingBox()->getPosition().y > 20.0f) {
				counter++;
			}
		}

		collision->collisions.clear();

		if (collision->padding < 0.0f) {
			collision->padding = glm::length(boundingBox->getBoundingBox()->getHalfSize());
		}

		float updateableDt = dt;

		if (m_octree) {
			if (!csc) {
				//Not implemented for spheres yet
				collisionUpdate(e, updateableDt);

				surfaceFromCollision(e, collision->collisions);

				if (rayCastCheck(e, *boundingBox->getBoundingBox(), updateableDt)) {
					//Object is moving fast, ray cast for collisions
					rayCastUpdate(e, *boundingBox->getBoundingBox(), updateableDt);
					movement->oldVelocity = movement->velocity;
				}
			}
		}

		movement->updateableDt = updateableDt;
	}
	//Logger::Log(std::to_string(counter));
}

const bool CollisionSystem::collisionUpdate(Entity* e, const float dt) {
	//Update collision data
	std::vector<Octree::CollisionInfo> collisions;
	CollisionComponent* collision = e->getComponent<CollisionComponent>();

	bool hasSpheres = e->hasComponent<CollisionSpheresComponent>();
	if (hasSpheres) {
		assert(false); //Not implemented
	}
	else {
		m_octree->getCollisions(e, &collisions, collision->doSimpleCollisions);
	}

	return handleCollisions(e, collisions, dt);
}

const bool CollisionSystem::handleCollisions(Entity* e, std::vector<Octree::CollisionInfo>& collisions, const float dt) {
	bool returnValue = false;

	MovementComponent* movement = e->getComponent<MovementComponent>();
	CollisionComponent* collision = e->getComponent<CollisionComponent>();
	BoundingBox* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();

	collision->onGround = false;

	const size_t collisionCount = collisions.size();

	if (collisionCount > 0) {
		std::vector<int> groundIndices;
		glm::vec3 sumVec(0.0f);
		std::vector<Octree::CollisionInfo> trueCollisions;

		//Get the combined normals and detect "true" collisions
		for (size_t i = 0; i < collisionCount; i++) {
			Octree::CollisionInfo& collisionInfo_i = collisions[i];

			//Find true collisions
			if (collisionInfo_i.shape->isTrueCollision(boundingBox)) {

				sumVec += collisionInfo_i.normal;

				collisionInfo_i.intersectionPosition = collisionInfo_i.shape->getIntersectionPosition(boundingBox);

				//Add collision to current collisions for collisionComponent
				collision->collisions.push_back(collisionInfo_i);

				//Add collision to true collisions
				trueCollisions.push_back(collisionInfo_i);

				returnValue = true;
			}
		}

		//Loop through true collisions and handle them
		const size_t trueCollisionCount = trueCollisions.size();
		for (size_t i = 0; i < trueCollisionCount; i++) {
			const Octree::CollisionInfo& collisionInfo_i = trueCollisions[i];

			//Save ground collisions
			if (collisionInfo_i.normal.y > 0.7f) {
				collision->onGround = true;
				bool newGround = true;
				for (size_t j = 0; j < groundIndices.size(); j++) {
					if (collisionInfo_i.normal == trueCollisions[groundIndices[j]].normal) {
						newGround = false;
					}
				}
				if (newGround) {
					//Save collision for friction calculation
					groundIndices.push_back(i);
				}
			}

			//----Velocity changes from collisions----

			//Stop movement towards triangle
			float projectionSize = glm::dot(movement->velocity, -collisionInfo_i.normal);

			if (projectionSize > 0.0f) { //Is pushing against wall
				movement->velocity += collisionInfo_i.normal * (projectionSize * (1.0f + collision->bounciness)); //Limit movement towards wall
			}


			//Tight angle corner special case
			const float dotProduct = glm::dot(collisionInfo_i.normal, glm::normalize(sumVec));
			if (dotProduct < 0.7072f && dotProduct > 0.0f) { //Colliding in a tight angle corner
				glm::vec3 normalToNormal = sumVec - glm::dot(sumVec, collisionInfo_i.normal) * collisionInfo_i.normal;
				normalToNormal = glm::normalize(normalToNormal);

				//Stop movement towards corner
				projectionSize = glm::dot(movement->velocity, -normalToNormal);

				if (projectionSize > 0.0f) {
					movement->velocity += normalToNormal * projectionSize * (1.0f + collision->bounciness);
				}
			}
			//----------------------------------------
		}

		//----Drag----
		if (collision->onGround) { //Ground drag
			size_t nrOfGroundCollisions = groundIndices.size();
			for (size_t i = 0; i < nrOfGroundCollisions; i++) {
				const Octree::CollisionInfo& collisionInfo_ground_i = trueCollisions[groundIndices[i]];
				const glm::vec3 velAlongPlane = movement->velocity - collisionInfo_ground_i.normal * glm::dot(collisionInfo_ground_i.normal, movement->velocity);
				const float sizeOfVel = glm::length(velAlongPlane);
				if (sizeOfVel > 0.0f) {
					const float slowdown = glm::min((collision->drag / nrOfGroundCollisions) * dt, sizeOfVel);
					movement->velocity -= slowdown * glm::normalize(velAlongPlane);
				}
			}
		}
		//------------
	}

	return returnValue;
}

const bool CollisionSystem::rayCastCheck(Entity* e, const BoundingBox& boundingBox, float& dt) const {
	MovementComponent* movementComp = e->getComponent<MovementComponent>();

	if (glm::abs(movementComp->velocity.x * dt) > glm::abs(boundingBox.getHalfSize().x)
		|| glm::abs(movementComp->velocity.y * dt) > glm::abs(boundingBox.getHalfSize().y)
		|| glm::abs(movementComp->velocity.z * dt) > glm::abs(boundingBox.getHalfSize().z)) {
		//Object is moving at a speed that risks missing collisions
		return true;
	}
	return false;
}

void CollisionSystem::rayCastUpdate(Entity* e, BoundingBox& boundingBox, float& dt) {
	MovementComponent* movement = e->getComponent<MovementComponent>();
	TransformComponent* transform = e->getComponent<TransformComponent>();
	CollisionComponent* collision = e->getComponent<CollisionComponent>();

	const float velocityAmp = glm::length(movement->velocity) * dt;

	//Ray cast to find upcoming collisions, use padding for "swept sphere"
	Octree::RayIntersectionInfo intersectionInfo;
	m_octree->getRayIntersection(boundingBox.getPosition(), glm::normalize(movement->velocity), &intersectionInfo, e, collision->padding, collision->doSimpleCollisions);

	if (intersectionInfo.closestHit <= velocityAmp && intersectionInfo.closestHit >= 0.0f) { //Found upcoming collision
		//Calculate new dt
		float newDt = ((intersectionInfo.closestHit) / velocityAmp) * dt;

		//Move untill first overlap
		boundingBox.setPosition(boundingBox.getPosition() + movement->velocity * newDt);
		transform->translate(movement->velocity * newDt);

		dt -= newDt;

		//Collision update
		if (handleCollisions(e, intersectionInfo.info, 0.0f)) {
			surfaceFromCollision(e, intersectionInfo.info);
		}
		else {
			//Move back 
			const glm::vec3 normalizedVel = glm::normalize(movement->velocity);
			boundingBox.setPosition(boundingBox.getPosition() - normalizedVel * collision->padding * 0.5f);
			transform->translate(-normalizedVel * collision->padding * 0.5f);

			//Step forward to find collision
			stepToFindMissedCollision(e, boundingBox, intersectionInfo.info, collision->padding * 2.0f);
		}
		rayCastUpdate(e, boundingBox, dt);
	}
}

void CollisionSystem::stepToFindMissedCollision(Entity* e, BoundingBox& boundingBox, std::vector<Octree::CollisionInfo>& collisions, float distance) {
	MovementComponent* movement = e->getComponent<MovementComponent>();
	TransformComponent* transform = e->getComponent<TransformComponent>();
	CollisionComponent* collision = e->getComponent<CollisionComponent>();

	const int split = 5;

	const glm::vec3 normalizedVel = glm::normalize(movement->velocity);
	const glm::vec3 distancePerStep = (distance / (float)split) * normalizedVel;
	
	for (int i = 0; i < split; i++) {
		boundingBox.setPosition(boundingBox.getPosition() + distancePerStep);
		transform->translate(distancePerStep);

		if (handleCollisions(e, collisions, 0.0f)) {
			surfaceFromCollision(e, collisions);
			i = split;
		}
	}
}

void CollisionSystem::surfaceFromCollision(Entity* e, std::vector<Octree::CollisionInfo>& collisions) {
	glm::vec3 distance(0.0f);
	auto bb = e->getComponent<BoundingBoxComponent>();
	auto movement = e->getComponent<MovementComponent>();
	auto transform = e->getComponent<TransformComponent>();

	const size_t count = collisions.size();
	for (size_t i = 0; i < count; i++) {
		const Octree::CollisionInfo& collisionInfo_i = collisions[i];
		glm::vec3 intersectionAxis;
		float intersectionDepth;

		if (collisionInfo_i.shape->getIntersectionDepthAndAxis(bb->getBoundingBox(), &intersectionAxis, &intersectionDepth)) {
			if (glm::abs(glm::dot(intersectionAxis, collisionInfo_i.normal)) > 0.98f) { //True collision
				bb->getBoundingBox()->setPosition(bb->getBoundingBox()->getPosition() + collisionInfo_i.normal * intersectionDepth);
				distance += collisionInfo_i.normal * intersectionDepth;
			}
		}
	}
	transform->translate(distance);
}