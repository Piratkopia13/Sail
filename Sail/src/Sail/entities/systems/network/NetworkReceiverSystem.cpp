#include "pch.h"
#include "NetworkReceiverSystem.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/NetworkReceiverComponent.h"
#include "Sail/entities/systems/network/NetworkSenderSystem.h"
#include "Sail/entities/components/OnlineOwnerComponent.h"
#include "Sail/entities/components/LocalOwnerComponent.h"
#include "../SPLASH/src/game/states/GameState.h"

#include "Network/NWrapperSingleton.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"

// Creation of mid-air bullets from here.
#include "Sail/entities/systems/Gameplay/GunSystem.h"

NetworkReceiverSystem::NetworkReceiverSystem() : BaseComponentSystem() {
	registerComponent<NetworkReceiverComponent>(true, true, true);
	registerComponent<TransformComponent>(false, true, true);
}

NetworkReceiverSystem::~NetworkReceiverSystem() 
{}

void NetworkReceiverSystem::init(unsigned char playerID, GameState* gameStatePtr, NetworkSenderSystem* netSendSysPtr) {
	m_playerID = playerID;
	m_gameStatePtr = gameStatePtr;
	m_netSendSysPtr = netSendSysPtr;
}

void NetworkReceiverSystem::initPlayer(Entity* pPlayerEntity) {
	m_playerEntity = pPlayerEntity;
}

// Push incoming data strings to the back of a FIFO list
void NetworkReceiverSystem::pushDataToBuffer(std::string data) {
	std::scoped_lock lock(m_bufferLock);
	m_incomingDataBuffer.push(data);
}


/*
  The parsing of messages needs to match how the NetworkSenderSystem constructs them so
  any changes made here needs to be made there as well!

  Logical structure of the packages that will be decoded by this function:

	__int32         nrOfEntities

		NetworkObjectID entity[0].id
		EntityType		entity[0].type
		__int32			nrOfMessages
			MessageType     entity[0].messageType
			MessageData     entity[0].data
		
		NetworkObjectID entity[0].id
		EntityType		entity[0].type
		__int32			nrOfMessages
			MessageType     entity[0].messageType
			MessageData     entity[0].data

		NetworkObjectID entity[0].id
		EntityType		entity[0].type
		__int32			nrOfMessages
			MessageType     entity[0].messageType
			MessageData     entity[0].data
		....
		
*/
void NetworkReceiverSystem::update() {
	using namespace Netcode;

	// Don't push more data to the buffer whilst this function is running
	std::scoped_lock lock(m_bufferLock);

	__int32 nrOfObjectsInMessage = 0;
	NetworkObjectID id = 0;
	Netcode::MessageType messageType;
	Netcode::EntityType entityType;
	__int32 messageTypesSize = 0;
	glm::vec3 translation;
	glm::vec3 rotation;
	glm::vec3 gunPosition;
	glm::vec3 gunVelocity;
	
	// Process all messages in the buffer
	while (!m_incomingDataBuffer.empty()) {
		std::istringstream is(m_incomingDataBuffer.front());
		cereal::PortableBinaryInputArchive ar(is);

		// Read message metadata
		ar(nrOfObjectsInMessage);

		// Read and process data
		for (int i = 0; i < nrOfObjectsInMessage; ++i) {
			ar(id);				// NetworkObject-ID
			ar(entityType);		//
			ar(messageTypesSize);	//

			// Read per data type
			for (int j = 0; j < messageTypesSize; j++) {
				ar(messageType);

				// Read and process the data
				switch (messageType) {
					// Send necessary info to create the networked entity 
				case Netcode::MessageType::CREATE_NETWORKED_ENTITY:
				{
					Archive::loadVec3(ar, translation); // Read translation
					createEntity(id, entityType, translation);
				}
				break;
				case Netcode::MessageType::MODIFY_TRANSFORM:
				{
					Archive::loadVec3(ar, translation); // Read translation
					setEntityTranslation(id, translation);

				}
				break;
				case Netcode::MessageType::ROTATION_TRANSFORM:
				{
					Archive::loadVec3(ar, rotation);	// Read rotation
					setEntityRotation(id, rotation);
				}
				break;
				default:
					break;
				}
			}
		}


		// Recieve 'one-time' events
		__int32 eventSize;
		Netcode::MessageType eventType;
		__int32 netObjectID;
		ar(eventSize);

		for (int i = 0; i < eventSize; i++) {

			std::cout << "EVENT SIZE: " << eventSize << "\n";
			// Handle-Single-Frame events
			ar(eventType);

			// NEW STUFF
		//	ar(netObjectID);

			/* READ ALL DATA */

			/* EARLY EXIT */

			/* */

			if (eventType == Netcode::MessageType::PLAYER_JUMPED) {
			//	playerJumped(netObjectID);
			} 
			else if (eventType == Netcode::MessageType::WATER_HIT_PLAYER) {	
			//	if (static_cast<unsigned char>(netObjectID >> 18) != m_playerID) {

					ar(netObjectID);	// Find out which player was hit

					// CURRENT SPRINT IS FOR 2-PLAYER MULTIPLAYER ONLY.
			//		if (NWrapperSingleton::getInstance().isHost()) {
			//			NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
			//				Netcode::MessageType::WATER_HIT_PLAYER,
			//				SAIL_NEW Netcode::MessageDataWaterHitPlayer{
			//					netObjectID
			//				}
			//			);
			//		}

					waterHitPlayer(netObjectID);
			//	}
			}
			else if (eventType == Netcode::MessageType::SPAWN_PROJECTILE) {
				// If the message was sent from me but rerouted back from the host, ignore it.
			//	unsigned char wtf = static_cast<unsigned char>(netObjectID >> 18);
			//	std::cout << "I (" << (unsigned __int32)m_playerID << ")" << " think (" << (unsigned __int32)wtf << ") sent this originally\n";
				
			//	if (static_cast<unsigned char>(netObjectID >> 18) != m_playerID) { // First byte is always the ID of the player who created the object
					
					// If it wasn't sent from me, deal with it
					Archive::loadVec3(ar, gunPosition);
					Archive::loadVec3(ar, gunVelocity);

					EntityFactory::CreateProjectile(gunPosition, gunVelocity);
			}
	
			else if (eventType == Netcode::MessageType::PLAYER_DIED) {
				ar(netObjectID);

				playerDied(netObjectID);
			}
			else if (eventType == Netcode::MessageType::MATCH_ENDED) {
				matchEnded();
			}
		}


		m_incomingDataBuffer.pop();
	}
}


/*
  Creates a new entity of the specified entity type and with a NetworkReceiverComponent attached to it

  TODO: Use an entity factory with blueprints or something like that instead of manually constructing entities here
*/
void NetworkReceiverSystem::createEntity(Netcode::NetworkObjectID id, Netcode::EntityType entityType, const glm::vec3& translation) {
	using namespace Netcode;

	// If the message was sent from me but rerouted back from the host, ignore it.
	if (static_cast<unsigned char>(id >> 18) == m_playerID) { // First byte is always the ID of the player who created the object
		return;
	}

	// Early exit if the entity already exists
	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {
			return;
		}
	}
	
	auto e = ECS::Instance()->createEntity("ReceiverEntity");
	entities.push_back(e.get());	// Needs to be before 'addComponent' or packets might be lost.
	e->addComponent<NetworkReceiverComponent>(id, entityType);
	int test = e->getComponent<NetworkReceiverComponent>()->m_id;
	e->addComponent<OnlineOwnerComponent>(id);

	// If you are the host create a pass-through sender component to pass on the info to all players
	if (NWrapperSingleton::getInstance().isHost()) {
		// NOTE: Assumes that the data type is MODIFY_TRANSFORM, might be changed in the future
		std::cout << "I'd like for y'all to create 1 more dude.\n";
		m_netSendSysPtr->addEntityToListONLYFORNETWORKRECIEVER(e.get());
		e->addComponent<NetworkSenderComponent>(Netcode::MessageType::CREATE_NETWORKED_ENTITY, entityType, id);
	}

	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet<GBufferOutShader>();
	Model* characterModel = &Application::getInstance()->getResourceManager().getModel("character1.fbx", shader);
	characterModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/character1texture.tga");
	auto* wireframeShader = &Application::getInstance()->getResourceManager().getShaderSet<WireframeShader>();
	Model* lightModel = &Application::getInstance()->getResourceManager().getModel("candleExported.fbx", shader);
	lightModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/candleBasicTexture.tga");
	//Wireframe bounding box model
	Model* boundingBoxModel = &Application::getInstance()->getResourceManager().getModel("boundingBox.fbx", wireframeShader);
	boundingBoxModel->getMesh(0)->getMaterial()->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	// create the new entity
	switch (entityType) {
	case EntityType::PLAYER_ENTITY:
	{
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(translation);
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();

		// Adding audio component and adding all sounds attached to the player entity
		e->addComponent<AudioComponent>();
		// e->getComponent<AudioComponent>()->defineSound(SoundType::RUN, "../Audio/footsteps_1.wav", 0.94f, false);
		// e->getComponent<AudioComponent>()->defineSound(SoundType::JUMP, "../Audio/jump.wav", 0.0f, true);

		//creates light with model and pointlight
		auto light = ECS::Instance()->createEntity("ReceiverLight");
		light->addComponent<CandleComponent>();
		light->addComponent<ModelComponent>(lightModel);
		light->addComponent<TransformComponent>(glm::vec3(0.f, 2.f, 0.f));
		light->addComponent<BoundingBoxComponent>(boundingBoxModel);
		light->addComponent<CollidableComponent>();
		light->addComponent<OnlineOwnerComponent>(id);
		PointLight pl;
		pl.setColor(glm::vec3(0.2f, 0.2f, 0.2f));
		pl.setPosition(glm::vec3(0.2f, 0.2f + .37f, 0.2f));
		pl.setAttenuation(.0f, 0.1f, 0.02f);
		//pl.setIndex(m_currLightIndex++);
		pl.setIndex(999); // TODO: unique light index needed?
		light->addComponent<LightComponent>(pl);

		e->addChildEntity(light);
	}
	break;
	default:
		break;
	}

	// Manually add the entity to this system in case there's another message telling us to modify it, don't wait for ECS
	// --- Then we need to prevent ECS from adding all together or we'll end up with 2 instances of the same entity in the list...
	
}

// Might need some optimization (like sorting) if we have a lot of networked entities
void NetworkReceiverSystem::setEntityTranslation(Netcode::NetworkObjectID id, const glm::vec3& translation) {
	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {
			e->getComponent<TransformComponent>()->setTranslation(translation);
			break;
		}
	}
}

void NetworkReceiverSystem::setEntityRotation(Netcode::NetworkObjectID id, const glm::vec3& rotation){
	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {
			e->getComponent<TransformComponent>()->setRotations(rotation);
			break;
		}
	}
}

void NetworkReceiverSystem::playerJumped(Netcode::NetworkObjectID id) {
	// How do i trigger a jump from here?
	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {
		//	e->getComponent<AudioComponent>()->m_isPlaying[SoundType::JUMP] = true;
		}
	}
}

void NetworkReceiverSystem::waterHitPlayer(Netcode::NetworkObjectID id) {
	// If it is me who was hit
	if (id == m_playerEntity->getComponent<LocalOwnerComponent>()->netEntityID) {
		// Bad way of getting the candle component
		std::vector<Entity::SPtr> childEntities = m_playerEntity->getChildEntities();
		for (auto& child : childEntities) {
			if (child.get()->hasComponent<CandleComponent>()) {
				// Hit me
				child.get()->getComponent<CandleComponent>()->hitWithWater(10.0f);

				// If i'm host, relay message onwards
				if (NWrapperSingleton::getInstance().isHost()) {
					NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
						Netcode::MessageType::WATER_HIT_PLAYER,
						m_playerEntity
					);
				}
			}
		}
	}

	// Needs to be after the function above.
	// If the message was sent from me but rerouted back from the host, ignore it.
	//if (static_cast<unsigned char>(id >> 18) == m_playerID) { // First byte is always the ID of the player who created the object
	//	return;
	//}

	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {	
			// Bad way of getting the candle component
			std::vector<Entity::SPtr> childEntities = m_playerEntity->getChildEntities();
			for (auto& child : childEntities) {

				if (child.get()->hasComponent<CandleComponent>()) {
					// Hit player with water
					child.get()->getComponent<CandleComponent>()->hitWithWater(10.0f);
				}
			}
		}
	}
}

void NetworkReceiverSystem::playerDied(Netcode::NetworkObjectID id) {

	if (NWrapperSingleton::getInstance().isHost()) {
		NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
			Netcode::MessageType::PLAYER_DIED, nullptr);
	}

	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {
	
			e->removeDeleteAllChildren();
			// TODO: Remove all the components that can/should be removed

			e->queueDestruction();

			break; // Break because should only be one candle; stop looping!
		}
	}
}

void NetworkReceiverSystem::matchEnded() {

	if (NWrapperSingleton::getInstance().isHost()) {
		NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
			Netcode::MessageType::MATCH_ENDED, nullptr);
	}

	m_gameStatePtr->requestStackPop();
	m_gameStatePtr->requestStackPush(States::EndGame);
}

