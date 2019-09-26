#include "pch.h"
#include "RenderSystem.h"
#include "..//..//components/ModelComponent.h"
#include "..//..//Entity.h"

RenderSystem::RenderSystem() {
	requiredComponentTypes.push_back(ModelComponent::ID);
}

RenderSystem::~RenderSystem() {
}

 void RenderSystem::update(float dt) {
}
