#include "pch.h"
#include "FleeingState.h"

#include "Sail/ai/pathfinding/NodeSystem.h"
#include "Sail/entities/components/AiComponent.h"
#include "Sail/entities/components/TransformComponent.h"

FleeingState::FleeingState(NodeSystem* nodeSystem)
	: m_nodeSystem(nodeSystem) {}

FleeingState::~FleeingState() {}

void FleeingState::update(float dt, Entity* entity) {
	auto aiComp = entity->getComponent<AiComponent>();
	auto aiPos = entity->getComponent<TransformComponent>()->getMatrixWithUpdate()[3];
	int bestNodeIndex = findBestNode(aiPos, aiComp->entityTarget);

	if ( bestNodeIndex != -1 && aiComp->timeTakenOnPath > 2.f) {
		aiComp->posTarget = m_nodeSystem->getNodes()[bestNodeIndex].position;
		aiComp->doWalk = true;
		aiComp->updatePath = true;
		aiComp->reachedPathingTarget = false;
	}

}

void FleeingState::reset(Entity* entity) {}

void FleeingState::init(Entity* entity) {
	auto aiComp = entity->getComponent<AiComponent>();
	aiComp->doWalk = false;
	aiComp->updatePath = false;
	aiComp->reachedPathingTarget = true;
	aiComp->timeTakenOnPath = 3.f;
}

int FleeingState::findBestNode(const glm::vec3& aiPos, Entity* enemy) {
	float farthestDist2 = -9999999.0f;
	int bestNodeIndex = -1;
	int nodeIndex = 0;

	for ( auto node : m_nodeSystem->getNodes() ) {

		if ( !node.blocked ) {
			float enemyDist = 0.f;
			if ( enemy != nullptr ) {
				enemyDist = glm::distance2(glm::vec3(enemy->getComponent<TransformComponent>()->getMatrixWithUpdate()[3]), node.position);
			}
			float aiDist = glm::distance2(aiPos, node.position) / 8.f;
			float testDist = enemyDist - aiDist;
			// Good to be close to ai, bad to be close to enemy
			if ( testDist > farthestDist2 ) {
				farthestDist2 = testDist;
				bestNodeIndex = nodeIndex;
			}
		}

		nodeIndex++;
	}

	return bestNodeIndex;
}
