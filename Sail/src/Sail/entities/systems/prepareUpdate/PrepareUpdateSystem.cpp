#include "pch.h"
#include "PrepareUpdateSystem.h"

#include "Sail/entities/Entity.h"
#include "Sail/entities/components/TransformComponent.h"

PrepareUpdateSystem::PrepareUpdateSystem() {
	registerComponent<TransformComponent>(true, true, false);
}

PrepareUpdateSystem::~PrepareUpdateSystem() {

}

void PrepareUpdateSystem::update(float dt) {
	for (auto e : entities) {
		e->getComponent<TransformComponent>()->prepareUpdate();
	}
}

