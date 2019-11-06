#include "pch.h"
#include "ParticleEmitterComponent.h"

ParticleEmitterComponent::ParticleEmitterComponent() {
	position = glm::vec3(0.0f);
	spread = glm::vec3(0.0f);
	velocity = glm::vec3(0.0f);
	acceleration = glm::vec3(0.0f);
	spawnRate = 0.1f;
	spawnTimer = 0.0f;
}

ParticleEmitterComponent::~ParticleEmitterComponent() {

}
