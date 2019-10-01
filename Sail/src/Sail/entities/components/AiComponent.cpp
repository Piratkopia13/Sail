#include "pch.h"
#include "AiComponent.h"

void AiComponent::setTarget(Entity* entityTarget_) {
	entityTarget = entityTarget_;
	reachedTarget = false;
}

void AiComponent::setTarget(glm::vec3 targetPos) {
	posTarget = targetPos;
	reachedTarget = false;
}
