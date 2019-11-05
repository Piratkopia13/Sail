#pragma once

#include "pch.h"
#include "Component.h"

class ParticleComponent : public Component<ParticleComponent> {
public:
	ParticleComponent();
	~ParticleComponent();

	glm::vec3 position;
	glm::vec3 spread;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	float spawnRate;
};