#include "pch.h"
#include "SimplePhysicsSystem.h"

#include "../components/MovementComponent.h"
#include "../components/TransformComponent.h"


SimplePhysicsSystem::SimplePhysicsSystem() {}

SimplePhysicsSystem::~SimplePhysicsSystem() {}

void SimplePhysicsSystem::registerEntity(Entity::SPtr entity) {
	m_entities.push_back(entity);
}

void SimplePhysicsSystem::execute(float dt) {

	for ( Entity::SPtr& entity : m_entities ) {
		MovementComponent* moveComp = entity->getComponent<MovementComponent>();
		TransformComponent* transComp = entity->getComponent<TransformComponent>();

		if ( moveComp->getSpeed() > 0.f ) {
			transComp->translate(moveComp->getDirection() * moveComp->getSpeed() * dt);
			if ( moveComp->getSpeed() < 0.1f ) {
				moveComp->setSpeed(0.f);
			}
			else {
				moveComp->setSpeed(moveComp->getSpeed() * ( 1.0f - 4.0f * dt ));
			}
		}
	}

}
