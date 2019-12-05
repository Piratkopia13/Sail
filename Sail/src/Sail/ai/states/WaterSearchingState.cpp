#include "pch.h"
#include "WaterSearchingState.h"

#include "Sail/ai/pathfinding/NodeSystem.h"
#include "Sail.h"

WaterSearchingState::WaterSearchingState(NodeSystem* nodeSystem)
	: m_nodeSystemRef(nodeSystem)
	, m_rendererWrapperRef(Application::getInstance()->getRenderWrapper())
	, m_targetPos(0.f, 0.f, 0.f)
	, m_searchingClock(4.f) {}

WaterSearchingState::~WaterSearchingState() {}

void WaterSearchingState::update(float dt, Entity* entity) {
	auto aiComp = entity->getComponent<AiComponent>();
	auto aiPos = entity->getComponent<TransformComponent>()->getMatrixWithUpdate()[3];

	m_searchingClock += dt;
	if (m_searchingClock > 2.f && aiComp->currPath.size() / 2 <= aiComp->currNodeIndex) {
		int index = -1;
		if (aiComp->currPath.size() > 0) {
			index = aiComp->currPath[aiComp->currNodeIndex].index;
		}

		m_searchingClock = glm::linearRand(-2.f, 1.f);
		if (glm::linearRand(0.f, 1.f) > 0.4f) {
			findRandomNodeIndex(index);
		} else {
			searchForWater(aiPos);
		}
		aiComp->posTarget = m_targetPos;
		aiComp->updatePath = true;
	}

	aiComp->doWalk = true;

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

	m_rendererWrapperRef->removeWaterPoint(aiPos, posOffset, negOffset);
}

void WaterSearchingState::reset(Entity* entity) {
	auto aiComp = entity->getComponent<AiComponent>();
	if (aiComp) {
		aiComp->automaticallyUpdatePath = true;
	}
}

void WaterSearchingState::init(Entity* entity) {
	m_targetPos = glm::vec3(0.f, 0.f, 0.f);
	m_searchingClock = 4.f;
	auto aiComp = entity->getComponent<AiComponent>();
	if (aiComp) {
		aiComp->automaticallyUpdatePath = false;
	}
}

void WaterSearchingState::findRandomNodeIndex(int currNodeIndex) {
	bool foundIndex = false;
	int targetNode = 0;
	while (!foundIndex) {
		int randX = (std::rand() % 20) - 10;
		int randZ = (std::rand() % 20) - 10;
		targetNode = currNodeIndex;
		targetNode += randX + randZ * m_nodeSystemRef->getXMax();
		targetNode %= (m_nodeSystemRef->getXMax() * m_nodeSystemRef->getZMax());
		targetNode = std::max(targetNode, 0);
		if (!m_nodeSystemRef->getNodes()[targetNode].blocked) {
			foundIndex = true;
		}
	}

	m_targetPos = m_nodeSystemRef->getNodes()[targetNode].position;
}

void WaterSearchingState::searchForWater(const glm::vec3& currPos) {
	glm::vec3 maxOffset(30.f, 0.f, 30.f);
	m_targetPos = m_rendererWrapperRef->getNearestWaterPosition(currPos, maxOffset);
}
