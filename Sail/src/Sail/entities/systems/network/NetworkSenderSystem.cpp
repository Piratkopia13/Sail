#include "pch.h"
#include "NetworkSenderSystem.h"

#include "Sail/entities/components/NetworkSenderComponent.h"
#include "Sail/entities/Entity.h"

#include "Network/NWrapperSingleton.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"

#include <vector>

NetworkSenderSystem::NetworkSenderSystem() : BaseComponentSystem() {
	registerComponent<NetworkSenderComponent>(true, true, true);
	registerComponent<TransformComponent>(false, true, false);
}

NetworkSenderSystem::~NetworkSenderSystem() {

}

/*
  The construction of messages needs to match how the NetworkReceiverSystem parses them so
  any changes made here needs to be made there as well!

  Logical structure of the package that will be sent by this function:

	__int32         nrOfEntities

		NetworkObjectID entity[0].id
		EntityType		entity[0].type
		__int32			nrOfMessages
			MessageType     entity[0].messageType
			MessageData     entity[0].data

		NetworkObjectID entity[0].id
		__int32			nrOfMessages
			MessageType     entity[0].messageType
			MessageData     entity[0].data

		NetworkObjectID entity[0].id
		__int32			nrOfMessages
			MessageType     entity[0].messageType
			MessageData     entity[0].data
		....

*/
void NetworkSenderSystem::update(float dt) {
	using namespace Netcode;

	// Loop through networked entities and serialize their data
	std::ostringstream os(std::ios::binary);
	cereal::PortableBinaryOutputArchive ar(os);

	// Write nrOfEntities
	ar(static_cast<__int32>(entities.size()));

	// TODO: Add game tick here in the future

	// Per entity with sender component
	for (auto e : entities) {
		NetworkSenderComponent* nsc = e->getComponent<NetworkSenderComponent>();
		ar(nsc->m_id);										// NetworkObjectID
		ar(nsc->m_entityType);								// Entity type
		ar(static_cast<__int32>(nsc->m_dataTypes.size()));	// NrOfMessages

		// Per type of data
		for (auto& messageType : nsc->m_dataTypes) {
			ar(messageType);										// Current MessageType

			// Package it depending on the type
			switch (messageType) {
				// Send necessary info to create the networked entity 
			case Netcode::MessageType::CREATE_NETWORKED_ENTITY:
			{
				TransformComponent* t = e->getComponent<TransformComponent>();
				Archive::archiveVec3(ar, t->getTranslation()); // Send translation

				// After the remote entity has been created we'll want to be able to modify its transform
				messageType = Netcode::MODIFY_TRANSFORM;
			}
			break;
			case Netcode::MessageType::MODIFY_TRANSFORM:
			{
				TransformComponent* t = e->getComponent<TransformComponent>();
				Archive::archiveVec3(ar, t->getTranslation()); // Send translation
			}
			break;
			case Netcode::MessageType::ROTATION_TRANSFORM:
			{
				TransformComponent* t = e->getComponent<TransformComponent>();
				Archive::archiveVec3(ar, t->getRotations());	// Send rotation
			}
			break;
			case Netcode::MessageType::SPAWN_PROJECTILE:
			{
				// For projectiles, 'nsc->m_id' corresponds to the id of the entity they hit!
				TransformComponent* t = e->getComponent<TransformComponent>();
				MovementComponent* m = e->getComponent<MovementComponent>();
				Archive::archiveVec3(ar, t->getTranslation());
				Archive::archiveVec3(ar, glm::normalize(m->velocity));	// Normalize since direction is expected, not velocity

				// Remove data type from it
				e->getComponent<NetworkSenderComponent>()->removeDataType(Netcode::MessageType::SPAWN_PROJECTILE);

				// If It is empty, and has fulfilled its purpose...
				if (e->getComponent<NetworkSenderComponent>()->m_dataTypes.size() == 0) {
					e->removeComponent<NetworkSenderComponent>();
				}
			}
			break;
			case Netcode::MessageType::PLAYER_JUMPED:
			{
				// Send ENUM for 'player jumped' the ID for the entity is already appended earlier
				// ar(Netcode::MessageType::PLAYER_JUMPED); Even necessary?

				// Remove the component for this behavior once executed, as it is not per tick.
				e->getComponent<NetworkSenderComponent>()->removeDataType(Netcode::MessageType::PLAYER_JUMPED);
			}
			break;
			case Netcode::MessageType::WATER_HIT_PLAYER:
			{
				// For projectiles, 'nsc->m_id' corresponds to the id of the entity they hit!
				std::cout << nsc->m_id << " was hit by me!\n";

				// Who was hit? It was not nsc->m_id, but rather...
				// 'e' is the projectileEntity.
			
				// Remove data type from it
				NetworkSenderComponent* n = e->getComponent<NetworkSenderComponent>();
				n->removeDataType(Netcode::MessageType::WATER_HIT_PLAYER);

				// If It is empty, and has fulfilled its purpose...
				if (n->m_dataTypes.size() == 0) {
					e->removeComponent<NetworkSenderComponent>();
				}
			}
			break;
			case Netcode::MessageType::PLAYER_DIED:
			{

			}
			break;
			default:
				break;
			}
		}
	}

	std::string binaryData = os.str();

	// send the serialized archive over the network
	if (NWrapperSingleton::getInstance().isHost()) {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataAllClients(binaryData);
	}
	else {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataToHost(binaryData);
	}
}