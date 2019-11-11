#include "pch.h"
#include "ParticleEmitterComponent.h"

ParticleEmitterComponent::ParticleEmitterComponent() {
	position = { 0.f, 0.f, 0.f };
	spread = { 0.f, 0.f, 0.f };
	velocity = { 0.f, 0.f, 0.f };
	acceleration = { 0.f, 0.f, 0.f };
	lifeTime = 2.0f;
	spawnRate = 0.1f;
	spawnTimer = 0.0f;
}

ParticleEmitterComponent::~ParticleEmitterComponent() {

}
