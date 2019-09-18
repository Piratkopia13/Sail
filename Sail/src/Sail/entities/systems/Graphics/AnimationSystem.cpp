#include "pch.h"
#include "AnimationSystem.h"
#include "..//..//Entity.h"
#include "..//..//components/ModelComponent.h"
#include "..//..//components/AnimationComponent.h"

AnimationSystem::AnimationSystem() : BaseComponentSystem() {
	m_requiredComponentTypes.push_back(AnimationComponent::ID);
	m_requiredComponentTypes.push_back(ModelComponent::ID);
}

AnimationSystem::~AnimationSystem() {
}

void AnimationSystem::update(float dt) {
	for (auto& e : m_entities) {





		//TransformComponent* transform = e->getComponent<TransformComponent>();
		//PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
		//
		//transform->rotate(physics->rotation * dt);
		//transform->translate(physics->velocity * dt + physics->acceleration * (dt * dt * 0.5f));
		//
		//physics->velocity += physics->acceleration * dt;
	}
}
