#include "pch.h"
#include "PhysicSystem.h"
#include "..//..//Entity.h"
//#include "..//..//components/TransformComponent.h"
#include "..//..//components/GameTransformComponent.h"
#include "..//..//components/PhysicsComponent.h"

PhysicSystem::PhysicSystem() : BaseComponentSystem() {
	m_requiredComponentTypes.push_back(GameTransformComponent::ID);
	m_requiredComponentTypes.push_back(PhysicsComponent::ID);
}

PhysicSystem::~PhysicSystem() {
}

void PhysicSystem::update(float dt) {
	for (auto& e : m_entities) {
		GameTransformComponent* transform = e->getComponent<GameTransformComponent>();
		PhysicsComponent* physics = e->getComponent<PhysicsComponent>();

		transform->rotate(physics->rotation * dt);
		transform->translate(physics->velocity * dt + physics->acceleration * (dt * dt * 0.5f));

		physics->velocity += physics->acceleration * dt;
	}
}
