#include "pch.h"

#include "GunSystem.h"


#include "Sail/entities/ECS.h"

#include "Sail/entities/components/GunComponent.h"
#include "Sail/entities/components/NetworkSenderComponent.h"
#include "Sail/entities/components/MovementComponent.h"

#include "Sail/utils/GameDataTracker.h"
#include "../Sail/src/Network/NWrapperSingleton.h"
#include "Sail/netcode/NetworkedStructs.h"
#include <xaudio2.h>
#include <xaudio2fx.h>
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

					// Determine projectileSpeed based on how long the gun has been firing continuously
					alterProjectileSpeed(gun);

					// Tell yours and everybody else's NetworkReceiverSystem to spawn the projectile
					for (int i = 0; i < 2; i++) {
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

		// Play shooting sounds based on the GunState
		playShootingSounds(e);

		gun->gunOverloadTimer -= dt;
		gun->projectileSpawnTimer -= dt;
	}
}


void GunSystem::alterProjectileSpeed(GunComponent* gun) {
	gun->projectileSpeed = gun->baseProjectileSpeed + (gun->projectileSpeedRange * (gun->gunOverloadvalue/gun->gunOverloadThreshold));
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


	XAUDIO2FX_REVERB_PARAMETERS reverbParams;
	float frequency = 20 + 20000 * (gun->gunOverloadvalue / gun->gunOverloadThreshold);
	float eqCutoff = 0 + 14 * (gun->gunOverloadvalue / gun->gunOverloadThreshold);

	reverbParams.ReflectionsDelay = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY;
	reverbParams.ReverbDelay = XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY;
	reverbParams.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY;
	reverbParams.PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	reverbParams.PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	reverbParams.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	reverbParams.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	reverbParams.EarlyDiffusion = XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION;
	reverbParams.LateDiffusion = XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION;
	reverbParams.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN;
	reverbParams.LowEQCutoff = eqCutoff;// XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF;
	reverbParams.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN;
	reverbParams.HighEQCutoff = eqCutoff; XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF;
	reverbParams.RoomFilterFreq = frequency;//XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ;
	reverbParams.RoomFilterMain = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN;
	reverbParams.RoomFilterHF = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF;
	reverbParams.ReflectionsGain = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN;
	reverbParams.ReverbGain = XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN;
	reverbParams.DecayTime = XAUDIO2FX_REVERB_DEFAULT_DECAY_TIME;
	reverbParams.Density = XAUDIO2FX_REVERB_DEFAULT_DENSITY;
	reverbParams.RoomSize = XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE;
	reverbParams.WetDryMix = XAUDIO2FX_REVERB_DEFAULT_WET_DRY_MIX;
	reverbParams.DisableLateField = true;
	if (e->hasComponent<AudioComponent>()) {
		e->getComponent<AudioComponent>()->m_sounds[Audio::SHOOT_START].frequency = frequency;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SHOOT_LOOP].frequency = frequency;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SHOOT_END].frequency = frequency;
	}
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

void GunSystem::playShootingSounds(Entity* e) {
	GunComponent* gun = e->getComponent<GunComponent>();

	// Play sounds depending on which state the gun is in.
	if (gun->state == GunState::STARTING) {
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_START].isPlaying = true;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_START].playOnce = true;
	}
	else if (gun->state == GunState::LOOPING) {
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_START].isPlaying = false;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].isPlaying = true;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].playOnce = true;
	}
	else if (gun->state == GunState::ENDING) {
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].isPlaying = false;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_END].isPlaying = true;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_END].playOnce = true;

		gun->state = GunState::STANDBY;
	}
	else {	/* gun->state == GunState::STANDBY */

	}
}
