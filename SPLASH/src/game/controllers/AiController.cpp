#include "pch.h"
#include "AiController.h"
#include "Sail.h"

AiController::AiController()
	: m_movementSpeed(5.f)
	, m_timeBetweenPathUpdate(1.0f /*seconds*/)
	, m_timeTaken(0.f)
	, m_currNodeIndex(-1)
	, m_lastVisitedNode(NodeSystem::Node(glm::vec3(10000.f, 10000.f, 10000.f), false, 2381831))
	, m_controlledEntity(nullptr)
	, m_physComp(nullptr)
	, m_transComp(nullptr)
	, m_entityTarget(nullptr)
	, m_lastTargetPos(glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX))
	, m_target(glm::vec3(0.f, 0.f, 0.f))
	, m_targetReachedThreshold(0.3f)
	, m_reachedTarget(true)
	, m_maxSteeringForce(0.3f)
	, m_mass(1.f)
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

	if (m_currPath.empty()) {
		return;
	}

	if ( m_reachedTarget && m_currNodeIndex < m_currPath.size() - 1 ) {
		m_currNodeIndex++;
		m_target = m_currPath[m_currNodeIndex].position;
		m_reachedTarget = false;
	}
	
	if ( !m_reachedTarget ) {
		// Check if the distance between target and ai is low enough
		if ( glm::distance(m_transComp->getTranslation(), m_currPath[m_currNodeIndex].position) < m_targetReachedThreshold ) {
			m_lastVisitedNode = m_currPath[m_currNodeIndex];
			m_reachedTarget = true;
		}
		else {
			glm::vec3 desiredDir = m_currPath[m_currNodeIndex].position - m_transComp->getTranslation();
			if ( desiredDir == glm::vec3(0.f) ) {
				desiredDir = glm::vec3(1.0f, 0.f, 0.f);
			}
			desiredDir = glm::normalize(desiredDir);
			glm::vec3 desiredVel = desiredDir * m_movementSpeed;
			glm::vec3 steering = ( desiredVel - m_physComp->velocity );
			float steeringMag = ( glm::length(steering) * m_mass );
			steering *= m_maxSteeringForce / (steeringMag != 0.f ? steeringMag : 1.f);
			m_physComp->velocity += steering;
			// Float used to clamp the magnitude of the velocity between 0 and m_movementSpeed
			float velMagClamper = glm::length(m_physComp->velocity);
			velMagClamper = ( velMagClamper > m_movementSpeed ) ? m_movementSpeed / velMagClamper : 1.0f;
			m_physComp->velocity = m_physComp->velocity * velMagClamper;
			//Logger::Log("Velocity: " + Utils::toStr(m_physComp->velocity));
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

		m_currNodeIndex = 0;

		// Fix problem of always going toward closest node
		if ( tempPath.size() > 1 && glm::distance(tempPath[1].position, m_transComp->getTranslation()) < glm::distance(tempPath[1].position, tempPath[0].position) ) {
			m_currNodeIndex += 1;
		}

		m_currPath = tempPath;

		/*std::string toPrint = "[";
		for ( auto p : m_currPath ) {
			toPrint += (std::to_string(p.index) + ", ");
		}
		toPrint += "]";
		Logger::Log(toPrint);*/
	}
}
