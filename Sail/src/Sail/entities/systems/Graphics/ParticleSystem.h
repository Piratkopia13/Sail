#pragma once
#include "..//BaseComponentSystem.h"

class ParticleSystem final : public BaseComponentSystem {
public:
	ParticleSystem();
	~ParticleSystem();

	void spawnParticles(int numberOfParticles);

	void update(float dt);
};