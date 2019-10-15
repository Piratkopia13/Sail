#include "pch.h"
#include "NetworkReceiverSystem.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/NetworkReceiverComponent.h"
#include "Sail/entities/components/OnlineOwnerComponent.h"
#include "Sail/entities/components/LocalOwnerComponent.h"

#include "Network/NWrapperSingleton.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"

#include "NetworkSenderSystem.h"

// Creation of mid-air bullets from here.
#include "Sail/entities/systems/Gameplay/GunSystem.h"

NetworkReceiverSystem::NetworkReceiverSystem() : BaseComponentSystem() {
	registerComponent<NetworkReceiverComponent>(true, true, true);
	
	// Can create entities with these components:
	registerComponent<NetworkSenderComponent>(false, true, true);
	registerComponent<ModelComponent>(false, true, true);
	registerComponent<TransformComponent>(false, true, true);
	registerComponent<BoundingBoxComponent>(false, true, true);
	registerComponent<CollidableComponent>(false, true, true);
}

NetworkReceiverSystem::~NetworkReceiverSystem() 
{}

void NetworkReceiverSystem::initWithPlayerID(unsigned char playerID) {
	m_playerID = playerID;
}

void NetworkReceiverSystem::initWithPlayerEntityPointer(Entity* pPlayerEntity_) {
	m_playerEntity = pPlayerEntity_;
}

// Push incoming data strings to the back of a FIFO list
void NetworkReceiverSystem::pushDataToBuffer(std::string data) {
	std::scoped_lock lock(m_bufferLock);
	m_incomingDataBuffer.push(data);
}

void NetworkReceiverSystem::addSenderSystemP(NetworkSenderSystem* p) {
	pSenderSystem = p;
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
void NetworkReceiverSystem::update(float dt) {
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
		unsigned __int32 eventSize;
		Netcode::MessageType eventType;
		Netcode::NetworkObjectID netObjectID;
		ar(eventSize);

		for (int i = 0; i < eventSize; i++) {
			// Handle-Single-Frame events
			ar(eventType);

			if (eventType == Netcode::MessageType::PLAYER_JUMPED) {
				ar(netObjectID);

				playerJumped(netObjectID);
			} 
			else if (eventType == Netcode::MessageType::WATER_HIT_PLAYER) {	
				ar(netObjectID);

				waterHitPlayer(netObjectID);
			}
			else if (eventType == Netcode::MessageType::SPAWN_PROJECTILE) {
				Archive::loadVec3(ar, gunPosition);
				Archive::loadVec3(ar, gunVelocity);

			/*	if (NWrapperSingleton::getInstance().isHost()) {
					NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
						Netcode::MessageType::WATER_HIT_PLAYER,
						relevantEntity
					);
				}*/

				EntityFactory::CreateProjectile(gunPosition, gunVelocity);
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
	std::cout << "Someone wanted me to create an entity, ";

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

	std::cout << " and i did\n";
	
	auto e = ECS::Instance()->createEntity("ReceiverEntity");
	entities.push_back(e.get());	// Needs to be before 'addComponent' or packets might be lost.
	e->addComponent<NetworkReceiverComponent>(id, entityType);
	int test = e->getComponent<NetworkReceiverComponent>()->m_id;
	e->addComponent<OnlineOwnerComponent>(id);

	// If you are the host create a pass-through sender component to pass on the info to all players
	if (NWrapperSingleton::getInstance().isHost()) {
		// NOTE: Assumes that the data type is MODIFY_TRANSFORM, might be changed in the future
		std::cout << "I'd like for y'all to create 1 more dude.\n";
		pSenderSystem->addEntityToListONLYFORNETWORKRECIEVER(e.get());
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
	//	e->getComponent<AudioComponent>()->defineSound(SoundType::RUN, "../Audio/footsteps_1.wav", 0.94f, false);
	//	e->getComponent<AudioComponent>()->defineSound(SoundType::JUMP, "../Audio/jump.wav", 0.0f, true);

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
	// If the message was sent from me but rerouted back from the host, ignore it.
	if (static_cast<unsigned char>(id >> 18) == m_playerID) { // First byte is always the ID of the player who created the object
		return;
	}


	// Was i hit?
	if (id == m_playerEntity->getComponent<LocalOwnerComponent>()->netEntityID) {
		// Bad way of getting the candle component
		std::vector<Entity::SPtr> childEntities = m_playerEntity->getChildEntities();
		for (auto& child : childEntities) {
			if (child.get()->hasComponent<CandleComponent>()) {

				child.get()->getComponent<CandleComponent>()->hitWithWater(10.0f);
			}
		}
	}

	for (auto& e : entities) {
		NetworkReceiverComponent* n = e->getComponent<NetworkReceiverComponent>();
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {	
			// Hit player with water
			std::cout << id << " was hit by a player!\n";
			e->getComponent<CandleComponent>()->hitWithWater(10.0f);
		}
	}
}

void NetworkReceiverSystem::playerDied(Netcode::NetworkObjectID id) {
	// How do i trigger a jump from here?
	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {
			e->queueDestruction();
		}
	}
}



