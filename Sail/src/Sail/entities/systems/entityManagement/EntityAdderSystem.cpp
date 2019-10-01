#include "pch.h"
#include "EntityAdderSystem.h"
#include "..//..//ECS.h"

EntityAdderSystem::EntityAdderSystem() {
}

EntityAdderSystem::~EntityAdderSystem() {
}

void EntityAdderSystem::update(float dt) {
	ECS::Instance()->addAllQueuedEntities();
}
