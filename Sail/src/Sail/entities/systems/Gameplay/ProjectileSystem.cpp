#include "pch.h"
#include "ProjectileSystem.h"
#include "..//..//Entity.h"
#include "..//..//components/GunComponent.h"


ProjectileSystem::ProjectileSystem() : BaseComponentSystem() {
	requiredComponentTypes.push_back(GunComponent::ID);
}

ProjectileSystem::~ProjectileSystem() {

}

void ProjectileSystem::update(float dt, PerspectiveCamera cam, Scene* scene) {
	for (auto& e : entities) {
		GunComponent* gun = e->getComponent<GunComponent>();

		if (gun->firing) {
			if (gun->projectileSpawnTimer == 0.f) {
				auto e = ECS::Instance()->createEntity("projectile");
				e->addComponent<ModelComponent>(gun->getProjectileModel());
				e->addComponent<BoundingBoxComponent>(gun->getWireframeModel());
				//e->addComponent<LifeTimeComponenent>(); add when in dev
				glm::vec3 camRight = glm::cross(cam.getUp(), cam.getDirection());
				e->addComponent<TransformComponent>(cam.getPosition() + (cam.getDirection() + camRight - cam.getUp()), glm::vec3(0.f), glm::vec3(0.3f));
				TransformComponent* transform = e->getComponent<TransformComponent>();
				
				e->addComponent<PhysicsComponent>();
				PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
				physics->velocity = cam.getDirection() * gun->projectileSpeed;
				physics->acceleration = glm::vec3(0.f, -25.f, 0.f);//fix when scaling is in dev

				scene->addEntity(e);//change when scene is a component.

			}
			gun->projectileSpawnTimer += dt;
			if (gun->projectileSpawnTimer > gun->getSpawnLimit()) {
				gun->projectileSpawnTimer = 0.f;
			}
		}
		else {
			gun->projectileSpawnTimer = 0.f;
		}
	}
}
void ProjectileSystem::update(float dt) {

}
