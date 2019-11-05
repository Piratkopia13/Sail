#include "pch.h"
#include "ParticleSystem.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/Entity.h"

ParticleSystem::ParticleSystem() {
	registerComponent<ParticleEmitterComponent>(true, true, true);
}

ParticleSystem::~ParticleSystem() {

}

void ParticleSystem::spawnParticles(int numberOfParticles) {
	//Not implemented yet
	//Will probably send the number of particles to the gpu so that 
	//the right number of particles can be spawned in the compute shader.
	assert(false);
}

void ParticleSystem::update(float dt) {
	for (auto& e : entities) {
		ParticleEmitterComponent* particleEmitterComp = e->getComponent<ParticleEmitterComponent>();

		if (particleEmitterComp->spawnTimer >= particleEmitterComp->spawnRate) {
			//Spawn the correct number of particles
			int particlesToSpawn = (int) glm::floor(particleEmitterComp->spawnTimer / glm::max(particleEmitterComp->spawnRate, 0.00001f));
			spawnParticles(particlesToSpawn);
			//Decrease timer
			particleEmitterComp->spawnTimer -= particleEmitterComp->spawnRate * particlesToSpawn;
		}
		particleEmitterComp->spawnTimer += dt;
	}
}