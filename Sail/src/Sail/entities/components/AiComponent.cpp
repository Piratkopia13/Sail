#include "pch.h"
#include "AiComponent.h"

void AiComponent::setTarget(Entity* entityTarget_) {
	entityTarget = entityTarget_;
	reachedPathingTarget = false;
}

void AiComponent::setTarget(glm::vec3 targetPos) {
	posTarget = targetPos;
	reachedPathingTarget = false;
}
