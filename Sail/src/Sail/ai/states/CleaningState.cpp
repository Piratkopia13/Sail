#include "pch.h"
#include "CleaningState.h"

#include "Sail/ai/pathfinding/NodeSystem.h"
#include "Sail.h"

CleaningState::CleaningState(NodeSystem* nodeSystem) 
	: m_nodeSystemRef(nodeSystem)
	, m_rendererWrapperRef(Application::getInstance()->getRenderWrapper())
	, m_targetPos(0.f, 0.f, 0.f)
	, m_searchingClock(4.f)
	, m_doSwitch(false) {
	m_stateTimer = 0.f;
}

CleaningState::~CleaningState() {}

void CleaningState::update(float dt, Entity* entity) {
	m_stateTimer += dt;

	auto aiComp = entity->getComponent<AiComponent>();
	auto aiPos = entity->getComponent<TransformComponent>()->getMatrixWithUpdate()[3];
	
	aiComp->doWalk = true;

	if (aiComp->currNodeIndex == aiComp->currPath.size() - 1) {
		m_doSwitch = true;
	}
}

void CleaningState::reset(Entity* entity) {
	m_doSwitch = false;
	auto aiComp = entity->getComponent<AiComponent>();
	if (aiComp) {
		aiComp->automaticallyUpdatePath = true;
	}
	m_stateTimer = 0.f;
}

void CleaningState::init(Entity* entity) {
	m_stateTimer = 0.f;
	m_doSwitch = false;
	m_targetPos = glm::vec3(0.f, 0.f, 0.f);
	m_searchingClock = 4.f;
	auto aiComp = entity->getComponent<AiComponent>();
	if (aiComp) {
		aiComp->automaticallyUpdatePath = false;
	}

	// Create a path for the bot to walk
	{
		// First try to walk to the left-bottom most index of the water
		glm::vec3 maxOffset(30.f, 0.f, 30.f);
		glm::vec3 aiPos = entity->getComponent<TransformComponent>()->getMatrixWithUpdate()[3];
		auto found = m_rendererWrapperRef->getNearestWaterPosition(aiPos, maxOffset);
		aiComp->posTarget = found.second;

		/* TODO: Should probably be improved so we don't need to copy this code */
		{
#ifdef _DEBUG_NODESYSTEM
			m_nodeSystemRef->colorPath(aiComp->currPath, m_nodeSystemRef->getMaxColourID());
#endif
			aiComp->timeTakenOnPath = 0.f;
			aiComp->reachedPathingTarget = false;

			// aiComp->posTarget is updated in each FSM state
			aiComp->lastTargetPos = aiComp->posTarget;

			auto tempPath = m_nodeSystemRef->getPath(aiPos, aiComp->posTarget);

			aiComp->currNodeIndex = 0;

			// Fix problem of always going toward closest node
			if (tempPath.size() > 1 && glm::distance(tempPath[1].position, aiPos) < glm::distance(tempPath[1].position, tempPath[0].position)) {
				aiComp->currNodeIndex += 1;
			}

			aiComp->currPath = tempPath;

			aiComp->updatePath = false;
		}

		auto nodes = m_nodeSystemRef->getNodes();
		//auto connections = m_nodeSystemRef->getConnections();
		auto currNode = aiComp->lastVisitedNode;

		// Get a direction in which there is water
		float rad = 0.5f;
		glm::vec3 dir = glm::vec3(glm::linearRand(-1.f, 1.f), 0.f, glm::linearRand(-1.f, 1.f));
		for (auto x = -1; x < 2; x++) {
			for (auto z = -1; z < 2; z++) {
				if (x != 0 && z != 0) {
					auto offset = glm::vec3(x * rad * 2.f, 0.f, z * rad * 2.f);
					auto currFound = m_rendererWrapperRef->getNearestWaterPosition(aiComp->posTarget + offset, glm::vec3(rad, 0.f, rad));
					if (currFound.first) {
						dir = offset;
						x = 5555;
						break;
					}
				}
			}
		}


		auto xSign = Utils::testSign(dir.x);
		auto zSign = Utils::testSign(dir.z);
		
		// Find a suitable path to clean an area near the found water
		auto cleaningSize = glm::ivec2((std::rand() % 3) + 1, (std::rand() % 3) + 1);
		for (auto z = 0; z < cleaningSize.y; z++) {
			int multiplier = 0;
			if (z % 2 == 0) {
				multiplier = 1;
			}
			int currIndex;
			if (aiComp->currPath.size() > 0) {
				 currIndex = aiComp->currPath[aiComp->currPath.size() - 1].index;
			} else {
				currIndex = m_nodeSystemRef->getNearestNode(aiPos).index;
			}
			std::vector<NodeSystem::Node> shortPath;
			int pathEndIndex = currIndex + xSign * cleaningSize.x * multiplier + zSign * z * m_nodeSystemRef->getXMax();
			if (currIndex > -1 && pathEndIndex > -1 && pathEndIndex < nodes.size()) {
				shortPath = m_nodeSystemRef->getPath(nodes[currIndex], nodes[pathEndIndex]);

				if (shortPath.size() > 0) {
					aiComp->currPath.insert(aiComp->currPath.end(), shortPath.begin() + 1, shortPath.end());
				}
			}
		}

#ifdef _DEBUG_NODESYSTEM
		m_nodeSystemRef->colorPath(aiComp->currPath, entity->getID() % (m_nodeSystemRef->getMaxColourID() - 1));
#endif
 	}
}

bool* CleaningState::getDoSwitch() {
	return &m_doSwitch;
}
