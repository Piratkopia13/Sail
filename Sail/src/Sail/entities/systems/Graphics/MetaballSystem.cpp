#include "pch.h"
#include "MetaballSystem.h"
#include "Sail/Application.h"
#include "Sail/entities/components/MetaballComponent.h"
#include "Sail/entities/components/TransformComponent.h"

MetaballSystem::MetaballSystem() {
	requiredComponentTypes.push_back(MetaballComponent::ID);
	requiredComponentTypes.push_back(TransformComponent::ID);
}

MetaballSystem::~MetaballSystem() {

}

void MetaballSystem::update(float dt) {
	//Application::getInstance()->ge
	for (auto& e : entities) {

	}
}
