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
			if (gun->projectileSpawnTimer == 0.f) {
				auto e = ECS::Instance()->createEntity("projectile");
				e->addComponent<ModelComponent>(gun->getProjectileModel());
				e->addComponent<BoundingBoxComponent>();
				BoundingBox* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
				//Done here because systems doesn't update the bounding box first frame so it passes through things
				boundingBox->setHalfSize(glm::vec3(0.2f));
				boundingBox->setPosition(gun->position);
				e->addComponent<LifeTimeComponent>(2.0f);
				e->addComponent<ProjectileComponent>();
				e->addComponent<TransformComponent>(gun->position);
				TransformComponent* transform = e->getComponent<TransformComponent>();
				transform->setScale(glm::vec3(1.0f, 1.0f, 1.0f) * 0.2f);
				transform->rotateAroundY(glm::atan(gun->direction.x / gun->direction.z));
				
				e->addComponent<PhysicsComponent>();
				PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
				physics->velocity = gun->direction * gun->projectileSpeed;
				physics->constantAcceleration = glm::vec3(0.f, -9.8f, 0.f);
				physics->drag = 2.0f;
				physics->bounciness = 0.1f;
				physics->padding = 0.2f;

				m_gameDataTracker->logWeaponFired();
			}
			gun->projectileSpawnTimer += dt;
			if (gun->projectileSpawnTimer > gun->getSpawnLimit()) {
				gun->projectileSpawnTimer = 0.f;
			}

			gun->firing = false;
		}
		else {
			gun->projectileSpawnTimer = 0.f;
		}
	}
} // update
