#include "pch.h"
#include "AiController.h"
#include "Sail.h"

AiController::AiController()
	: m_controlledEntity(nullptr)
	, m_entityTarget(nullptr)
	, m_reachedTarget(true)
	, m_lastTargetPos(glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX))
	, m_target(glm::vec3(0.f, 0.f, 0.f))
	, m_movementSpeed(2.f)
	, m_timeBetweenPathUpdate(1000.f/1000.f)
	, m_timeTaken(0.f)
	, m_currNodeIndex(-1)
	, m_lastVisitedNode(NodeSystem::Node(glm::vec3(10000.f, 10000.f, 10000.f), 2381831))
{
}

AiController::AiController(std::shared_ptr<Entity> toControl) 
	: AiController() 
{
	m_controlledEntity = toControl;
	m_transComp = m_controlledEntity->getComponent<TransformComponent>();
	m_physComp = m_controlledEntity->getComponent<PhysicsComponent>();
}

AiController::~AiController() {}

void AiController::update(float dt) {
	if ( m_entityTarget != nullptr ) {
		m_timeTaken += dt;
		if ( m_timeTaken > m_timeBetweenPathUpdate ) {
			m_timeTaken = 0.f;
			moveTo(m_entityTarget->getComponent<TransformComponent>()->getTranslation());
		}
	}

	if ( m_reachedTarget && m_currNodeIndex < m_currPath.size() - 1 ) {
		m_currNodeIndex++;
		m_target = m_currPath[m_currNodeIndex].position;
		m_reachedTarget = false;
	}
	
	if (!m_reachedTarget) {
		auto dir = m_currPath[m_currNodeIndex].position - m_transComp->getTranslation();
		if ( dir == glm::vec3(0.f) ) {
			dir = glm::vec3(1.0f, 0.f, 0.f);
		}
		dir = glm::normalize(dir);
		m_physComp->velocity = dir * m_movementSpeed;
		if (glm::distance(m_transComp->getTranslation(), m_currPath[m_currNodeIndex].position) < 1.f) {
			m_lastVisitedNode = m_currPath[m_currNodeIndex];
			m_reachedTarget = true;
		}
	}
	else {
		m_physComp->velocity = glm::vec3(0.f);
	}
}

void AiController::moveTo(glm::vec3 point) {
	m_target = point;
	m_reachedTarget = false;

	updatePath();
}

void AiController::chaseEntity(Entity* toChase) {
	m_entityTarget = toChase;
}

void AiController::setEntity(std::shared_ptr<Entity> toControl) {
	m_controlledEntity = toControl;
	m_transComp = m_controlledEntity->getComponent<TransformComponent>();
	m_physComp = m_controlledEntity->getComponent<PhysicsComponent>();
}

std::shared_ptr<Entity> AiController::getEntity() {
	return m_controlledEntity;
}

Entity* AiController::getTargetEntity() {
	return m_entityTarget;
}


/*
	PRIVATE
*/
void AiController::updatePath() {
	if ( m_target != m_lastTargetPos ) {

		m_lastTargetPos = m_target;

		auto tempPath = Application::getInstance()->getNodeSystem()->getPath(m_transComp->getTranslation(), m_target);

		auto lastCurrNodeIndex = m_currNodeIndex;
		m_currNodeIndex = 0;

		// Fix problem of always going toward closest node
		if ( tempPath.size() > 1 ) {
			if ( glm::distance(tempPath[1].position, m_transComp->getTranslation()) < glm::distance(tempPath[1].position, tempPath[0].position) ) {
				m_currNodeIndex += 1;
			}
		}

		m_currPath = tempPath;

		std::string toPrint = "[";
		for ( auto p : m_currPath ) {
			toPrint += (std::to_string(p.index) + ", ");
		}
		toPrint += "]";
		Logger::Log(toPrint);
	}
}
