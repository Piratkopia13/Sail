#include "pch.h"
#include "PhysicSystem.h"
#include "..//..//Entity.h"
#include "..//../components/TransformComponent.h"
#include "..//../components/PhysicsComponent.h"

PhysicSystem::PhysicSystem() : BaseComponentSystem() {
	m_requiredComponentTypes.push_back(TransformComponent::getStaticID());
	m_requiredComponentTypes.push_back(PhysicsComponent::getStaticID());
}

PhysicSystem::~PhysicSystem() {
}

void PhysicSystem::update(float dt) {

	int idtran = TransformComponent::getStaticID();
	int idphy = PhysicsComponent::getStaticID();

	for (auto e : m_entities) {
		TransformComponent* transform = e->getComponent<TransformComponent>();
		PhysicsComponent* physics = e->getComponent<PhysicsComponent>();

		transform->translate(physics->velocity * dt);
		transform->rotate(physics->rotation * dt);
	}
}
