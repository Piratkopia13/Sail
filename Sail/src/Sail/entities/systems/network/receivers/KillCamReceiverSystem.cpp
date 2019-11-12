#include "pch.h"
#include "KillCamReceiverSystem.h"
#include "Sail/entities/Entity.h"

// TODO: Remove unnecessary includes

#include "Sail/entities/components/NetworkReceiverComponent.h"
#include "Sail/entities/systems/network/NetworkSenderSystem.h"
#include "Sail/entities/components/OnlineOwnerComponent.h"
#include "Sail/entities/components/LocalOwnerComponent.h"
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

//#define _LOG_TO_FILE
#if defined(DEVELOPMENT) && defined(_LOG_TO_FILE)
#include <fstream>
static std::ofstream out("LogFiles/KillCamReceiverSystem.cpp.log");
#endif


// TODO: register more components
KillCamReceiverSystem::KillCamReceiverSystem() : ReceiverBase() {
	registerComponent<ReplayComponent>(true, false, false);

	//EventDispatcher::Instance().subscribe(Event::Type::NETWORK_DISCONNECT, this);
}

KillCamReceiverSystem::~KillCamReceiverSystem() {
	//EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_DISCONNECT, this);
}

void KillCamReceiverSystem::handleIncomingData(const std::string& data) {
	pushDataToBuffer(data);
}




void KillCamReceiverSystem::createPlayer(const PlayerComponentInfo& info, const glm::vec3& pos) {
	//// Early exit if the entity already exists
	//if (findFromNetID(info.playerCompID)) {
	//	return;
	//}

	//auto e = ECS::Instance()->createEntity("networkedEntity");
	//instantAddEntity(e.get());

	//SAIL_LOG("Created player with id: " + std::to_string(info.playerCompID));

	//// lightIndex set to 999, can probably be removed since it no longer seems to be used
	//EntityFactory::CreateOtherPlayer(e, info.playerCompID, info.candleID, info.gunID, 999, pos);

}

void KillCamReceiverSystem::destroyEntity(const Netcode::CompID entityID) {
	//if (auto e = findFromNetID(entityID); e) {
	//	e->queueDestruction();
	//	return;
	//}
	//SAIL_LOG_WARNING("destoryEntity called but no matching entity found");
}

void KillCamReceiverSystem::enableSprinklers() {
	//ECS::Instance()->getSystem<SprinklerSystem>()->enableSprinklers();
}

void KillCamReceiverSystem::extinguishCandle(const Netcode::CompID candleId, const Netcode::PlayerID shooterID) {
	//for (auto& e : entities) {
	//	if (e->getComponent<NetworkReceiverComponent>()->m_id == candleId) {

	//		e->getComponent<CandleComponent>()->wasJustExtinguished = true;
	//		e->getComponent<CandleComponent>()->wasHitByPlayerID = shooterID;

	//		return;
	//	}
	//}
	//SAIL_LOG_WARNING("extinguishCandle called but no matching candle entity found");
}

void KillCamReceiverSystem::hitBySprinkler(const Netcode::CompID candleOwnerID) {
	//waterHitPlayer(candleOwnerID, Netcode::MESSAGE_SPRINKLER_ID);
}

void KillCamReceiverSystem::igniteCandle(const Netcode::CompID candleID) {
	//EventDispatcher::Instance().emit(IgniteCandleEvent(candleID));
}

void KillCamReceiverSystem::playerDied(const Netcode::CompID networkIdOfKilled, const Netcode::PlayerID playerIdOfShooter) {
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

void KillCamReceiverSystem::setAnimation(const Netcode::CompID id, const AnimationInfo& info) {
	//if (auto e = findFromNetID(id); e) {
	//	auto animation = e->getComponent<AnimationComponent>();
	//	animation->setAnimation(info.index);
	//	animation->animationTime = info.time;
	//	return;
	//}
	//SAIL_LOG_WARNING("setAnimation called but no matching entity found");
}

void KillCamReceiverSystem::setCandleHealth(const Netcode::CompID candleId, const float health) {
	//for (auto& e : entities) {
	//	if (e->getComponent<NetworkReceiverComponent>()->m_id == candleId) {
	//		e->getComponent<CandleComponent>()->health = health;
	//		return;
	//	}
	//}
	//SAIL_LOG_WARNING("setCandleHelath called but no matching candle entity found");
}

// The player who puts down their candle does this in CandleSystem and tests collisions
// The candle will be moved for everyone else in here
void KillCamReceiverSystem::setCandleState(const Netcode::CompID id, const bool isHeld) {
	//EventDispatcher::Instance().emit(HoldingCandleToggleEvent(id, isHeld));
}

// Might need some optimization (like sorting) if we have a lot of networked entities
void KillCamReceiverSystem::setLocalPosition(const Netcode::CompID id, const glm::vec3& translation) {
	//if (auto e = findFromNetID(id); e) {
	//	e->getComponent<TransformComponent>()->setTranslation(translation);
	//	return;
	//}
	//SAIL_LOG_WARNING("setLocalPosition called but no matching entity found");
}

void KillCamReceiverSystem::setLocalRotation(const Netcode::CompID id, const glm::vec3& rotation) {
	//if (auto e = findFromNetID(id); e) {
	//	e->getComponent<TransformComponent>()->setRotations(rotation);
	//	return;
	//}
	//SAIL_LOG_WARNING("setLocalRotation called but no matching entity found");
}

void KillCamReceiverSystem::setLocalRotation(const Netcode::CompID id, const glm::quat& rotation) {
	//if (auto e = findFromNetID(id); e) {
	//	e->getComponent<TransformComponent>()->setRotations(rotation);
	//	return;
	//}
	//SAIL_LOG_WARNING("setLocalRotation called but no matching entity found");
}

// If I requested the projectile it has a local owner
void KillCamReceiverSystem::spawnProjectile(const ProjectileInfo& info) {
	//const bool wasRequestedByMe = (Netcode::getComponentOwner(info.ownerID) == m_playerID);

	//// Also play the sound
	//EntityFactory::CreateProjectile(info.position, info.velocity, wasRequestedByMe, info.ownerID, info.projectileID);
}

void KillCamReceiverSystem::waterHitPlayer(const Netcode::CompID id, const Netcode::PlayerID senderId) {
	//EventDispatcher::Instance().emit(WaterHitPlayerEvent(id, senderId));
}



// AUDIO

void KillCamReceiverSystem::playerJumped(const Netcode::CompID id) {
	//EventDispatcher::Instance().emit(PlayerJumpedEvent(id));
}

void KillCamReceiverSystem::playerLanded(const Netcode::CompID id) {
	//EventDispatcher::Instance().emit(PlayerLandedEvent(id));
}


// TODO: Remove info since it's unused or are these functions not finished?
void KillCamReceiverSystem::shootStart(const Netcode::CompID id, const ShotFiredInfo& info) {
	// Only called when another player shoots
	//EventDispatcher::Instance().emit(StartShootingEvent(id));
}

void KillCamReceiverSystem::shootLoop(const Netcode::CompID id, const ShotFiredInfo& info) {
	// Only called when another player shoots
	//if (auto e = findFromNetID(id); e) {
	//	e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_START].isPlaying = false;
	//	e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].isPlaying = true;
	//	e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].playOnce = true;
	//	return;
	//}
	//SAIL_LOG_WARNING("shootLoop called but no matching entity found");
}

void KillCamReceiverSystem::shootEnd(const Netcode::CompID id, const ShotFiredInfo& info) {
	// Only called when another player shoots
	//EventDispatcher::Instance().emit(StopShootingEvent(id));
}

void KillCamReceiverSystem::runningMetalStart(const Netcode::CompID id) {
	//EventDispatcher::Instance().emit(ChangeWalkingSoundEvent(id, Audio::SoundType::RUN_METAL));
}

void KillCamReceiverSystem::runningTileStart(const Netcode::CompID id) {
	//EventDispatcher::Instance().emit(ChangeWalkingSoundEvent(id, Audio::SoundType::RUN_TILE));
}

void KillCamReceiverSystem::runningStopSound(const Netcode::CompID id) {
	//EventDispatcher::Instance().emit(StopWalkingEvent(id));
}



// These functions are only used by NetworkReceiverSystemHost so their implementations are empty here
void KillCamReceiverSystem::endMatch() {}
void KillCamReceiverSystem::endMatchAfterTimer(const float dt) {}
void KillCamReceiverSystem::prepareEndScreen(const Netcode::PlayerID sender, const EndScreenInfo& info) {}
void KillCamReceiverSystem::mergeHostsStats() {}



// NOT FROM SERIALIZED MESSAGES

// NOTE: This is not called on the host, since the host receives the disconnect through NWrapperHost::playerDisconnected()
void KillCamReceiverSystem::playerDisconnect(const Netcode::PlayerID playerID) 
{}


// Helper function

// TODO: use something other than networkReceiverComponent ?
Entity* KillCamReceiverSystem::findFromNetID(const Netcode::CompID id) const {
	//for (auto e : entities) {
	//	if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {
	//		return e;
	//	}
	//}
	return nullptr;
}


bool KillCamReceiverSystem::onEvent(const Event& event) {
	switch (event.type) {
	default: 
		break;
	}

	return true;
}