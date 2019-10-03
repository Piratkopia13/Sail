#include "pch.h"

#include "GunSystem.h"

#include "Sail/entities/ECS.h"

#include "Sail/entities/components/ProjectileComponent.h"
#include "Sail/entities/components/LifeTimeComponent.h"
#include "Sail/entities/components/BoundingBoxComponent.h"
#include "Sail/entities/components/ModelComponent.h"
#include "Sail/entities/components/PhysicsComponent.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/GunComponent.h"

#include "Sail/entities/components/MetaballComponent.h"
#include "Sail/utils/GameDataTracker.h"
#include "Sail/entities/components/CollidableComponent.h"

GunSystem::GunSystem() : BaseComponentSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<GunComponent>(true, true, true);
	m_gameDataTracker = &GameDataTracker::getInstance();
}

GunSystem::~GunSystem() {

}


void GunSystem::update(float dt) {
	for (auto& e : entities) {
		GunComponent* gun = e->getComponent<GunComponent>();

		if (gun->firing) {
			if (gun->gunOverloadTimer <= 0) {
				if ((gun->gunOverloadvalue += dt) > gun->gunOverloadThreshold) {
					gun->gunOverloadTimer = gun->m_gunOverloadCooldown;
					gun->gunOverloadvalue = 0;
				}

				if (gun->projectileSpawnTimer <= 0.f) {
					gun->projectileSpawnTimer = gun->m_projectileSpawnCooldown;

					for (int i = 0; i <= 1; i++) {
						auto e = ECS::Instance()->createEntity("projectile");
						glm::vec3 randPos;
						float maxrand = 0.2f;
						randPos.r = ((float)rand() / RAND_MAX) * maxrand;
						randPos.g = ((float)rand() / RAND_MAX) * maxrand;
						randPos.b = ((float)rand() / RAND_MAX) * maxrand;

						e->addComponent<MetaballComponent>();
						e->addComponent<BoundingBoxComponent>();
						e->getComponent<BoundingBoxComponent>()->getBoundingBox()->setHalfSize(glm::vec3(0.1, 0.1, 0.1));
						e->addComponent<LifeTimeComponent>(4.0f);
						e->addComponent<ProjectileComponent>();
						e->addComponent<TransformComponent>((gun->position + randPos) - gun->direction * (0.15f * i));

						e->addComponent<PhysicsComponent>();
						PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
						physics->velocity = gun->direction * gun->projectileSpeed;
						physics->constantAcceleration = glm::vec3(0.f, -9.8f, 0.f);
						physics->drag = 2.0f;
						physics->bounciness = 0.1f;
						physics->padding = 0.2f;

						m_gameDataTracker->logWeaponFired();
					}
				}
			}

			gun->firing = false;
		}
		else {
			if (gun->gunOverloadvalue > 0) {
				gun->gunOverloadvalue -= dt;
			}
		}

		gun->gunOverloadTimer -= dt;
		gun->projectileSpawnTimer -= dt;
	}
}
