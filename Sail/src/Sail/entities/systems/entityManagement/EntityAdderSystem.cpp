#include "pch.h"
#include "EntityAdderSystem.h"
#include "..//..//ECS.h"

EntityAdderSystem::EntityAdderSystem() {
}

EntityAdderSystem::~EntityAdderSystem() {
}

void EntityAdderSystem::update() {
	ECS::Instance()->addAllQueuedEntities();
}
