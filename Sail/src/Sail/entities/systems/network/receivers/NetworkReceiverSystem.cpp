#include "pch.h"
#include "NetworkReceiverSystem.h"
#include "Sail/entities/Entity.h"
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
static std::ofstream out("LogFiles/NetworkReceiverSystem.cpp.log");
#endif

// The host will now automatically forward all incoming messages to other players so
// no need to use any host-specific logic in this system.
#define BANNED(func) sorry_##func##_is_a_banned_function
#undef isHost
#define isHost() BANNED(isHost())


// TODO: register more components
NetworkReceiverSystem::NetworkReceiverSystem() : ReceiverBase() {
	registerComponent<NetworkReceiverComponent>(true, true, true);
	registerComponent<TransformComponent>(false, true, true);

	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_DISCONNECT, this);
}

NetworkReceiverSystem::~NetworkReceiverSystem() {
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_DISCONNECT, this);
}

//void NetworkReceiverSystem::init(Netcode::PlayerID playerID, NetworkSenderSystem* netSendSysPtr) {
//	m_playerID = playerID;
//	m_netSendSysPtr = netSendSysPtr;
//
//	m_gameDataTracker = &GameDataTracker::getInstance();
//}
//
//void NetworkReceiverSystem::setPlayer(Entity* player) {
//	m_playerEntity = player;
//}
//
//void NetworkReceiverSystem::setGameState(GameState* gameState) {
//	m_gameStatePtr = gameState;
//}
//
//void NetworkReceiverSystem::pushDataToBuffer(std::string data) {
//	std::scoped_lock lock(m_bufferLock);
//	m_incomingDataBuffer.push(data);
//}
//
//const std::vector<Entity*>& NetworkReceiverSystem::getEntities() const {
//	return entities;
//}

/*
  The parsing of messages needs to match how the NetworkSenderSystem constructs them so
  any changes made here needs to be made there as well!

  Logical structure of the packages that will be decoded by this function:
	--------------------------------------------------
	| PlayerID        senderID                       |
	| size_t          nrOfEntities                   |
	|     ComponentID     entity[0].id               |
	|     EntityType      entity[0].type             |
	|     size_t          nrOfMessages               |
	|         MessageType     entity[0].messageType  |
	|         MessageData     entity[0].data         |
	|         ...                                    |
	|     ComponentID     entity[1].id               |
	|     EntityType      entity[1].type             |
	|     size_t          nrOfMessages               |
	|         MessageType     entity[0].messageType	 |
	|         MessageData     entity[0].data         |
	|         ...                                    |
	|     ComponentID     entity[2].id               |
	|     EntityType      entity[2].type             |
	|     size_t          nrOfMessages               |
	|         MessageType     entity[0].messageType  |
	|         MessageData     entity[0].data         |
	|         ...                                    |
	|     ...                                        |
	| size_t          nrOfEvents                     |
	|     MessageType     eventType[0]               |
	|     EventData       eventData[0]               |
	|     ...                                        |
	| ...                                            |
	--------------------------------------------------

//*/
//void NetworkReceiverSystem::update(float dt) {
//	std::scoped_lock lock(m_bufferLock); // Don't push more data to the buffer whilst this function is running
//
//	size_t nrOfSenderComponents = 0;
//	Netcode::PlayerID senderID = 0;
//	Netcode::CompID id = 0;
//	Netcode::MessageType messageType;
//	Netcode::EntityType entityType;
//	size_t nrOfMessagesInComponent = 0;
//	glm::vec3 translation;
//	glm::vec3 rotation;
//	glm::quat rotationQuat;
//	glm::vec3 gunPosition;
//	glm::vec3 gunVelocity;
//	unsigned int animationIndex;
//	float animationTime;
//
//	// Process all messages in the buffer
//	while (!m_incomingDataBuffer.empty()) {
//		std::istringstream is(m_incomingDataBuffer.front());
//		Netcode::InArchive ar(is);
//
//		ar(senderID);
//		
//		// If the packet was originally sent over the network from ourself 
//		// then don't process it and go to the next packet
//		if (senderID == m_playerID) { m_incomingDataBuffer.pop(); continue; }
//
//		// If the message was sent internally to ourself then correct the senderID
//		if (senderID == Netcode::MESSAGE_FROM_SELF_ID) { senderID = m_playerID; }
//
//		// -+-+-+-+-+-+-+-+ Process data from senderComponents -+-+-+-+-+-+-+-+ 
//
//		ar(nrOfSenderComponents);
//		// Read and process data from SenderComponents (i.e. stuff that is continuously updated such as positions)
//		for (size_t i = 0; i < nrOfSenderComponents; ++i) {
//			ar(id);               // NetworkObject-ID
//			ar(entityType);       //
//			ar(nrOfMessagesInComponent); //
//
//			// Read per data type
//			for (size_t j = 0; j < nrOfMessagesInComponent; j++) {
//				ar(messageType);
//#if defined(DEVELOPMENT) && defined(_LOG_TO_FILE)
//				out << "ReciverComp: " << Netcode::MessageNames[(int)(messageType)-1] << "\n";
//#endif
//				// Read and process the data
//				// NOTE: Please keep this switch in alphabetical order (at least for the first word)
//				switch (messageType) {
//				case Netcode::MessageType::ANIMATION:
//				{
//					ar(animationIndex);		// Read
//					ar(animationTime);		//
//					setEntityAnimation(id, animationIndex, animationTime);
//				}
//				break;
//				case Netcode::MessageType::CHANGE_ABSOLUTE_POS_AND_ROT:
//				{
//					ArchiveHelpers::loadVec3(ar, translation);
//					ArchiveHelpers::loadQuat(ar, rotationQuat);
//
//					setEntityLocalPosition(id, translation);
//					setEntityLocalRotation(id, rotationQuat);
//				}
//				break;
//				case Netcode::MessageType::CHANGE_LOCAL_POSITION:
//				{
//					ArchiveHelpers::loadVec3(ar, translation); // Read translation
//					setEntityLocalPosition(id, translation);
//
//				}
//				break;
//				case Netcode::MessageType::CHANGE_LOCAL_ROTATION:
//				{
//					ArchiveHelpers::loadVec3(ar, rotation);	// Read rotation
//					setEntityLocalRotation(id, rotation);
//				}
//				break;
//				case Netcode::MessageType::DESTROY_ENTITY:
//				{
//					destroyEntity(id);
//				}
//				break;
//				case Netcode::MessageType::SHOOT_START:
//				{
//					ArchiveHelpers::loadVec3(ar, gunPosition);
//					ArchiveHelpers::loadVec3(ar, gunVelocity);
//
//					shootStart(gunPosition, gunVelocity, id);
//				}
//				break;
//				case Netcode::MessageType::SHOOT_LOOP:
//				{
//					ArchiveHelpers::loadVec3(ar, gunPosition);
//					ArchiveHelpers::loadVec3(ar, gunVelocity);
//
//					shootLoop(gunPosition, gunVelocity, id);
//				}
//				break;
//				case Netcode::MessageType::SHOOT_END:
//				{
//					ArchiveHelpers::loadVec3(ar, gunPosition);
//					ArchiveHelpers::loadVec3(ar, gunVelocity);
//
//					shootEnd(gunPosition, gunVelocity, id);
//				}
//				break;
//				default:
//					SAIL_LOG_ERROR("INVALID NETWORK MESSAGE RECEIVED FROM " + NWrapperSingleton::getInstance().getPlayer(senderID)->name + "\n");
//					break;
//				}
//			}
//		}
//
//
//		// Receive 'one-time' events
//		size_t nrOfEvents;
//		Netcode::MessageType eventType = Netcode::MessageType::EMPTY;
//		Netcode::MessageType lastEventType = Netcode::MessageType::EMPTY;
//
//#ifdef DEVELOPMENT
//		Netcode::MessageType REDUNDANTTYPE;
//#endif
//		Netcode::CompID componentID;
//		Netcode::PlayerID playerID;
//
//
//		// -+-+-+-+-+-+-+-+ Process events -+-+-+-+-+-+-+-+ 
//		ar(nrOfEvents);
//
//		// Read and process data from SenderComponents (i.e. stuff that is continuously updated such as positions)
//		for (size_t i = 0; i < nrOfEvents; i++) {
//
//			// Handle-Single-Frame events
//			ar(eventType);
//#ifdef DEVELOPMENT
//			ar(REDUNDANTTYPE);
//			if (eventType != REDUNDANTTYPE) {
//				SAIL_LOG_ERROR("CORRUPTED NETWORK EVENT RECEIVED\n");
//				SAIL_LOG_WARNING("Make sure that all players are in either a DEVELOPER branch or the Release branch\n");
//				m_incomingDataBuffer.pop();
//				return;
//			}
//#endif
//#if defined(DEVELOPMENT) && defined(_LOG_TO_FILE)
//			out << "Event: " << Netcode::MessageNames[(int)(eventType) - 1] << "\n";
//#endif
//
//
//			// NOTE: Please keep this switch in alphabetical order (at least for the first word)
//			switch (eventType) {
//			
//			case Netcode::MessageType::CANDLE_HELD_STATE:
//			{
//				bool isCarried;
//
//				ar(componentID);
//				ar(isCarried);
//				setCandleHeldState(componentID, isCarried);
//			}
//			break;
//			case Netcode::MessageType::CREATE_NETWORKED_PLAYER:
//			{
//				Netcode::CompID playerCompID;
//				Netcode::CompID candleCompID;
//				Netcode::CompID gunCompID;
//
//				ar(playerCompID); // Read what Netcode::ComponentID the player entity should have
//				ar(candleCompID); // Read what Netcode::ComponentID the candle entity should have
//				ar(gunCompID);    // Read what Netcode::ComponentID the gun entity should have
//				ArchiveHelpers::loadVec3(ar, translation); // Read the player's position
//				
//				createPlayerEntity(playerCompID, candleCompID, gunCompID, translation);
//			}
//			break;
//			case Netcode::MessageType::ENABLE_SPRINKLERS:
//			{
//				enableSprinklers();
//			}
//			break;
//			case Netcode::MessageType::ENDGAME_STATS:
//			{
//				// Receive player count
//				size_t nrOfPlayers;
//				ar(nrOfPlayers);
//
//				// create temporary variables to hold data when reading netmessage
//				Netcode::PlayerID pID;
//				int nKills;
//				int placement;
//
//				int bulletsFired, jumpsMade;
//				float distanceWalked;
//				Netcode::PlayerID bulletsFiredID, distanceWalkedID, jumpsMadeID;
//
//				// Get all per player data from the Host
//				for (int k = 0; k < nrOfPlayers; k++) {
//					ar(pID);
//					ar(nKills);
//					ar(placement);
//					GameDataTracker::getInstance().setStatsForPlayer(pID, nKills, placement);
//				}
//
//				// Get all specific data from the Host
//				ar(bulletsFired);
//				ar(bulletsFiredID);
//
//				ar(distanceWalked);
//				ar(distanceWalkedID);
//
//				ar(jumpsMade);
//				ar(jumpsMadeID);
//
//				GameDataTracker::getInstance().setStatsForOtherData(
//					bulletsFiredID, bulletsFired, distanceWalkedID, distanceWalked, jumpsMadeID, jumpsMade);
//
//				endMatch();
//			}
//			break;
//			case Netcode::MessageType::EXTINGUISH_CANDLE:
//			{
//				Netcode::CompID candleID;
//				Netcode::PlayerID shooterID;
//
//				ar(candleID);
//				ar(playerID);
//
//				extinguishCandle(candleID, playerID);
//			}
//			break;
//			case Netcode::MessageType::HIT_BY_SPRINKLER:
//			{
//				Netcode::CompID candleOwnerID;
//				ar(candleOwnerID);
//				hitBySprinkler(candleOwnerID);
//			}
//			break;
//			case Netcode::MessageType::IGNITE_CANDLE:
//			{
//				Netcode::CompID candleID;
//				ar(candleID);
//				igniteCandle(candleID);
//			}
//			break;
//			case Netcode::MessageType::MATCH_ENDED:
//			{
//				NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
//					Netcode::MessageType::PREPARE_ENDSCREEN,
//					SAIL_NEW Netcode::MessagePrepareEndScreen(),
//					false
//				);
//
//				GameDataTracker::getInstance().turnOffLocalDataTracking();
//
//				mergeHostsStats();
//				// Dispatch game over event
//				EventDispatcher::Instance().emit(GameOverEvent());
//			}
//			break;
//			case Netcode::MessageType::PLAYER_DIED:
//			{
//				Netcode::PlayerID playerIdOfShooter;
//				Netcode::CompID networkIdOfKilled;
//
//				ar(networkIdOfKilled); // Receive
//				ar(playerIdOfShooter);
//				playerDied(networkIdOfKilled, playerIdOfShooter);
//			}
//			break;
//			case Netcode::MessageType::PLAYER_JUMPED:
//			{
//				ar(componentID);
//				playerJumped(componentID);
//			}
//			break;
//			case Netcode::MessageType::PLAYER_LANDED:
//			{
//				ar(componentID);
//				playerLanded(componentID);
//			}
//			break;
//			case Netcode::MessageType::PREPARE_ENDSCREEN:
//			{
//				GameDataTracker* dgtp = &GameDataTracker::getInstance();
//				// create temporary variables to hold data when reading netmessage
//				int bulletsFired, jumpsMade;
//				float distanceWalked;
//				// Get the data
//				ar(bulletsFired);
//				ar(distanceWalked);
//				ar(jumpsMade);
//
//				prepareEndScreen(bulletsFired, distanceWalked, jumpsMade, senderID);
//			}
//			break;
//			case Netcode::MessageType::RUNNING_METAL_START:
//			{
//				ar(componentID);
//				runningMetalStart(componentID);
//			}
//			break;
//			case Netcode::MessageType::RUNNING_TILE_START:
//			{
//				ar(componentID);
//				runningTileStart(componentID);
//			}
//			break;
//			case Netcode::MessageType::RUNNING_STOP_SOUND:
//			{
//				ar(componentID);
//				runningStopSound(componentID);
//			}
//			break;
//			case Netcode::MessageType::SET_CANDLE_HEALTH:
//			{
//				Netcode::CompID candleID;
//				float health;
//				ar(candleID);
//				ar(health);
//				setCandleHealth(candleID, health);
//			}
//			break;
//			case Netcode::MessageType::SPAWN_PROJECTILE:
//			{
//				Netcode::CompID projectileOwnerID;
//				Netcode::CompID projectileComponentID;
//
//				ArchiveHelpers::loadVec3(ar, gunPosition);
//				ArchiveHelpers::loadVec3(ar, gunVelocity);
//				ar(projectileComponentID);
//				ar(projectileOwnerID);
//
//				projectileSpawned(gunPosition, gunVelocity, projectileComponentID, projectileOwnerID);
//			}
//			break;
//			case Netcode::MessageType::WATER_HIT_PLAYER:
//			{
//				Netcode::CompID playerwhoWasHit;
//				ar(playerwhoWasHit);
//
//				// NOTE!
//				// This function is and should be empty for the NetworkReceiverSystemClient. 
//				// Only the Host has the authority to damage candles.
//				waterHitPlayer(playerwhoWasHit, senderID);
//			}
//			break;
//			default:
//				SAIL_LOG_ERROR("INVALID NETWORK EVENT NR " + std::to_string((int)eventType) + " RECEIVED FROM" + NWrapperSingleton::getInstance().getPlayer(senderID)->name + "\n");
//				break;
//			}
//
//		}
//
//		m_incomingDataBuffer.pop();
//	}
//
//	// End game timer 
//	endMatchAfterTimer(dt);
//}


void NetworkReceiverSystem::createPlayer(const PlayerComponentInfo& info, const glm::vec3& pos) {
	// Early exit if the entity already exists
	if (findFromNetID(info.playerCompID)) {
		return;
	}

	auto e = ECS::Instance()->createEntity("networkedEntity");
	instantAddEntity(e.get());

	SAIL_LOG("Created player with id: " + std::to_string(info.playerCompID));

	// lightIndex set to 999, can probably be removed since it no longer seems to be used
	EntityFactory::CreateOtherPlayer(e, info.playerCompID, info.candleID, info.gunID, 999, pos);

}

void NetworkReceiverSystem::destroyEntity(const Netcode::CompID entityID) {
	if (auto e = findFromNetID(entityID); e) {
		e->queueDestruction();
		return;
	}
	SAIL_LOG_WARNING("destoryEntity called but no matching entity found");
}

void NetworkReceiverSystem::enableSprinklers() {
	ECS::Instance()->getSystem<SprinklerSystem>()->enableSprinklers();
}

void NetworkReceiverSystem::extinguishCandle(const Netcode::CompID candleId, const Netcode::PlayerID shooterID) {
	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == candleId) {

			e->getComponent<CandleComponent>()->wasJustExtinguished = true;
			e->getComponent<CandleComponent>()->wasHitByPlayerID = shooterID;

			return;
		}
	}
	SAIL_LOG_WARNING("extinguishCandle called but no matching candle entity found");
}

void NetworkReceiverSystem::hitBySprinkler(const Netcode::CompID candleOwnerID) {
	waterHitPlayer(candleOwnerID, Netcode::MESSAGE_SPRINKLER_ID);
}

void NetworkReceiverSystem::igniteCandle(const Netcode::CompID candleID) {
	EventDispatcher::Instance().emit(IgniteCandleEvent(candleID));
}

void NetworkReceiverSystem::playerDied(const Netcode::CompID networkIdOfKilled, const Netcode::PlayerID playerIdOfShooter) {
	if (auto e = findFromNetID(networkIdOfKilled); e) {
		EventDispatcher::Instance().emit(PlayerDiedEvent(
			e,
			m_playerEntity,
			playerIdOfShooter,
			networkIdOfKilled)
		);
		return;
	}
	SAIL_LOG_WARNING("playerDied called but no matching entity found");
}

void NetworkReceiverSystem::setAnimation(const Netcode::CompID id, const AnimationInfo& info) {
	if (auto e = findFromNetID(id); e) {
		auto animation = e->getComponent<AnimationComponent>();
		animation->setAnimation(info.index);
		animation->animationTime = info.time;
		return;
	}
	SAIL_LOG_WARNING("setAnimation called but no matching entity found");
}

void NetworkReceiverSystem::setCandleHealth(const Netcode::CompID candleId, const float health) {
	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == candleId) {
			e->getComponent<CandleComponent>()->health = health;
			return;
		}
	}
	SAIL_LOG_WARNING("setCandleHelath called but no matching candle entity found");
}

// The player who puts down their candle does this in CandleSystem and tests collisions
// The candle will be moved for everyone else in here
void NetworkReceiverSystem::setCandleState(const Netcode::CompID id, const bool isHeld) {
	EventDispatcher::Instance().emit(HoldingCandleToggleEvent(id, isHeld));
}

// Might need some optimization (like sorting) if we have a lot of networked entities
void NetworkReceiverSystem::setLocalPosition(const Netcode::CompID id, const glm::vec3& translation) {
	if (auto e = findFromNetID(id); e) {
		e->getComponent<TransformComponent>()->setTranslation(translation);
		return;
	}
	SAIL_LOG_WARNING("setLocalPosition called but no matching entity found");
}

void NetworkReceiverSystem::setLocalRotation(const Netcode::CompID id, const glm::vec3& rotation) {
	if (auto e = findFromNetID(id); e) {
		e->getComponent<TransformComponent>()->setRotations(rotation);
		return;
	}
	SAIL_LOG_WARNING("setLocalRotation called but no matching entity found");
}

void NetworkReceiverSystem::setLocalRotation(const Netcode::CompID id, const glm::quat& rotation) {
	if (auto e = findFromNetID(id); e) {
		e->getComponent<TransformComponent>()->setRotations(rotation);
		return;
	}
	SAIL_LOG_WARNING("setLocalRotation called but no matching entity found");
}

// If I requested the projectile it has a local owner
void NetworkReceiverSystem::spawnProjectile(const ProjectileInfo& info) {
	const bool wasRequestedByMe = (Netcode::getComponentOwner(info.ownerID) == m_playerID);

	// Also play the sound
	EntityFactory::CreateProjectile(info.position, info.velocity, wasRequestedByMe, info.ownerID, info.projectileID);
}

void NetworkReceiverSystem::waterHitPlayer(const Netcode::CompID id, const Netcode::PlayerID senderId) {
	EventDispatcher::Instance().emit(WaterHitPlayerEvent(id, senderId));
}



// AUDIO

void NetworkReceiverSystem::playerJumped(const Netcode::CompID id) {
	EventDispatcher::Instance().emit(PlayerJumpedEvent(id));
}

void NetworkReceiverSystem::playerLanded(const Netcode::CompID id) {
	EventDispatcher::Instance().emit(PlayerLandedEvent(id));
}


// TODO: Remove info since it's unused or are these functions not finished?
void NetworkReceiverSystem::shootStart(const Netcode::CompID id, const ShotFiredInfo& info) {
	// Only called when another player shoots
	EventDispatcher::Instance().emit(StartShootingEvent(id));
}

void NetworkReceiverSystem::shootLoop(const Netcode::CompID id, const ShotFiredInfo& info) {
	// Only called when another player shoots
	if (auto e = findFromNetID(id); e) {
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_START].isPlaying = false;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].isPlaying = true;
		e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::SHOOT_LOOP].playOnce = true;
		return;
	}
	SAIL_LOG_WARNING("shootLoop called but no matching entity found");
}

void NetworkReceiverSystem::shootEnd(const Netcode::CompID id, const ShotFiredInfo& info) {
	// Only called when another player shoots
	EventDispatcher::Instance().emit(StopShootingEvent(id));
}

void NetworkReceiverSystem::runningMetalStart(const Netcode::CompID id) {
	EventDispatcher::Instance().emit(ChangeWalkingSoundEvent(id, Audio::SoundType::RUN_METAL));
}

void NetworkReceiverSystem::runningTileStart(const Netcode::CompID id) {
	EventDispatcher::Instance().emit(ChangeWalkingSoundEvent(id, Audio::SoundType::RUN_TILE));
}

void NetworkReceiverSystem::runningStopSound(const Netcode::CompID id) {
	EventDispatcher::Instance().emit(StopWalkingEvent(id));
}



// NOT FROM SERIALIZED MESSAGES

// NOTE: This is not called on the host, since the host receives the disconnect through NWrapperHost::playerDisconnected()
void NetworkReceiverSystem::playerDisconnect(const Netcode::PlayerID playerID) {
	for (auto e : entities) {
		// Find the player entity and delete it
		if (e->hasComponent<PlayerComponent>() && Netcode::getComponentOwner(e->getComponent<NetworkReceiverComponent>()->m_id) == playerID) {
			e->removeDeleteAllChildren();
			e->queueDestruction();
			return;
		}
	}
	SAIL_LOG_WARNING("playerDisconnect called but no matching entity found");
}



// Helper function
Entity* NetworkReceiverSystem::findFromNetID(const Netcode::CompID id) const {
	for (auto e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {
			return e;
		}
	}
	return nullptr;
}


bool NetworkReceiverSystem::onEvent(const Event& event) {
	switch (event.type) {
	case Event::Type::NETWORK_DISCONNECT:		playerDisconnect(((const NetworkDisconnectEvent&)(event)).player.id); break;
	default: break;
	}

	return true;
}