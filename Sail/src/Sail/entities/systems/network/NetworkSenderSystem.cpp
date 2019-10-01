#include "pch.h"
#include "NetworkSenderSystem.h"

#include "Sail/entities/components/NetworkSenderComponent.h"
#include "Sail/entities/Entity.h"

#include "Network/NWrapperSingleton.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"


NetworkSenderSystem::NetworkSenderSystem() : BaseComponentSystem() {
	registerComponent<NetworkSenderComponent>(true, true, true);
	//requiredComponentTypes.push_back(NetworkSenderComponent::ID);
	//readBits |= NetworkSenderComponent::BID;
	//writeBits |= NetworkSenderComponent::BID;
}

NetworkSenderSystem::~NetworkSenderSystem() {

}

// Logical structure of the package that will be sent by this function
/*

__int32 nrOfEntities
__int32 entity[0].id
__int32 entity[0].dataType
Data    entity[0].data
__int32 entity[1].id
__int32 entity[1].dataType
Data    entity[1].data
__int32 entity[2].id
__int32 entity[2].dataType
Data    entity[2].data
....

*/

// Needs to match how the NetworkReceiverSystem updates
void NetworkSenderSystem::update(float dt) {
	using namespace Netcode;

	// Loop through networked entities and serialize their data
	std::ostringstream os(std::ios::binary);
	cereal::PortableBinaryOutputArchive ar(os);

	// Write metaData
	ar(static_cast<__int32>(entities.size()));

	// TODO: Add game tick here in the future


	for (auto e : entities) {
		NetworkSenderComponent* nsc = e->getComponent<NetworkSenderComponent>();
		ar(nsc->m_id, nsc->m_dataType);
		//ar(nsc->m_dataType);

		switch (nsc->m_dataType) {

			// Send necessary info to create the networked entity 
		case NetworkDataType::CREATE_NETWORKED_ENTITY:
		{
			// Send Entity type
			ar(nsc->m_entityType);

			// Send translation
			TransformComponent* t = e->getComponent<TransformComponent>();
			Archive::archiveVec3(ar, t->getTranslation());

			// After the remote entity has been created we'll want to track and modify it's transform
			nsc->m_dataType = Netcode::MODIFY_TRANSFORM;
		}
		break;
		case NetworkDataType::MODIFY_TRANSFORM:
		{
			TransformComponent* t = e->getComponent<TransformComponent>();
			Archive::archiveVec3(ar, t->getTranslation());
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

	std::string binaryData = os.str();

	// send the serialized data over the network
	if (NWrapperSingleton::getInstance().isHost()) {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataAllClients(binaryData);
	} else {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataToHost(binaryData);
	}
}
