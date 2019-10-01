#include "pch.h"
#include "AttackingState.h"

#include "Sail/entities/components/AiComponent.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/GunComponent.h"
#include "Sail/entities/components/BoundingBoxComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "../Physics/Octree.h"
#include "../Physics/Intersection.h"

AttackingState::AttackingState() {}

AttackingState::~AttackingState() {}

void AttackingState::update(float dt, Entity* entity) {
	// TODO: Implement this stuff
}

void AttackingState::reset() {}

void AttackingState::init() {}

void AttackingState::entityTargetFunc(AiComponent* aiComp, TransformComponent* transComp, GunComponent* gunComp, Octree* octree) {
	if ( aiComp->entityTarget != nullptr ) {

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
				octree->getRayIntersection(gunPos + fireDir /*In order to (hopefully) miss itself*/, fireDir, &rayHitInfo);
				if ( hitDist < 7.f && glm::abs(hitDist - glm::distance(enemyPos, gunPos)) < 1.f ) {
					gunComp->setFiring(gunPos += fireDir, fireDir);

					if ( fireDir.z < 0.f ) {
						transComp->setRotations(0.f, glm::atan(fireDir.x / fireDir.z) + 1.5707f, 0.f);
					} else {
						transComp->setRotations(0.f, glm::atan(fireDir.x / fireDir.z) - 1.5707f, 0.f);
					}
				}
			}
		}
	}
}

void AttackingState::updatePath(AiComponent* aiComp, TransformComponent* transComp, NodeSystem* nodeSystem) {
	if ( aiComp->posTarget != aiComp->lastTargetPos ) {

		aiComp->lastTargetPos = aiComp->posTarget;

		auto tempPath = nodeSystem->getPath(transComp->getTranslation(), aiComp->posTarget);

		aiComp->currNodeIndex = 0;

		// Fix problem of always going toward closest node
		if ( tempPath.size() > 1 && glm::distance(tempPath[1].position, transComp->getTranslation()) < glm::distance(tempPath[1].position, tempPath[0].position) ) {
			aiComp->currNodeIndex += 1;
		}

		aiComp->currPath = tempPath;
	}
}