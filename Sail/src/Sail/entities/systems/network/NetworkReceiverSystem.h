#pragma once

#include "../BaseComponentSystem.h"
#include "Sail/netcode/NetworkedStructs.h"

class NetworkReceiverSystem : BaseComponentSystem {
public:
	NetworkReceiverSystem();
	~NetworkReceiverSystem();
	void update(float dt) override;

	void pushDataToBuffer(std::string data);
private:
	// FIFO container of serialized strings to decode
	std::queue<std::string> m_incomingDataBuffer;
	std::mutex m_bufferLock;

	void createEntity(Netcode::NetworkObjectID id, Netcode::NetworkEntityType entityType, const glm::vec3& translation);
	void setEntityTranslation(Netcode::NetworkObjectID id, const glm::vec3& translation);
};