#include "ParticleComponent.h"

ParticleComponent::ParticleComponent() {
	position = glm::vec3(0.0f);
	spread = glm::vec3(0.0f);
	velocity = glm::vec3(0.0f);
	acceleration = glm::vec3(0.0f);
	spawnRate = 0.1f;
}

ParticleComponent::~ParticleComponent() {

}
