#pragma once
#include "../BaseComponentSystem.h"
#include "Sail/netcode/NetworkedStructs.h"

// NOTE: As of right now this system can create entities
class NetworkReceiverSystem : public BaseComponentSystem {
public:
	NetworkReceiverSystem();
	~NetworkReceiverSystem();

	void initWithPlayerID(unsigned char playerID);
	void pushDataToBuffer(std::string data);

	void update(float dt = 0.0f) override;
private:
	// FIFO container of serialized data-strings to decode
	std::queue<std::string> m_incomingDataBuffer;
	std::mutex m_bufferLock;

	// The player's ID is used to prevent creation of receiver components for entities controlled by the player
	unsigned char m_playerID;

	//void processData(Netcode::MessageType dataType, Netcode::EntityType* entityType, cereal::PortableBinaryInputArchive* ar);

	void createEntity(Netcode::NetworkObjectID id, Netcode::EntityType entityType, const glm::vec3& translation);
	void setEntityTranslation(Netcode::NetworkObjectID id, const glm::vec3& translation);
	void setEntityRotation(Netcode::NetworkObjectID id, const glm::vec3& rotation);
	void playerJumped(Netcode::NetworkObjectID id);
	void waterHitPlayer(Netcode::NetworkObjectID id);
	void playerDied(Netcode::NetworkObjectID id);
};