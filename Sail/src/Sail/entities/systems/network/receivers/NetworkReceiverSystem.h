#pragma once

#include "../../BaseComponentSystem.h"
#include "Sail/netcode/NetworkedStructs.h"
#include "Sail/events/EventReceiver.h"

class GameState;
class NetworkSenderSystem;
class GameDataTracker;

class NetworkReceiverSystem : public BaseComponentSystem, public EventReceiver {
public:
	NetworkReceiverSystem();
	~NetworkReceiverSystem();

	virtual void handleIncomingData(std::string data) = 0;
	void pushDataToBuffer(std::string);

	void init(Netcode::PlayerID playerID, GameState* gameStatePtr, NetworkSenderSystem* netSendSysPtr);
	const std::vector<Entity*>& getEntities() const;

	void update(float dt);

#ifdef DEVELOPMENT
	void imguiPrint(Entity** selectedEntity = nullptr) {

		ImGui::Text(std::string("ID: " + std::to_string((int)m_playerID)).c_str());
	}
#endif

protected:


	GameState* m_gameStatePtr;
	NetworkSenderSystem* m_netSendSysPtr;
	GameDataTracker* m_gameDataTracker;

	// FIFO container of serialized data-strings to decode
	std::queue<std::string> m_incomingDataBuffer;
	std::mutex m_bufferLock;

	// The player's ID is used to prevent creation of receiver components for entities controlled by the player
	Netcode::PlayerID m_playerID;
private:
	void createPlayerEntity(Netcode::ComponentID playerCompID, Netcode::ComponentID candleCompID, Netcode::ComponentID gunCompID, const glm::vec3& translation);
	void setEntityLocalPosition(Netcode::ComponentID id, const glm::vec3& translation);
	void setEntityLocalRotation(Netcode::ComponentID id, const glm::vec3& rotation);
	void setEntityLocalRotation(Netcode::ComponentID id, const glm::quat& rotation);
	void setEntityAnimation(Netcode::ComponentID id, unsigned int animationIndex, float animationTime);
	void setCandleHealth(Netcode::ComponentID candleId, float health);
	void extinguishCandle(Netcode::ComponentID candleId, Netcode::PlayerID shooterID);
	void playerJumped(Netcode::ComponentID id);
	void playerLanded(Netcode::ComponentID id);
	void projectileSpawned(glm::vec3& pos, glm::vec3 vel, Netcode::ComponentID ownerID);
	void playerDied(Netcode::ComponentID id, Netcode::PlayerID shooterID);
	void playerDisconnect(Netcode::PlayerID playerID);
	void setCandleHeldState(Netcode::ComponentID id, bool isHeld);
	void igniteCandle(Netcode::ComponentID candleID);

	void shootStart(glm::vec3& gunPos, glm::vec3& gunVel, Netcode::ComponentID id);
	void shootLoop(glm::vec3& gunPos, glm::vec3& gunVel, Netcode::ComponentID id);
	void shootEnd(glm::vec3& gunPos, glm::vec3& gunVel, Netcode::ComponentID id);

	virtual void waterHitPlayer(Netcode::ComponentID id, Netcode::PlayerID SenderId) = 0;
	virtual void endMatch() = 0;                   // Start end timer for host
	virtual void endMatchAfterTimer(float dt) = 0; // Made for the host to quit the game after a set time
	virtual void mergeHostsStats() = 0;            // Host adds its data to global statistics before waiting for clients
	virtual void prepareEndScreen(int bf, float dw, int jm, Netcode::PlayerID id) = 0;

	void backToLobby();

	void runningMetalStart(Netcode::ComponentID id);
	void runningTileStart(Netcode::ComponentID id);
	void runningStopSound(Netcode::ComponentID id);

	void setGameStatePtr(GameState* ptr) { m_gameStatePtr = ptr; }

	bool onEvent(const Event& event) override;
};
