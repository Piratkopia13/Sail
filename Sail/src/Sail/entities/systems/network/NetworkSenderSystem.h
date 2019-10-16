#pragma once

#include "../BaseComponentSystem.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"
#include "Sail/netcode/NetworkedStructs.h"
// "Sail/../../libraries/cereal/archives/portable_binary.hpp"

class Entity;
class MessageType;
class NetworkSenderEvent;

class NetworkSenderSystem : public BaseComponentSystem {
public:
	NetworkSenderSystem();
	~NetworkSenderSystem();
	void update();

	const void queueEvent(NetworkSenderEvent* event);
	
	virtual void stop();

	void addEntityToListONLYFORNETWORKRECIEVER(Entity* e);
	void initWithPlayerID(unsigned char playerID);
	void addEntityToListONLYFORNETWORKRECIEVER(Entity*);

private:
	Netcode::NetworkObjectID m_playerID;

	void handleEvent(Netcode::MessageType& messageType, Entity* e, cereal::PortableBinaryOutputArchive* ar);
	void handleEvent(NetworkSenderEvent* event, cereal::PortableBinaryOutputArchive* ar);
	std::queue<NetworkSenderEvent*> eventQueue;


	//void archiveData(Netcode::MessageType* type, Entity* e, cereal::PortableBinaryOutputArchive* ar);
};