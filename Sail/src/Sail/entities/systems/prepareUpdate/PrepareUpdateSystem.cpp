#include "pch.h"
#include "PrepareUpdateSystem.h"

#include "Sail/entities/Entity.h"
#include "Sail/entities/components/TransformComponent.h"

PrepareUpdateSystem::PrepareUpdateSystem() {
	requiredComponentTypes.push_back(TransformComponent::ID); // read-only
	readBits |= TransformComponent::BID;
}

PrepareUpdateSystem::~PrepareUpdateSystem() {

}

void PrepareUpdateSystem::update(float dt) {
	for (auto e : entities) {
		e->getComponent<TransformComponent>()->prepareUpdate();
	}
}

