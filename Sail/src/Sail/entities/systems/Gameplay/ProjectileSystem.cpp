#include "pch.h"
#include "ProjectileSystem.h"
#include "Sail/entities/components/ProjectileComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/LocalPlayerComponent.h"
#include "Sail/entities/components/OnlinePlayerComponent.h"
#include "Sail/entities/components/CollisionComponent.h"
#include "Sail/entities/components/NetworkSenderComponent.h"

ProjectileSystem::ProjectileSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<ProjectileComponent>(true, true, true);
	registerComponent<CollisionComponent>(true, true, false);
	registerComponent<CandleComponent>(false, true, true);
}

ProjectileSystem::~ProjectileSystem() {

}

void ProjectileSystem::update(float dt) {
	for (auto& e : entities) {
		CollisionComponent* collisionComp = e->getComponent<CollisionComponent>();
		auto projectileCollisions = collisionComp->collisions;
		for (auto& collision : projectileCollisions) {
			// Upon colliding with candle
			
			if (collision.entity->hasComponent<CandleComponent>()) {
				ProjectileComponent* p = e->getComponent<ProjectileComponent>();
				
				// Only do hitdetection with bullets from local player which hit online players
				if (p->ownedbyLocalPlayer == true && !collision.entity->hasComponent<LocalPlayerComponent>()) {
					// TODO: Consume da waterball (smök)
					collision.entity->getComponent<CandleComponent>()->hitWithWater(e->getComponent<ProjectileComponent>()->m_damage);
				
					// Send to network ---------------------
					if (e->hasComponent<NetworkSenderComponent>() == false) {
						e->addComponent<NetworkSenderComponent>(
							Netcode::MessageType::WATER_HIT_PLAYER,
							Netcode::EntityType::PLAYER_ENTITY,
							(unsigned char)0
						);
					}
					else {
						// Not mroe than 1
						if (e->getComponent<NetworkSenderComponent>()->getDataTypeExists(Netcode::MessageType::WATER_HIT_PLAYER) == false) {
							e->getComponent<NetworkSenderComponent>()->addDataType(Netcode::MessageType::WATER_HIT_PLAYER);
						}
					}
					netSystem.pushEvent(WaterhitPlayer);

					NetworkSenderComponent* n = e->getComponent<NetworkSenderComponent>();
					n->m_id = collision.entity->getComponent<OnlinePlayerComponent>()->netEntityID;
				}
			}
		}
	}
}