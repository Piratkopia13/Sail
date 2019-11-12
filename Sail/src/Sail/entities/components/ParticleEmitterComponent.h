#pragma once

#include "Component.h"

#include <glm/glm.hpp>

class ParticleEmitterComponent : public Component<ParticleEmitterComponent> {
public:
	ParticleEmitterComponent();
	~ParticleEmitterComponent();

	glm::vec3 position;
	glm::vec3 spread;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	float lifeTime;
	float spawnRate;
	float spawnTimer;
};