#include "pch.h"
#include "NetworkReceiverSystem.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/NetworkReceiverComponent.h"

#include "Network/NWrapperSingleton.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"

NetworkReceiverSystem::NetworkReceiverSystem() : BaseComponentSystem() {
	// As of right now this system can create entities so don't run it in parallel with other systems
	registerComponent<NetworkReceiverComponent>(true, true, true);
}

NetworkReceiverSystem::~NetworkReceiverSystem() {

}

void NetworkReceiverSystem::initWithPlayerID(unsigned char playerID) {
	m_playerID = playerID;
}

// Needs to match how the NetworkSenderSystem updates
void NetworkReceiverSystem::update(float dt) {
	using namespace Netcode;

	NetworkObjectID id = 0;
	NetworkDataType dataType;
	NetworkEntityType entityType;
	glm::vec3 translation;


	//std::istringstream is(std::ios::binary);
	std::scoped_lock lock(m_bufferLock);

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
				// Get the entity ID and what type of data it is
				ar(id, dataType);
			}

			// Read and process the data
			switch (dataType) {
				// Send necessary info to create the networked entity 
			case NetworkDataType::CREATE_NETWORKED_ENTITY:
			{
				// Read entity type and translation
				ar(entityType);
				Archive::loadVec3(ar, translation);
				createEntity(id, entityType, translation);
			}
			break;
			case NetworkDataType::MODIFY_TRANSFORM:
			{
				Archive::loadVec3(ar, translation);
				setEntityTranslation(id, translation);
			}
			break;
			case NetworkDataType::SPAWN_PROJECTILE:
			{
				// TODO: Spawn (or tell some system to spawn) a projectile
			}
			break;
			default:
			{

			}
			break;
			}
		}

		m_incomingDataBuffer.pop();
	}

}

void NetworkReceiverSystem::pushDataToBuffer(std::string data) {
	std::scoped_lock lock(m_bufferLock);
	m_incomingDataBuffer.push(data);
}

void NetworkReceiverSystem::createEntity(Netcode::NetworkObjectID id, Netcode::NetworkEntityType entityType, const glm::vec3& translation) {
	using namespace Netcode;

	// Early exit if the entity belongs to the player (since host sends out messages to all players including the one who sent it to them)
	if (static_cast<unsigned char>(id >> 18) == m_playerID) {
		return;
	}

	// Early exit if the entity already exists
	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {
			return;
		}
	}
	
	//auto e = ECS::Instance()->createEntity(std::string("NetcodedEntity " + id));
	auto e = ECS::Instance()->createEntity("ReceiverEntity");
	e->addComponent<NetworkReceiverComponent>(id, entityType);

	// If you are the host create a sender component to pass on the info to all players
	if (NWrapperSingleton::getInstance().isHost()) {
		// Currently assumes that the data type is MODIFY_TRANSFORM, might be changed in the future
		e->addComponent<NetworkSenderComponent>(NetworkDataType::CREATE_NETWORKED_ENTITY, entityType, id);
	}


	// TODO: USE AN ENTITY FACTORY INSTEAD
	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet<GBufferOutShader>();
	Model* characterModel = &Application::getInstance()->getResourceManager().getModel("character1.fbx", shader);
	characterModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");
	auto* wireframeShader = &Application::getInstance()->getResourceManager().getShaderSet<WireframeShader>();

	auto boundingBoxModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f), wireframeShader);
	boundingBoxModel->getMesh(0)->getMaterial()->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));


	// create the new entity
	switch (entityType) {
	case NetworkEntityType::PLAYER_ENTITY:
		// TODO: use some entity factory to make the creation of entities unified
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(translation);
		//e->addComponent<BoundingBoxComponent>(boundingBoxModel.get());
		e->addComponent<BoundingBoxComponent>(nullptr); // TODO: Use a bounding box
		//e->getComponent<BoundingBoxComponent>()->getBoundingBox()->setHalfSize(glm::vec3(0.7f, .9f, 0.7f));
		e->addComponent<CollidableComponent>();
		break;
	default:
		break;
	}

	// Manually add entity to this system, don't wait for ECS
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

