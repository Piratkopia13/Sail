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
	void update(float dt) override;

	const void queueEvent(NetworkSenderEvent* event);
	void addEntityToListONLYFORNETWORKRECIEVER(Entity*);

private:
	void handleEvent(Netcode::MessageType& messageType, Entity* e, cereal::PortableBinaryOutputArchive* ar);
	void handleEvent(NetworkSenderEvent* event, cereal::PortableBinaryOutputArchive* ar);
	std::queue<NetworkSenderEvent*> eventQueue;

	//void archiveData(Netcode::MessageType* type, Entity* e, cereal::PortableBinaryOutputArchive* ar);
};