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
		ar(nsc->m_id);										// Entity.Id
		ar(nsc->m_entityType);								// Entity type
		ar(static_cast<__int32>(nsc->m_dataTypes.size()));	// NrOfMessages

		// Per type of data
		for (auto& type : nsc->m_dataTypes)	{
			ar(type);
			//ar(Netcode::MessageType::ROTATION_TRANSFORM);

			// Package it depending on the type
			switch (type) {
				// Send necessary info to create the networked entity 
			case Netcode::MessageType::CREATE_NETWORKED_ENTITY:
			{
				TransformComponent* t = e->getComponent<TransformComponent>();
				Archive::archiveVec3(ar, t->getTranslation()); // Send translation

				// After the remote entity has been created we'll want to be able to modify its transform
				type = Netcode::MODIFY_TRANSFORM;
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
				// TODO: Send the information needed to spawn a projectile
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
	} else {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataToHost(binaryData);
	}
}
//
//void NetworkSenderSystem::archiveData(Netcode::MessageType* type, Entity* e, cereal::PortableBinaryOutputArchive* ar) {
//
//	// Package it depending on the type
//	switch (*type) {
//		// Send necessary info to create the networked entity 
//	case Netcode::MessageType::CREATE_NETWORKED_ENTITY:
//	{
//		TransformComponent* t = e->getComponent<TransformComponent>();
//		Archive::archiveVec3(ar, t->getTranslation()); // Send translation
//
//		// After the remote entity has been created we'll want to be able to modify its transform
//		*type = Netcode::MODIFY_TRANSFORM;
//	}
//	break;
//	case Netcode::MessageType::MODIFY_TRANSFORM:
//	{
//		TransformComponent* t = e->getComponent<TransformComponent>();
//	//	Archive::archiveVec3(ar, t->getTranslation()); // Send translation
//	}
//	break;
//	case Netcode::MessageType::ROTATION_TRANSFORM:
//	{
//		TransformComponent* t = e->getComponent<TransformComponent>();
////		Archive::archiveVec3(ar, t->getRotations());	// Send rotation
//	}
//	break;
//	case Netcode::MessageType::SPAWN_PROJECTILE:
//	{
//		// TODO: Send the information needed to spawn a projectile
//	}
//	break;
//	default:
//		break;
//	}
//}
