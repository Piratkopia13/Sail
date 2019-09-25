#include "pch.h"
#include "GunComponent.h"

void GunComponent::setFiring(glm::vec3 pos, glm::vec3 dir) {
	firing = true;
	position = pos;
	direction = dir;
}