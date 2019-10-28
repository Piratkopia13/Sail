#pragma once
#include "Component.h"
#include <glm/glm.hpp>

class Model;

class GunComponent : public Component<GunComponent> {
public:
	GunComponent();
	~GunComponent() {};

	void setFiring(glm::vec3 pos, glm::vec3 dir);

	glm::vec3 position;
	glm::vec3 direction;

	float projectileSpawnTimer;
	float gunOverloadTimer;
	float m_projectileSpawnCooldown;
	float m_gunOverloadCooldown;

	float projectileSpeed;

	float gunOverloadvalue;
	float gunOverloadThreshold;

	bool firing;

private:

	float m_projectileSpawnLimit = .3f;
};
