#include "pch.h"
#include "AiController.h"
#include "Sail.h"

AiController::AiController() {
	m_controlledEntity = nullptr;
	m_entityTarget = nullptr;
	m_target = glm::vec3(0.f, 0.f, 0.f);
}

AiController::AiController(std::shared_ptr<Entity> toControl) 
	: AiController() 
{
	m_controlledEntity = toControl;
	m_transComp = m_controlledEntity->getComponent<TransformComponent>();
	m_physComp = m_controlledEntity->getComponent<PhysicsComponent>();
}

AiController::~AiController() {}

void AiController::update() {
	if ( m_entityTarget != nullptr ) {
		m_target = m_entityTarget->getComponent<TransformComponent>()->getTranslation();
		m_reachedTarget = false;
	}
	
	if (!m_reachedTarget) {
		auto dir = glm::normalize(m_target - m_transComp->getTranslation());
		m_physComp->velocity = dir * m_movementSpeed;
		m_transComp->setForward(dir);
		if (glm::distance(m_transComp->getTranslation(), m_target) < 3.f) {
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
}

void AiController::chaseEntity(Entity* toChase) {
	m_entityTarget = toChase;
}

void AiController::setEntity(std::shared_ptr<Entity> toControl) {
	m_controlledEntity = toControl;
}

std::shared_ptr<Entity> AiController::getEntity() {
	return m_controlledEntity;
}

Entity* AiController::getTargetEntity() {
	return m_entityTarget;
}
