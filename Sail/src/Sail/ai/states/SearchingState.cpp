#include "pch.h"
#include "SearchingState.h"

#include "Sail/ai/pathfinding/NodeSystem.h"
#include "Sail/entities/components/AiComponent.h"
#include "Sail/entities/components/TransformComponent.h"

#include "Sail/utils/Utils.h"

SearchingState::SearchingState(NodeSystem* nodeSystem) : m_nodeSystem(nodeSystem) {
	m_distToHost = INFINITY;
	m_targetNode = 0;
}

SearchingState::~SearchingState() {
}

void SearchingState::update(float dt, Entity* entity) {
	
	auto aiComp = entity->getComponent<AiComponent>();
	auto aiPos = entity->getComponent<TransformComponent>()->getMatrixWithUpdate()[3];

	static float clock = 0;
	clock += dt;
	// Change condition to not use a clock to get new directions
	if (clock > 5) {
		findRandomNodeIndex();
		// Save this comment to later track how to fine the way.
		//Logger::Log("node: " + std::to_string(m_targetNode) + " clock: " + std::to_string(clock));
		clock = 0;
	}
	if (aiComp->entityTarget != nullptr) {
		auto enemyPos = aiComp->entityTarget->getComponent<TransformComponent>()->getMatrixWithUpdate()[3];
		m_distToHost = glm::distance2(enemyPos, aiPos);
		

		// This is can be rewritten in a better way
		aiComp->posTarget = m_nodeSystem->getNodes()[m_targetNode].position;
		aiComp->doWalk = true;

		if (m_targetNode != -1 && aiComp->timeTakenOnPath > 2.f) {
			aiComp->posTarget = m_nodeSystem->getNodes()[m_targetNode].position;
			aiComp->doWalk = true;
			aiComp->updatePath = true;
			aiComp->reachedPathingTarget = false;
		}
	}
	

}

void SearchingState::reset(Entity* entity) {
}

void SearchingState::init(Entity* entity) {
}

float* SearchingState::getDistToHost() {
	return &m_distToHost;
}

void SearchingState::findRandomNodeIndex() {

	bool foundIndex = false;
	int nrOfNodes = 0;

	while (!foundIndex) {
		nrOfNodes = m_nodeSystem->getNodes().size();
		if (nrOfNodes < 1) {
			return;
		}
		// Get a random node from the range of nodes
		m_targetNode += 1949;
		m_targetNode = m_targetNode % nrOfNodes;

		if (!m_nodeSystem->getNodes().at(m_targetNode).blocked) {
			foundIndex = true;
			return;
		}
	}
}
