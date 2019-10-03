#include "pch.h"
#include "AttackingState.h"

#include "Sail/entities/components/AiComponent.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/GunComponent.h"
#include "Sail/entities/components/BoundingBoxComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "../Physics/Octree.h"
#include "../Physics/Intersection.h"

AttackingState::AttackingState() {
	m_distToHost = INFINITY;
}

AttackingState::~AttackingState() {}

void AttackingState::update(float dt, Entity* entity) {
	auto aiComp = entity->getComponent<AiComponent>();
	auto transComp = entity->getComponent<TransformComponent>();
	auto gunComp = entity->getComponent<GunComponent>();

	for ( auto& e : entity->getChildEntities() ) {
		auto candle = e->getComponent<CandleComponent>();
		if ( candle && !candle->getIsAlive() ) {
			return;
		}
	}

	if ( aiComp->entityTarget != nullptr ) {
		aiComp->posTarget = aiComp->entityTarget->getComponent<TransformComponent>()->getMatrix()[3];

		m_distToHost = glm::distance2(aiComp->posTarget, glm::vec3(transComp->getMatrix()[3]));

		// Approx AI gun pos
		auto gunPos = transComp->getTranslation() + glm::vec3(0.f, 0.9f, 0.f);

		// Candle pos
		const glm::vec3& candlePos = aiComp->entityTarget->getComponent<TransformComponent>()->getMatrix()[3];

		// Aim slightly higher to account for gravity (removed temporarily)
		const glm::vec3& enemyPos = candlePos;// +glm::vec3(0, 0.3f, 0);

		auto fireDir = enemyPos - gunPos;
		fireDir = glm::normalize(fireDir);

		float hitDist = Intersection::RayWithAabb(gunPos, fireDir, *aiComp->entityTarget->getComponent<BoundingBoxComponent>()->getBoundingBox());

		bool canSeeTarget = glm::abs(hitDist - glm::distance(enemyPos, gunPos)) < 0.3f;
		// Stop if the target is within 5 meters and if the target is visible
		if ( glm::distance(glm::vec3(transComp->getMatrix()[3]), aiComp->posTarget) > 5.f || !canSeeTarget ) {
			if ( !aiComp->doWalk ) {
				aiComp->updatePath = true;
			}
			aiComp->doWalk = true;
		} else {
			aiComp->doWalk = false;
			aiComp->currPath.clear();
			aiComp->currNodeIndex = 0;
		}

		if ( canSeeTarget ) {
			entityTargetFunc(aiComp, transComp, gunComp, fireDir, gunPos, hitDist);
		}
	}
}

void AttackingState::reset(Entity* entity) {
	// For future use, needs to be implemented in each state
}

void AttackingState::init(Entity* entity) {
	// For future use, needs to be implemented in each state
}

float* AttackingState::getDistToHost() {
	return &m_distToHost;
}

void AttackingState::entityTargetFunc(AiComponent* aiComp, TransformComponent* transComp, GunComponent* gunComp, const glm::vec3& fireDir, const glm::vec3& gunPos, const float hitDist) {

	// If the target is within 7 units and if there is nothing between the target and the ai and don't shoot unless the candle is lit up
	if ( hitDist < 7.f && aiComp->entityTarget->getComponent<CandleComponent>()->getIsAlive() ) {
		gunComp->setFiring(gunPos + fireDir, fireDir);

		float yaw = 0.f;		
		float piTwo = 6.28318f;
		if ( fireDir.z < 0.f ) {
			yaw = glm::atan(fireDir.x / fireDir.z) + 1.5707f;
		} else {
			yaw = glm::atan(fireDir.x / fireDir.z) - 1.5707f;
		}
		// To make sure that 0.f < yaw < 2 * pi
		if ( yaw < 0 ) {
			yaw = piTwo + yaw;
		}
		yaw = std::fmod(yaw, piTwo);
		transComp->setRotations(0.f, yaw, 0.f);
	}
}
