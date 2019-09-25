#include "pch.h"
#include "AiComponent.h"

void AiComponent::setTarget(Entity* targetEntity) {
	entityTarget = targetEntity;
	reachedTarget = false;
}

void AiComponent::setTarget(glm::vec3 targetPos) {
	posTarget = targetPos;
	reachedTarget = false;
}
