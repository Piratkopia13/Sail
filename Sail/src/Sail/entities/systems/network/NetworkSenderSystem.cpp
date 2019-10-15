#include "pch.h"
#include "NetworkSenderSystem.h"

#include "Sail/entities/components/NetworkSenderComponent.h"
#include "Sail/entities/components/OnlineOwnerComponent.h"
#include "Sail/entities/Entity.h"

#include "Network/NWrapperSingleton.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"

#include "../src/Network/NWrapperSingleton.h"

#include <vector>

NetworkSenderSystem::NetworkSenderSystem() : BaseComponentSystem() {
	registerComponent<NetworkSenderComponent>(true, true, true);
	registerComponent<TransformComponent>(false, true, false);
}

NetworkSenderSystem::~NetworkSenderSystem() {
	while (eventQueue.size() > 0) {
		NetworkSenderEvent* pEvent = eventQueue.front();
		eventQueue.pop();
		delete pEvent;
	}
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

	// TODO: Add game tick here in the future

	// -+-+-+-+-+-+-+-+ Per-frame sends to per-frame recieves via components -+-+-+-+-+-+-+-+ 
	// Write nrOfEntities
	ar(static_cast<__int32>(entities.size()));

	for (auto e : entities) {
		NetworkSenderComponent* nsc = e->getComponent<NetworkSenderComponent>();
		ar(nsc->m_id);										// NetworkObjectID
		ar(nsc->m_entityType);								// Entity type
		ar(static_cast<__int32>(nsc->m_dataTypes.size()));	// NrOfMessages

		// Per type of data
		for (auto& messageType : nsc->m_dataTypes) {
			ar(messageType);								// Current MessageType

			handleEvent(messageType, e, &ar);				// Add to archive depending on the message
		}
	}

	// -+-+-+-+-+-+-+-+ Per-instance events via eventQueue -+-+-+-+-+-+-+-+ 
	unsigned __int32 queueSize = static_cast<__int32>(eventQueue.size());
	ar(queueSize);

	while (eventQueue.empty() == false) {
		NetworkSenderEvent* pE = eventQueue.front();		// Fetch
		handleEvent(pE, &ar);								// Deal with
		eventQueue.pop();									// Pop
		delete pE;											// Delete
	}

	// send the serialized archive over the network
	std::string binaryData = os.str();
	if (NWrapperSingleton::getInstance().isHost()) {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataAllClients(binaryData);
	}
	else {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataToHost(binaryData);
	}
}

const void NetworkSenderSystem::queueEvent(NetworkSenderEvent* type) {
	this->eventQueue.push(type);
}

void NetworkSenderSystem::initWithPlayerID(unsigned char playerID) {
	m_playerID = playerID;
}

void NetworkSenderSystem::addEntityToListONLYFORNETWORKRECIEVER(Entity* e) {
	entities.push_back(e);
}

void NetworkSenderSystem::handleEvent(Netcode::MessageType& messageType, Entity* e, cereal::PortableBinaryOutputArchive* ar) {
	// Package it depending on the type
	switch (messageType) {
		// Send necessary info to create the networked entity 
	case Netcode::MessageType::CREATE_NETWORKED_ENTITY:
	{
		TransformComponent* t = e->getComponent<TransformComponent>();
		Archive::archiveVec3(*ar, t->getTranslation()); // Send translation

		// After the remote entity has been created we'll want to be able to modify its transform
		messageType = Netcode::MODIFY_TRANSFORM;
	}
	break;
	case Netcode::MessageType::MODIFY_TRANSFORM:
	{
		TransformComponent* t = e->getComponent<TransformComponent>();
		Archive::archiveVec3(*ar, t->getTranslation()); // Send translation
	}
	break;
	case Netcode::MessageType::ROTATION_TRANSFORM:
	{
		TransformComponent* t = e->getComponent<TransformComponent>();
		Archive::archiveVec3(*ar, t->getRotations());	// Send rotation
	}
	break;
	default:
		break;
	}
}

void NetworkSenderSystem::handleEvent(NetworkSenderEvent* event, cereal::PortableBinaryOutputArchive* ar) {
	(*ar)(event->type); // Send the event-type
	(*ar)(static_cast<unsigned __int32>(m_playerID));
	
	switch (event->type) {
	case Netcode::MessageType::SPAWN_PROJECTILE:
	{
		Netcode::MessageDataProjectile* data = dynamic_cast<Netcode::MessageDataProjectile*>(event->data);

		Archive::archiveVec3(*ar, data->translation);
		Archive::archiveVec3(*ar, data->velocity);

		delete data;
	}
	break;
	case Netcode::MessageType::PLAYER_JUMPED: 
	{
		// it is easy to deduce which player jumped based on who sent the first message
		// No need to send additional info here.
	}
	break;
	case Netcode::MessageType::WATER_HIT_PLAYER:
	{
		Netcode::MessageDataWaterHitPlayer* data = dynamic_cast<Netcode::MessageDataWaterHitPlayer*>(event->data);
 		
		(*ar)(data->playerWhoWasHitID);	

		delete data;
	}
	break;
	case Netcode::MessageType::PLAYER_DIED:
	{
		// Send additional info if needed
	}
	break;

	default:
		break;
	}
}
