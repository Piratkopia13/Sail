#include "pch.h"
#include "AttackingState.h"

#include "Sail/entities/components/AiComponent.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/GunComponent.h"
#include "Sail/entities/components/BoundingBoxComponent.h"
#include "Sail/entities/components/PhysicsComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "../Physics/Octree.h"
#include "../Physics/Intersection.h"

AttackingState::AttackingState(Octree* octree)
	: m_octree(octree)
{}

AttackingState::~AttackingState() {}

void AttackingState::update(float dt, Entity* entity) {
	auto aiComp = entity->getComponent<AiComponent>();
	auto transComp = entity->getComponent<TransformComponent>();
	auto physComp = entity->getComponent<PhysicsComponent>();
	auto gunComp = entity->getComponent<GunComponent>();

	for ( auto& e : entity->getChildEntities() ) {
		auto candle = e->getComponent<CandleComponent>();
		if ( candle ) {
			if ( !candle->getIsAlive() ) {
				return;
			}
		}
	}

	if ( aiComp->entityTarget != nullptr ) {
		aiComp->posTarget = aiComp->entityTarget->getComponent<TransformComponent>()->getMatrix()[3];
		// Stop if the target is within 5 meters (TODO: should also be to stop if the target is visible)
		if ( glm::distance(glm::vec3(transComp->getMatrix()[3]), aiComp->posTarget) > 5.f ) {
			if ( !aiComp->doWalk ) {
				aiComp->updatePath = true;
			}
			aiComp->doWalk = true;
		} else {
			aiComp->doWalk = false;
			aiComp->currPath.clear();
			aiComp->currNodeIndex = 0;
		}
		entityTargetFunc(aiComp, transComp, gunComp);
	} else {
		// Do transition to other state
	}
}

void AttackingState::reset() {}

void AttackingState::init() {}

void AttackingState::entityTargetFunc(AiComponent* aiComp, TransformComponent* transComp, GunComponent* gunComp) {
	if ( gunComp != nullptr ) {

		// Don't shoot unless the candle is lit up
		if ( aiComp->entityTarget->getComponent<CandleComponent>()->getIsAlive() ) {

			// Approx AI gun pos
			auto gunPos = transComp->getTranslation() + glm::vec3(0.f, 0.9f, 0.f);

			// Candle pos
			const glm::vec3& candlePos = aiComp->entityTarget->getComponent<TransformComponent>()->getMatrix()[3];

			// Aim slightly higher to account for gravity
			const glm::vec3& enemyPos = candlePos + glm::vec3(0, 0.3f, 0);

			auto fireDir = enemyPos - gunPos;
			fireDir = glm::normalize(fireDir);

			float hitDist = Intersection::rayWithAabb(gunPos, fireDir, *aiComp->entityTarget->getComponent<BoundingBoxComponent>()->getBoundingBox());

			Octree::RayIntersectionInfo rayHitInfo;
			m_octree->getRayIntersection(gunPos + fireDir /*In order to (hopefully) miss itself*/, fireDir, &rayHitInfo);
			if ( hitDist < 7.f && glm::abs(hitDist - glm::distance(enemyPos, gunPos)) < 1.f ) {
				//gunComp->setFiring(gunPos += fireDir, fireDir);

				if ( fireDir.z < 0.f ) {
					transComp->setRotations(0.f, glm::atan(fireDir.x / fireDir.z) + 1.5707f, 0.f);
				} else {
					transComp->setRotations(0.f, glm::atan(fireDir.x / fireDir.z) - 1.5707f, 0.f);
				}
			}
		}
	}
}
