#include "pch.h"
#include "ParticleSystem.h"
#include "Sail/entities/components/ParticleComponent.h"
#include "Sail/entities/Entity.h"

ParticleSystem::ParticleSystem() {
	registerComponent<ParticleComponent>(true, true, true);
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
		ParticleComponent* particleComp = e->getComponent<ParticleComponent>();

		if (particleComp->spawnTimer >= particleComp->spawnRate) {
			int particlesToSpawn = (int) glm::floor(particleComp->spawnTimer / glm::max(particleComp->spawnRate, 0.00001f));
			spawnParticles(particlesToSpawn);
			particleComp->spawnTimer -= particleComp->spawnRate * particlesToSpawn;
		}
		particleComp->spawnTimer += dt;
	}
}