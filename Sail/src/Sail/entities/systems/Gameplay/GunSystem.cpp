#include "pch.h"

#include "GunSystem.h"


#include "Sail/entities/ECS.h"

#include "Sail/entities/components/GunComponent.h"
#include "Sail/entities/components/NetworkSenderComponent.h"
#include "Sail/entities/components/MovementComponent.h"

#include "Sail/utils/GameDataTracker.h"
#include "../Sail/src/Network/NWrapperSingleton.h"
#include "Sail/netcode/NetworkedStructs.h"
#include <random>

// TODO: Remove, here only for temporary debugging
#include <iostream>

GunSystem::GunSystem() : BaseComponentSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<GunComponent>(true, true, true);
	registerComponent<MovementComponent>(true, true, false);
	registerComponent<NetworkSenderComponent>(false, true, true);

	m_gameDataTracker = &GameDataTracker::getInstance();
}

GunSystem::~GunSystem() {

}

void GunSystem::update(float dt) {
	for (auto& e : entities) {
		GunComponent* gun = e->getComponent<GunComponent>();

		// Gun is firing
		if (gun->firing) {

			// Gun is not overloaded
			if (gun->gunOverloadTimer <= 0) {

				// SHOOT
				if (gun->projectileSpawnTimer <= 0.f) {
					for (int i = 0; i < 2; i++) {
						// Tell yours and everybody else's NetworkReceiverSystem to spawn the projectile
						NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
							Netcode::MessageType::SPAWN_PROJECTILE,
							SAIL_NEW Netcode::MessageSpawnProjectile{
								gun->position,
								gun->direction * gun->projectileSpeed + e->getComponent<MovementComponent>()->velocity,
								e->getComponent<NetworkSenderComponent>()->m_id
							}
						);
					}
					m_gameDataTracker->logWeaponFired();
					fireGun(e, gun);
				}
				// DO NOT SHOOT (Cooldown between shots)
				else {
					gun->firingContinuously = false;
				}

				// Overload the gun if necessary
				if ((gun->gunOverloadvalue += dt) > gun->gunOverloadThreshold) {
					overloadGun(e, gun);
				}
			}
		}
		// Gun is not firing.
		else {
			// Reduce the overload value
			if (gun->gunOverloadvalue > 0) {
				gun->gunOverloadvalue -= dt;
			}

			// Gun stopped firing
			if (gun->firingContinuously) {
				// Set gun state to END
				setGunStateEND(e, gun);
			}

			gun->firingContinuously = false;
		}

		// Send shooting events based on the GunState
		sendShootingEvents(e);

		gun->gunOverloadTimer -= dt;
		gun->projectileSpawnTimer -= dt;
	}
}

void GunSystem::fireGun(Entity* e, GunComponent* gun) {
	gun->projectileSpawnTimer = gun->m_projectileSpawnCooldown;

	// If this is the first shot in this "burst" of projectiles...
	if (!gun->firingContinuously) {
		setGunStateSTART(e, gun);
	}
	else {
		setGunStateLOOP(e, gun);
	}

	gun->firingContinuously = true;
}

void GunSystem::overloadGun(Entity* e, GunComponent* gun) {
	gun->gunOverloadTimer = gun->m_gunOverloadCooldown;
	gun->gunOverloadvalue = 0;

	setGunStateEND(e, gun);

	gun->firingContinuously = false;
}

void GunSystem::setGunStateSTART(Entity* e, GunComponent* gun) {
	e->getComponent<NetworkSenderComponent>()->addMessageType(
		Netcode::MessageType::SHOOT_START
	);

	gun->state = GunState::STARTING;
}

void GunSystem::setGunStateLOOP(Entity* e, GunComponent* gun) {
	// Network automatically handles transition from starting-->looping
	gun->state = GunState::LOOPING;
}

void GunSystem::setGunStateEND(Entity* e, GunComponent* gun) {
	e->getComponent<NetworkSenderComponent>()->addMessageType(
		Netcode::MessageType::SHOOT_END
	);
	gun->state = GunState::ENDING;
}

void GunSystem::sendShootingEvents(Entity* e) {
	GunComponent* gun = e->getComponent<GunComponent>();

	switch (gun->state) {
	case GunState::STARTING:
		EventDispatcher::Instance().emit(StartShootingEvent(e->getComponent<NetworkReceiverComponent>()->m_id));
		break;
	case GunState::LOOPING:
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_START].isPlaying = false;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].isPlaying = true;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].playOnce = true;
		break;
	case GunState::ENDING:
		EventDispatcher::Instance().emit(StopShootingEvent(e->getComponent<NetworkReceiverComponent>()->m_id));
		gun->state = GunState::STANDBY;
		break;
	default:
		break;
	}
}
