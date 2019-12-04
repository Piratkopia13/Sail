#include "pch.h"
#include "CleaningState.h"

#include "Sail/ai/pathfinding/NodeSystem.h"
#include "Sail.h"

CleaningState::CleaningState(NodeSystem* nodeSystem) 
	: m_nodeSystem(nodeSystem)
	, m_targetNode(0)
	, m_searchingClock(4.f) {}

CleaningState::~CleaningState() {}

void CleaningState::update(float dt, Entity* entity) {
	auto aiComp = entity->getComponent<AiComponent>();
	auto aiPos = entity->getComponent<TransformComponent>()->getMatrixWithUpdate()[3];

	m_searchingClock += dt;
	if (m_searchingClock > 5.f) {
		int index = -1;
		if (aiComp->currPath.size() > 0) {
			index = aiComp->currPath[aiComp->currNodeIndex].index;
		}

		findRandomNodeIndex(index);
		m_searchingClock = 0.f;
		aiComp->posTarget = m_nodeSystem->getNodes()[m_targetNode].position;
		aiComp->updatePath = true;
	}

	aiComp->doWalk = true;

	/*if (m_targetNode != -1 && aiComp->timeTakenOnPath > 2.f) {
		aiComp->posTarget = m_nodeSystem->getNodes()[m_targetNode].position;
		aiComp->doWalk = true;
		aiComp->updatePath = true;
		aiComp->reachedPathingTarget = false;
	}*/

	auto moveC = entity->getComponent<MovementComponent>();
	glm::ivec3 posOffset, negOffset;
	posOffset = glm::ivec3(1, 0, 1);
	negOffset = glm::ivec3(-1, 0, -1);
	if (moveC && glm::length2(moveC->velocity) > 0.f) {
		auto velNorm = glm::normalize(moveC->velocity);
		if (velNorm.x > 0) {
			posOffset.x += int(velNorm.x * 3.f) + 1;
		} else {
			negOffset.x += int(velNorm.x * 3.f) - 1;
		}

		if (velNorm.z > 0) {
			posOffset.z += int(velNorm.z * 3.f) + 1;
		} else {
			negOffset.z += int(velNorm.z * 3.f) - 1;
		}
	}

	Application::getInstance()->getRenderWrapper()->removeWaterPoint(aiPos, posOffset, negOffset);
}

void CleaningState::reset(Entity* entity) {
	auto aiComp = entity->getComponent<AiComponent>();
	if (aiComp) {
		aiComp->automaticallyUpdatePath = true;
	}
}

void CleaningState::init(Entity* entity) {
	m_targetNode = 0;
	m_searchingClock = 4.f;
	auto aiComp = entity->getComponent<AiComponent>();
	if (aiComp) {
		aiComp->automaticallyUpdatePath = false;
	}
}

void CleaningState::findRandomNodeIndex(int currNodeIndex) {
	bool foundIndex = false;
	while (!foundIndex) {
		int randX = (std::rand() % 20) - 10;
		int randZ = (std::rand() % 20) - 10;
		m_targetNode = currNodeIndex;
		m_targetNode += randX + randZ * m_nodeSystem->getXMax();
		m_targetNode %= (m_nodeSystem->getXMax() * m_nodeSystem->getZMax());
		m_targetNode = std::max(m_targetNode, 0);
		if (!m_nodeSystem->getNodes()[m_targetNode].blocked) {
			foundIndex = true;
		}
	}
}
