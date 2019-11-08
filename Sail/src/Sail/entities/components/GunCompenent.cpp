#include "pch.h"
#include "GunComponent.h"

GunComponent::GunComponent() {
	 projectileSpawnTimer = 0.f;
	 gunOverloadTimer = 0.f;
	 m_projectileSpawnCooldown = 0.015f;
	 m_gunOverloadCooldown = .00f;

	 projectileSpeed = 20.f;
	 finalProjectileSpeed = 5.0f;
	 projectileSpeedRange = finalProjectileSpeed - projectileSpeed;


	 gunOverloadvalue = 0.f;
	 gunOverloadThreshold = 3.0f;	// How many seconds of continual firing until complete fall-off

	firing = false;
}

void GunComponent::setFiring(glm::vec3 pos, glm::vec3 dir) {
	firing = true;
	position = pos;
	direction = dir;
}