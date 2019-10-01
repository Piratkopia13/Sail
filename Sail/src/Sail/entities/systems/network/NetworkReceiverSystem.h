#pragma once

#include "../BaseComponentSystem.h"
#include "Sail/netcode/NetworkedStructs.h"

class NetworkReceiverSystem : public BaseComponentSystem {
public:
	NetworkReceiverSystem();
	~NetworkReceiverSystem();
	void initWithPlayerID(unsigned char playerID);

	void update(float dt = 0.0f) override;

	void pushDataToBuffer(std::string data);
private:
	// FIFO container of serialized strings to decode
	std::queue<std::string> m_incomingDataBuffer;
	std::mutex m_bufferLock;

	unsigned char m_playerID;

	void createEntity(Netcode::NetworkObjectID id, Netcode::NetworkEntityType entityType, const glm::vec3& translation);
	void setEntityTranslation(Netcode::NetworkObjectID id, const glm::vec3& translation);
};