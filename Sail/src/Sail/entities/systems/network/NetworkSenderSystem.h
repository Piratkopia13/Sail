#pragma once

#include "../BaseComponentSystem.h"

// "Sail/../../libraries/cereal/archives/portable_binary.hpp"

class Entity;
class MessageType;

class NetworkSenderSystem : public BaseComponentSystem {
public:
	NetworkSenderSystem();
	~NetworkSenderSystem();
	void update(float dt) override;

private:

	//void archiveData(Netcode::MessageType* type, Entity* e, cereal::PortableBinaryOutputArchive* ar);
};