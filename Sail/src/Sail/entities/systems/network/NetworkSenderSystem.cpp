#include "pch.h"
#include "NetworkSenderSystem.h"

#include "Sail/entities/components/NetworkSenderComponent.h"
#include "Sail/entities/Entity.h"

#include "Network/NWrapperSingleton.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"


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
void NetworkSenderSystem::update() {
	using namespace Netcode;

	// Loop through networked entities and serialize their data
	std::ostringstream os(std::ios::binary);
	cereal::PortableBinaryOutputArchive ar(os);

	// Write metaData
	ar(static_cast<__int32>(entities.size()));

	// TODO: Add game tick here in the future

	for (auto e : entities) {
		NetworkSenderComponent* nsc = e->getComponent<NetworkSenderComponent>();
		ar(nsc->m_id, nsc->m_dataType); // Send the entity ID and what type of data it is

		switch (nsc->m_dataType) {

			// Send necessary info to create the networked entity 
		case MessageType::CREATE_NETWORKED_ENTITY:
		{
			ar(nsc->m_entityType); // Send Entity type

			TransformComponent* t = e->getComponent<TransformComponent>();
			Archive::archiveVec3(ar, t->getTranslation()); // Send translation

			// After the remote entity has been created we'll want to be able to modify its transform
			nsc->m_dataType = Netcode::MODIFY_TRANSFORM;
		}
		break;
		case MessageType::MODIFY_TRANSFORM:
		{
			TransformComponent* t = e->getComponent<TransformComponent>();
			Archive::archiveVec3(ar, t->getTranslation()); // Send translation
		}
		break;
		case MessageType::SPAWN_PROJECTILE:
		{
			// TODO: Send the information needed to spawn a projectile
		}
		break;
		default:
			break;
		}
	}

	std::string binaryData = os.str();

	// send the serialized data over the network
	if (NWrapperSingleton::getInstance().isHost()) {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataAllClients(binaryData);
	} else {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataToHost(binaryData);
	}
}
