#include "pch.h"
#include "NetworkSenderSystem.h"

#include "Sail/entities/components/NetworkSenderComponent.h"
#include "Sail/entities/components/OnlineOwnerComponent.h"
#include "Sail/entities/components/LocalOwnerComponent.h"
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
void NetworkSenderSystem::update() {
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
		eventQueue.pop();									// Pop.
		delete pE;											// Delete
	}

	// -+-+-+-+-+-+-+-+ send the serialized archive over the network -+-+-+-+-+-+-+-+ 
	std::string binaryData = os.str();
	if (NWrapperSingleton::getInstance().isHost()) {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataAllClients(binaryData);
	}
	else {
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataToHost(binaryData);
	}
}

const void NetworkSenderSystem::queueEvent(NetworkSenderEvent* type) {
	eventQueue.push(type);
}

void NetworkSenderSystem::stop() {


	using namespace Netcode;

	// Loop through networked entities and serialize their data.
	std::ostringstream os(std::ios::binary);
	cereal::PortableBinaryOutputArchive ar(os);

	// TODO: Add game tick here in the future

	// -+-+-+-+-+-+-+-+ Per-frame sends to per-frame recieves via components -+-+-+-+-+-+-+-+ 
	// Write nrOfEntities
	ar(static_cast<__int32>(0));

	// -+-+-+-+-+-+-+-+ Per-instance events via eventQueue -+-+-+-+-+-+-+-+ 
	//__int32 test = static_cast<__int32>(eventQueue.size());
	bool ended = false;
	while (eventQueue.empty() == false) {
		NetworkSenderEvent* pE = eventQueue.front();		// Fetch
		if ((pE->type == Netcode::MessageType::MATCH_ENDED || pE->type == Netcode::MessageType::SEND_ALL_BACK_TO_LOBBY) && ended == false) {
			ended = true;
			ar(static_cast<__int32>(1));
			handleEvent(pE, &ar);
		}
		eventQueue.pop();									// Pop
		delete pE;											// Delete
	}
	if (!ended) {
		ar(static_cast<__int32>(0));
	}

	else {
		// send the serialized archive over the network
		std::string binaryData = os.str();
		if (NWrapperSingleton::getInstance().isHost()) {
			NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataAllClients(binaryData);
		}
		else {
			NWrapperSingleton::getInstance().getNetworkWrapper()->sendSerializedDataToHost(binaryData);
		}
	}
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

		// When the remote entity has been created we want to update translation and rotation of that entity
		auto networkComp = e->getComponent<NetworkSenderComponent>();
		networkComp->removeDataType(Netcode::CREATE_NETWORKED_ENTITY);
		networkComp->addDataType(Netcode::MODIFY_TRANSFORM);
		networkComp->addDataType(Netcode::ROTATION_TRANSFORM);
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

	// NEW STUFF
//	(*ar)(static_cast<unsigned __int32>(m_playerID));
	Entity* e = event->pRelevantEntity;

	
	switch (event->type) {
	case Netcode::MessageType::SPAWN_PROJECTILE:
	{
		// SHOULD BE:
	//	Netcode::MessageDataProjectile* data = dynamic_cast<Netcode::MessageDataProjectile*>(event->data);
	//	Archive::archiveVec3(*ar, data->translation);
	//	Archive::archiveVec3(*ar, data->velocity);
	//	delete data;

		// CURRENTLY IS:
		GunComponent* gun = e->getComponent<GunComponent>();
		Archive::archiveVec3(*ar, gun->position);
		Archive::archiveVec3(*ar, gun->direction * gun->projectileSpeed);
	}
	break;
	case Netcode::MessageType::PLAYER_JUMPED: 
	{
		// No need to send additional info here, we(this computer) made the jump´.
	}
	break;
	case Netcode::MessageType::WATER_HIT_PLAYER:
	{
		// SHOULD BE:
	//	Netcode::MessageDataWaterHitPlayer* data = dynamic_cast<Netcode::MessageDataWaterHitPlayer*>(event->data);
	//	(*ar)(data->playerWhoWasHitID);	
	//	delete data;

		// CURRENTLY IS:
		unsigned __int32 NetObjectID;
		if (e->hasComponent<LocalOwnerComponent>()) {
			NetObjectID = e->getComponent<LocalOwnerComponent>()->netEntityID;
		}
		else {
			NetObjectID = e->getComponent<OnlineOwnerComponent>()->netEntityID;
		}
		
		(*ar)(NetObjectID);
	}
	break;
	case Netcode::MessageType::PLAYER_DIED:
	{
		// NetObjectID should be send outside of this loop.
		__int32 NetObjectID = e->getComponent<NetworkSenderComponent>()->m_id;
		(*ar)(NetObjectID); // Send
	}
	break;
	case Netcode::MessageType::PLAYER_DISCONNECT:
	{
		// NetObjectID should be send outside of this loop.
		__int32 NetObjectID = e->getComponent<NetworkSenderComponent>()->m_id;
		(*ar)(NetObjectID); // Send
	}
	break;
	case Netcode::MessageType::MATCH_ENDED:
	{
	}
	break;
	case Netcode::MessageType::SEND_ALL_BACK_TO_LOBBY:
	{
	}
	break;
	case Netcode::MessageType::CANDLE_HELD_STATE:
	{
		// For projectiles, 'nsc->m_id' corresponds to the id of the entity they hit!
		TransformComponent* t = e->getComponent<TransformComponent>();
		CandleComponent* c = e->getComponent<CandleComponent>();

		__int32 NetObjectID = e->getParent()->getComponent<NetworkSenderComponent>()->m_id;
		(*ar)(NetObjectID);
		Archive::archiveVec3(*ar, glm::vec3(c->isCarried(), 0, 0));
		Archive::archiveVec3(*ar, t->getTranslation());
	}
	break;

	default:
		break;
	}
}