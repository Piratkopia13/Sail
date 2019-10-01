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
#include "Sail/entities/components/CollidableComponent.h"


GunSystem::GunSystem() : BaseComponentSystem() {
	requiredComponentTypes.push_back(GunComponent::ID);
	readBits |= GunComponent::BID;
	writeBits |= GunComponent::BID;
}

GunSystem::~GunSystem() {

}

void GunSystem::update(float dt, Scene* scene) {
	for (auto& e : entities) {
		GunComponent* gun = e->getComponent<GunComponent>();

		if (gun->firing) {
			if (gun->projectileSpawnTimer == 0.f) {
				
				
				for (int i = 0; i <= 10; i++) {
					auto e = ECS::Instance()->createEntity("projectile");
					e->addComponent<MetaballComponent>(/*gun->getProjectileModel()*/);
					e->addComponent<BoundingBoxComponent>();
					//e->addComponent<CollidableComponent>();
					e->getComponent<BoundingBoxComponent>()->getBoundingBox()->setHalfSize(glm::vec3(0.5, 0.5, 0.5));
					e->addComponent<LifeTimeComponent>(2.0f);
					e->addComponent<ProjectileComponent>();					
					e->addComponent<TransformComponent>(gun->position - gun->direction * (0.15f * i));
					e->addComponent<PhysicsComponent>();
					PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
					physics->velocity = gun->direction * (gun->projectileSpeed + i * 0.1f);
					physics->constantAcceleration = glm::vec3(0.f, -9.8f, 0.f);

					scene->addEntity(e);//change when scene is a component.
				}	


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
}
void GunSystem::update(float dt) {

}
