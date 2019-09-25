#include "pch.h"
#include "ProjectileSystem.h"
#include "..//..//components/GunComponent.h"


ProjectileSystem::ProjectileSystem() : BaseComponentSystem() {
	requiredComponentTypes.push_back(GunComponent::ID);
}

ProjectileSystem::~ProjectileSystem() {

}

void ProjectileSystem::update(float dt, Scene* scene) {
	for (auto& e : entities) {
		GunComponent* gun = e->getComponent<GunComponent>();

		if (gun->firing) {
			if (gun->projectileSpawnTimer == 0.f) {
				auto e = ECS::Instance()->createEntity("projectile");
				e->addComponent<ModelComponent>(gun->getProjectileModel());
				e->addComponent<BoundingBoxComponent>();
				e->addComponent<LifeTimeComponent>(2.0f);
				e->addComponent<TransformComponent>(gun->position);
				TransformComponent* transform = e->getComponent<TransformComponent>();
				transform->setScale(glm::vec3(1.0f, 1.0f, 5.0f) * 0.2f);
				transform->rotateAroundY(glm::atan(gun->direction.x / gun->direction.z));
				
				e->addComponent<PhysicsComponent>();
				PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
				physics->velocity = gun->direction * gun->projectileSpeed;
				physics->acceleration = glm::vec3(0.f, -30.f, 0.f);//fix when gravity is properly implemented

				scene->addEntity(e);//change when scene is a component.
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
void ProjectileSystem::update(float dt) {

}
