#include "pch.h"
#include "NetworkReceiverSystem.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/NetworkReceiverComponent.h"

#include "Network/NWrapperSingleton.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"

NetworkReceiverSystem::NetworkReceiverSystem() {

}

NetworkReceiverSystem::~NetworkReceiverSystem() {

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
	auto e = ECS::Instance()->createEntity(std::string("NetcodedEntity " + id));
	e->addComponent<NetworkReceiverComponent>(id, entityType);

	//switch (entityType) {

	//}
	//

	// TODO: Manually add entity to this system, don't wait for ECS


}

// Might need some optimization if we have a lot of networked entities
void NetworkReceiverSystem::setEntityTranslation(Netcode::NetworkObjectID id, const glm::vec3& translation) {
	for (auto& e : entities) {
		if (e->getComponent<NetworkReceiverComponent>()->m_id == id) {
			e->getComponent<TransformComponent>()->setTranslation(translation);
			break;
		}
	}
}

