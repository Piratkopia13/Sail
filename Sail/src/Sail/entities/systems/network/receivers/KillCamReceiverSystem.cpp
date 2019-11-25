#include "pch.h"
#include "KillCamReceiverSystem.h"
#include "Sail/entities/Entity.h"

// TODO: Remove unnecessary includes

#include "Sail/entities/components/components.h"
#include "Sail/entities/systems/network/NetworkSenderSystem.h"
#include "Sail/entities/systems/Gameplay/LevelSystem/LevelSystem.h"
#include "Sail/entities/systems/Gameplay/SprinklerSystem.h"

#include "Network/NWrapperSingleton.h"
#include "Sail/netcode/ArchiveTypes.h"

#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/physics/UpdateBoundingBoxSystem.h"

// Creation of mid-air bullets from here.
#include "Sail/entities/systems/Gameplay/GunSystem.h"
#include "Sail/utils/GameDataTracker.h"
#include "../SPLASH/src/game/events/GameOverEvent.h"

#include "Sail/events/EventDispatcher.h"

#include "Sail/graphics/camera/CameraController.h"


//#define _LOG_TO_FILE
#if defined(DEVELOPMENT) && defined(_LOG_TO_FILE)
#include <fstream>
static std::ofstream out("LogFiles/KillCamReceiverSystem.cpp.log");
#endif


// TODO: register more components
KillCamReceiverSystem::KillCamReceiverSystem() : ReceiverBase() {
	registerComponent<ReplayReceiverComponent>(true, false, false);

	EventDispatcher::Instance().subscribe(Event::Type::PLAYER_DEATH, this);
	EventDispatcher::Instance().subscribe(Event::Type::TOGGLE_SLOW_MOTION, this);
}

KillCamReceiverSystem::~KillCamReceiverSystem() {
	EventDispatcher::Instance().unsubscribe(Event::Type::PLAYER_DEATH, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::TOGGLE_SLOW_MOTION, this);
}

void KillCamReceiverSystem::stop() {
	// Clear the saved data
	for (std::queue<std::string>& data : m_replayData) {
		data = std::queue<std::string>();
	}

	m_slowMotionState = SlowMotionSetting::DISABLE;
	m_currentWriteInd = 0;
	m_currentReadInd  = 1;
	m_hasStarted      = false;
	m_idOfKiller      = Netcode::UNINITIALIZED;
	m_idOfKillingProjectile = Netcode::UNINITIALIZED;
	m_killerPlayer = nullptr;
	m_killerProjectile = nullptr;
	m_trackingProjectile = false;
	m_cam = nullptr;

	m_projectilePos = { 0,0,0 };
	m_killerHeadPos = { 0,0,0 };
}

void KillCamReceiverSystem::init(Netcode::PlayerID player, Camera* cam) {
	initBase(player);

	if (m_cam == nullptr) {
		m_cam = SAIL_NEW CameraController(cam);
	}
}

void KillCamReceiverSystem::handleIncomingData(const std::string& data) {
	if (!m_hasStarted) { // Stop writing data to this once the 
		std::lock_guard<std::mutex> lock(m_replayDataLock);

		m_replayData[m_currentWriteInd].push(data);
	}
}


// Prepare transform components for the next frame
void KillCamReceiverSystem::prepareUpdate() {
	for (auto e : entities) {
		e->getComponent<TransformComponent>()->prepareUpdate();
	}
}

// Increments the indexes in the ring buffer once per tick and clears the next write-index
void KillCamReceiverSystem::update(float dt) {
	if (!m_hasStarted) {
		std::lock_guard<std::mutex> lock(m_replayDataLock);

		m_currentWriteInd = ++m_currentWriteInd % REPLAY_BUFFER_SIZE;
		m_currentReadInd  = ++m_currentReadInd  % REPLAY_BUFFER_SIZE;

		m_replayData[m_currentWriteInd] = std::queue<std::string>(); // Clear the current write-position's queue in the ring buffer
	}
}

void KillCamReceiverSystem::updatePerFrame(float dt, float alpha) {
	// Helper function
	auto interpolate = [](float prev, float current, float alpha) {
		return (alpha * current) + ((1.0f - alpha) * prev);
	};

	// only update the camera's position if a player killed us
	if (m_idOfKillingProjectile == Netcode::UNINITIALIZED || m_killerPlayer == nullptr) {
		return;
	}

	static bool TEST = true;

	AnimationComponent* animation = m_killerPlayer->getComponent<AnimationComponent>();
	//BoundingBoxComponent* playerBB  = e->getComponent<BoundingBoxComponent>();
	TransformComponent* transform = m_killerPlayer->getComponent<TransformComponent>();

	if (m_killerPlayer != nullptr) {
		m_killerHeadPos = transform->getRenderMatrix(alpha) * glm::vec4(animation->headPositionLocalCurrent, 1.f);
	}


	if (m_trackingProjectile) {
		// Follow the killing piece of water in slow motion
		if (m_killerProjectile != nullptr) {
			m_projectilePos = m_killerProjectile->getComponent<TransformComponent>()->getInterpolatedTranslation(alpha);
		}


		if (TEST) {
			SAIL_LOG("H: x: " + std::to_string(m_killerHeadPos.x) + "\ty: " + std::to_string(m_killerHeadPos.y) + "\tz: " + std::to_string(m_killerHeadPos.z));
			SAIL_LOG("P: x: " + std::to_string(m_projectilePos.x) + "\ty: " + std::to_string(m_projectilePos.y) + "\tz: " + std::to_string(m_projectilePos.z));
			SAIL_LOG("D: " + std::to_string(glm::distance(m_killerHeadPos, m_projectilePos)));

			TEST = false;
		}

		const glm::vec3 headToProjectile = m_projectilePos - m_killerHeadPos;
		//const glm::vec3 lookAt = m_projectilePos - headToProjectile;
		const glm::vec3 camDir = glm::normalize(headToProjectile);
		const glm::vec3 cameraPos = m_projectilePos - (0.7f * camDir) + glm::vec3(0.f, 0.1f, 0.f);

		m_cam->setCameraPosition(cameraPos);
		m_cam->setCameralookAt(m_projectilePos);
		//m_cam->setCameraDirection(camDir);
	} else {
		// Show the killer's perspective
		m_cam->setCameraPosition(m_killerHeadPos);

		const glm::quat rot = glm::angleAxis(-interpolate(animation->prevPitch, animation->pitch, alpha), glm::vec3(1, 0, 0));
		const glm::quat rotated = glm::normalize(rot * transform->getInterpolatedRotation(alpha));

		const glm::vec3 forwards = rotated * glm::vec3(0.f, 0.f, 1.f);

		m_cam->setCameraDirection(forwards);
	}

}

// Should only be called when the killcam is active
void KillCamReceiverSystem::processReplayData(float dt) {
	// Add the entities to all relevant systems so that they for example will have their animations updated
	if (!m_hasStarted) {
		// Only play kill cam if we were actually killed by a player
		if (m_idOfKillingProjectile == Netcode::UNINITIALIZED) {
			EventDispatcher::Instance().emit(ToggleKillCamEvent(false));
			return;
		}

		const Netcode::PlayerID killerID = Netcode::getComponentOwner(m_idOfKillingProjectile);

		for (auto e : entities) {
			e->tryToAddToSystems = true;
			e->addComponent<RenderInReplayComponent>();


			if (e->hasComponent<PlayerComponent>()
				&& killerID == Netcode::getComponentOwner(e->getComponent<ReplayReceiverComponent>()->m_id)) 
			{
				e->addComponent<KillerComponent>();
				m_killerPlayer = e;

				SAIL_LOG("I was killed by: " + NWrapperSingleton::getInstance().getPlayer(killerID)->name);
			}
		}
		m_hasStarted = true;
	}

	std::lock_guard<std::mutex> lock(m_replayDataLock);

	processData(dt, m_replayData[m_currentReadInd], false);
	m_currentReadInd = ++m_currentReadInd % REPLAY_BUFFER_SIZE;

	// If we've reached the end of the killcam we should end it
	if (m_currentReadInd == m_currentWriteInd) {
		EventDispatcher::Instance().emit(ToggleKillCamEvent(false));
	}
}


#ifdef DEVELOPMENT
unsigned int KillCamReceiverSystem::getByteSize() const {
	unsigned int size = BaseComponentSystem::getByteSize() + sizeof(*this);
	for (int i = 0; i < REPLAY_BUFFER_SIZE; i++) {
		const auto& queue = m_replayData[i];
		const size_t queueSize = queue.size();
		size += queueSize * sizeof(std::string);								// string structure size
		if (queueSize) {
			size += queueSize * queue.front().capacity() * sizeof(unsigned char);	// approximate string character length
		}
	}
	return size;
}
#endif

void KillCamReceiverSystem::destroyEntity(const Netcode::ComponentID entityID) {
	if (auto e = findFromNetID(entityID); e) {
		e->queueDestruction();
		
		if (entityID == m_idOfKillingProjectile) {
			m_killerProjectile = nullptr;
		}

		if (entityID == m_idOfKiller) {
			m_killerPlayer = nullptr;
		}

		return;
	}
	SAIL_LOG_WARNING("destoryEntity called but no matching entity found");
}

void KillCamReceiverSystem::enableSprinklers() {
	//ECS::Instance()->getSystem<SprinklerSystem>()->enableSprinklers();
}

// SHOULD REMAIN EMPTY FOR THE KILLCAM
void KillCamReceiverSystem::endMatch(const GameDataForOthersInfo& info) {}

void KillCamReceiverSystem::extinguishCandle(const Netcode::ComponentID candleId, const Netcode::PlayerID shooterID) {
	if (auto e = findFromNetID(candleId); e) {
		e->getComponent<CandleComponent>()->wasJustExtinguished = true;
		e->getComponent<CandleComponent>()->wasHitByPlayerID = shooterID;

		return;
	}
	SAIL_LOG_WARNING("extinguishCandle called but no matching candle entity found");
}

void KillCamReceiverSystem::hitBySprinkler(const Netcode::ComponentID candleOwnerID) {
	//waterHitPlayer(candleOwnerID, Netcode::MESSAGE_SPRINKLER_ID);
}

void KillCamReceiverSystem::igniteCandle(const Netcode::ComponentID candleID) {
	//EventDispatcher::Instance().emit(IgniteCandleEvent(candleID));

	if (auto candle = findFromNetID(candleID); candle) {

		auto candleComp = candle->getComponent<CandleComponent>();
		if (!candleComp->isLit) {
			candleComp->health = MAX_HEALTH;
			candleComp->respawns++;
			candleComp->downTime = 0.f;
			candleComp->isLit = true;
			candleComp->userReignition = false;
			candleComp->invincibleTimer = 1.5f;
		}
	} else {
		SAIL_LOG_WARNING("igniteCandle called but no matching entity found");
	}
}

// SHOULD REMAIN EMPTY FOR THE KILLCAM
void KillCamReceiverSystem::matchEnded() {}

void KillCamReceiverSystem::playerDied(const Netcode::ComponentID networkIdOfKilled, const Netcode::ComponentID killerID) {
	destroyEntity(networkIdOfKilled);
	
	//if (auto e = findFromNetID(networkIdOfKilled); e) {
	//	EventDispatcher::Instance().emit(PlayerDiedEvent(
	//		e,
	//		m_playerEntity,
	//		playerIdOfShooter,
	//		networkIdOfKilled)
	//	);
	//	return;
	//}
	//SAIL_LOG_WARNING("playerDied called but no matching entity found");
}

void KillCamReceiverSystem::setAnimation(const Netcode::ComponentID id, const AnimationInfo& info) {
	if (auto e = findFromNetID(id); e) {
		auto animation = e->getComponent<AnimationComponent>();
		animation->setAnimation(info.index);
		animation->animationTime = info.time;
		animation->prevPitch = animation->pitch;
		animation->pitch = info.pitch;
		return;
	}
	SAIL_LOG_WARNING("setAnimation called but no matching entity found");
}

void KillCamReceiverSystem::setCandleHealth(const Netcode::ComponentID candleId, const float health) {
	for (auto& e : entities) {
		if (e->getComponent<ReplayReceiverComponent>()->m_id == candleId) {
			auto candle = e->getComponent<CandleComponent>();
			candle->health = health;
			// Scale fire particles with health
			auto particles = e->getComponent<ParticleEmitterComponent>();
			particles->spawnRate = 0.01f * (MAX_HEALTH / candle->health);

			if (candle->wasJustExtinguished) {
				candle->health = 0.0f;
				candle->isLit = false;
				candle->wasJustExtinguished = false; // reset for the next tick
			}

			// COLOR/INTENSITY
			float tempHealthRatio = (std::fmaxf(candle->health, 0.f) / MAX_HEALTH);

			LightComponent* lc = e->getComponent<LightComponent>();

			lc->getPointLight().setColor(tempHealthRatio * lc->defaultColor);
			return;
		}
	}
	SAIL_LOG_WARNING("setCandleHelath called but no matching candle entity found");
}

// The player who puts down their candle does this in CandleSystem and tests collisions
// The candle will be moved for everyone else in here
void KillCamReceiverSystem::setCandleState(const Netcode::ComponentID id, const bool isHeld) {

	//EventDispatcher::Instance().emit(HoldingCandleToggleEvent(id, isHeld));


	Entity* player = nullptr;
	Entity* candle = nullptr;

	// Find the candle whose parent has the correct ID
	for (auto candleEntity : entities) {
		if (auto parentEntity = candleEntity->getParent(); parentEntity) {
			if (parentEntity->getComponent<ReplayReceiverComponent>()->m_id == id) {
				player = parentEntity;
				candle = candleEntity;
				break;
			}
		}
	}

	// candle exists => player exists (only need to check candle)
	if (!candle) {
		Logger::Warning("Holding candle toggled but no matching entity found");
		return;
	}

	auto candleComp = candle->getComponent<CandleComponent>();
	auto candleTransComp = candle->getComponent<TransformComponent>();

	candleComp->isCarried = isHeld;
	candleComp->wasCarriedLastUpdate = isHeld;
	if (isHeld) {
		candleTransComp->setTranslation(glm::vec3(10.f, 2.0f, 0.f));
		candleTransComp->setParent(player->getComponent<TransformComponent>());

		player->getComponent<AnimationComponent>()->rightHandEntity = candle;
	} else {
		candleTransComp->removeParent();
		player->getComponent<AnimationComponent>()->rightHandEntity = nullptr;

		// Might be needed
		ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.0f);
	}

}

// Might need some optimization (like sorting) if we have a lot of networked entities
void KillCamReceiverSystem::setLocalPosition(const Netcode::ComponentID id, const glm::vec3& translation) {
	if (auto e = findFromNetID(id); e) {
		e->getComponent<TransformComponent>()->setTranslation(translation);
		return;
	}
	SAIL_LOG_WARNING("setLocalPosition called but no matching entity found");
}

void KillCamReceiverSystem::setLocalRotation(const Netcode::ComponentID id, const glm::vec3& rotation) {
	if (auto e = findFromNetID(id); e) {
		e->getComponent<TransformComponent>()->setRotations(rotation);
		return;
	}
	SAIL_LOG_WARNING("setLocalRotation called but no matching entity found");
}

void KillCamReceiverSystem::setLocalRotation(const Netcode::ComponentID id, const glm::quat& rotation) {
	if (auto e = findFromNetID(id); e) {
		e->getComponent<TransformComponent>()->setRotations(rotation);
		return;
	}
	SAIL_LOG_WARNING("setLocalRotation called but no matching entity found");
}

// SHOULD REMAIN EMPTY FOR THE KILLCAM
void KillCamReceiverSystem::setPlayerStats(Netcode::PlayerID player, int nrOfKills, int placement) {}

// SHOULD PROABABLY REMAIN EMPTY FOR THE KILLCAM
void KillCamReceiverSystem::updateSanity(const Netcode::ComponentID id, const float sanity) {}

void KillCamReceiverSystem::updateProjectile(const Netcode::ComponentID id, const glm::vec3& pos, const glm::vec3& vel) {
	if (auto e = findFromNetID(id); e) {
		e->getComponent<TransformComponent>()->setTranslation(pos);
		e->getComponent<MovementComponent>()->velocity = vel;
		return;
	}
	SAIL_LOG_WARNING("updateProjectile called but no matching entity found");
}

// If I requested the projectile it has a local owner
void KillCamReceiverSystem::spawnProjectile(const ProjectileInfo& info) {
	auto e = ECS::Instance()->createEntity("projectile");
	instantAddEntity(e.get());

	EntityFactory::ProjectileArguments args{};
	args.pos = info.position;
	args.velocity = info.velocity;
	args.ownersNetId = info.ownerID;
	args.netCompId = info.projectileID;
	args.lifetime *= SLOW_MO_MULTIPLIER;

	EntityFactory::CreateReplayProjectile(e, args);

	// Start the slow motion when the killing projectile spawns
	if (info.projectileID == m_idOfKillingProjectile) {
		m_slowMotionState = SlowMotionSetting::ENABLE;
		m_trackingProjectile = true;
		m_killerProjectile = e.get();
	}
}

void KillCamReceiverSystem::submitWaterPoint(const glm::vec3& point) {
}

void KillCamReceiverSystem::waterHitPlayer(const Netcode::ComponentID id, const Netcode::ComponentID killerID) {
	//EventDispatcher::Instance().emit(WaterHitPlayerEvent(id, senderId));
}



// AUDIO

void KillCamReceiverSystem::playerJumped(const Netcode::ComponentID id) {
	//EventDispatcher::Instance().emit(PlayerJumpedEvent(id));
}

void KillCamReceiverSystem::playerLanded(const Netcode::ComponentID id) {
	//EventDispatcher::Instance().emit(PlayerLandedEvent(id));
}


void KillCamReceiverSystem::shootStart(const Netcode::ComponentID id, float frequency) {
	// Only called when another player shoots
	//EventDispatcher::Instance().emit(StartShootingEvent(id));
}

void KillCamReceiverSystem::shootLoop(const Netcode::ComponentID id, float frequency) {
	// Only called when another player shoots
	//if (auto e = findFromNetID(id); e) {
	//	e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_START].isPlaying = false;
	//	e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].isPlaying = true;
	//	e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].playOnce = true;
	//	return;
	//}
	//SAIL_LOG_WARNING("shootLoop called but no matching entity found");
}

void KillCamReceiverSystem::shootEnd(const Netcode::ComponentID id, float frequency) {
	// Only called when another player shoots
	//EventDispatcher::Instance().emit(StopShootingEvent(id));
}

void KillCamReceiverSystem::runningMetalStart(const Netcode::ComponentID id) {
	//EventDispatcher::Instance().emit(ChangeWalkingSoundEvent(id, Audio::SoundType::RUN_METAL));
}

void KillCamReceiverSystem::runningWaterMetalStart(const Netcode::ComponentID id) {
	//EventDispatcher::Instance().emit(ChangeWalkingSoundEvent(id, Audio::SoundType::RUN_METAL));
}

void KillCamReceiverSystem::runningTileStart(const Netcode::ComponentID id) {
	//EventDispatcher::Instance().emit(ChangeWalkingSoundEvent(id, Audio::SoundType::RUN_TILE));
}

void KillCamReceiverSystem::runningWaterTileStart(const Netcode::ComponentID id) {
	//EventDispatcher::Instance().emit(ChangeWalkingSoundEvent(id, Audio::SoundType::RUN_TILE));
}

void KillCamReceiverSystem::runningStopSound(const Netcode::ComponentID id) {
	//EventDispatcher::Instance().emit(StopWalkingEvent(id));
}

void KillCamReceiverSystem::throwingStartSound(const Netcode::ComponentID id) {
	//EventDispatcher::Instance().emit(StartThrowingEvent(id));
}

void KillCamReceiverSystem::throwingEndSound(const Netcode::ComponentID id) {
	//EventDispatcher::Instance().emit(StopThrowingEvent(id));
}



// These functions are only used by NetworkReceiverSystemHost so their implementations are empty here
void KillCamReceiverSystem::endMatchAfterTimer(const float dt) {}
void KillCamReceiverSystem::prepareEndScreen(const Netcode::PlayerID sender, const EndScreenInfo& info) {}
void KillCamReceiverSystem::mergeHostsStats() {}



// NOT FROM SERIALIZED MESSAGES

// NOTE: This is not called on the host, since the host receives the disconnect through NWrapperHost::playerDisconnected()
void KillCamReceiverSystem::playerDisconnect(const Netcode::PlayerID playerID) 
{}


// Helper function

Entity* KillCamReceiverSystem::findFromNetID(const Netcode::ComponentID id) const {
	for (auto e : entities) {
		if (e->getComponent<ReplayReceiverComponent>()->m_id == id) {
			return e;
		}
	}
	return nullptr;
}


bool KillCamReceiverSystem::onEvent(const Event& event) {

	auto onPlayerDeath = [&](const PlayerDiedEvent& e) {
		if (Netcode::getComponentOwner(e.netIDofKilled) == m_playerID) {
			const bool wasKilledByAPlayer = (
				e.killerID != Netcode::UNINITIALIZED && 
				e.killerID != Netcode::INSANITY_COMP_ID && 
				e.killerID != Netcode::SPRINKLER_COMP_ID);

			// KillCam will only activate if we were killed by a player
			if (wasKilledByAPlayer) {
				m_idOfKillingProjectile = e.killerID;
			}
		}
	};

	auto onToggleSlowMotion = [&](const ToggleSlowMotionReplayEvent& e) {
		m_slowMotionState = e.setting;
	};


	switch (event.type) {
	case Event::Type::PLAYER_DEATH:       onPlayerDeath((const PlayerDiedEvent&)event); break;
	case Event::Type::TOGGLE_SLOW_MOTION: onToggleSlowMotion((const ToggleSlowMotionReplayEvent&)event); break;
	default: break;
	}

	return true;
}