#include "pch.h"
#include "NetworkReceiverSystem.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/NetworkReceiverComponent.h"

#include "Network/NWrapperSingleton.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"

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
    MessageType     entity[0].messageType
    MessageData     entity[0].data
	NetworkObjectID entity[1].id
	MessageType     entity[1].messageType
	MessageData     entity[1].data
	NetworkObjectID entity[2].id
	MessageType     entity[2].messageType
	MessageData     entity[2].data
    ....
*/
void NetworkReceiverSystem::update(float dt) {
	using namespace Netcode;

	// Don't push more data to the buffer whilst this function is running
	std::scoped_lock lock(m_bufferLock);

	NetworkObjectID id = 0;
	MessageType dataType;
	EntityType entityType;
	glm::vec3 translation;

	// Process all messages in the buffer
	while (!m_incomingDataBuffer.empty()) {
		std::istringstream is(m_incomingDataBuffer.front());
		cereal::PortableBinaryInputArchive ar(is);

		// Read message metadata
		__int32 nrOfObjectsInMessage = 0;
		{
			ar(nrOfObjectsInMessage);
		}

		// Read and process data
		for (int i = 0; i < nrOfObjectsInMessage; ++i) {
			{
				ar(id, dataType); // Get the entity ID and what type of data it is
			}

			// Read and process the data
			switch (dataType) {
				// Send necessary info to create the networked entity 
			case MessageType::CREATE_NETWORKED_ENTITY:
			{
				ar(entityType);                     // Read entity type
				Archive::loadVec3(ar, translation); // Read translation
				createEntity(id, entityType, translation);
			}
			break;
			case MessageType::MODIFY_TRANSFORM:
			{
				Archive::loadVec3(ar, translation); // Read translation
				setEntityTranslation(id, translation);
			}
			break;
			case MessageType::SPAWN_PROJECTILE:
			{
				// TODO: Spawn (or tell some system to spawn) a projectile
			}
			break;
			default:
				break;
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

	// Early exit if the entity belongs to the player 
	// (since host sends out messages to all players including the one who sent it to them)
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
	e->addComponent<NetworkReceiverComponent>(id, entityType);

	// If you are the host create a pass-through sender component to pass on the info to all players
	if (NWrapperSingleton::getInstance().isHost()) {
		// NOTE: Assumes that the data type is MODIFY_TRANSFORM, might be changed in the future
		e->addComponent<NetworkSenderComponent>(MessageType::CREATE_NETWORKED_ENTITY, entityType, id);
	}

	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet<GBufferOutShader>();
	Model* characterModel = &Application::getInstance()->getResourceManager().getModel("character1.fbx", shader);
	characterModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");
	auto* wireframeShader = &Application::getInstance()->getResourceManager().getShaderSet<WireframeShader>();
	auto boundingBoxModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f), wireframeShader);
	boundingBoxModel->getMesh(0)->getMaterial()->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));


	// create the new entity
	switch (entityType) {
	case EntityType::PLAYER_ENTITY:
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(translation);
		e->addComponent<BoundingBoxComponent>(nullptr); // TODO: Investigate crash when this is set to a bounding box model
		e->addComponent<CollidableComponent>();
		break;
	default:
		break;
	}

	// Manually add the entity to this system in case there's another message telling us to modify it, don't wait for ECS
	entities.push_back(e.get());
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

