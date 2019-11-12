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

	void init(Netcode::PlayerID playerID, NetworkSenderSystem* netSendSysPtr);
	void setPlayer(Entity* player);
	void setGameState(GameState* gameState);

	const std::vector<Entity*>& getEntities() const;

	void update(float dt);

#ifdef DEVELOPMENT
	void imguiPrint(Entity** selectedEntity = nullptr) {

		ImGui::Text(std::string("ID: " + std::to_string((int)m_playerID)).c_str());
	}
#endif

protected:
	NetworkSenderSystem* m_netSendSysPtr;
	GameDataTracker* m_gameDataTracker;

	// FIFO container of serialized data-strings to decode
	std::queue<std::string> m_incomingDataBuffer;
	std::mutex m_bufferLock;

	// The player entity is used to prevent creation of receiver components for entities controlled by the player
	Netcode::PlayerID m_playerID;
	Entity* m_playerEntity;

	GameState* m_gameStatePtr;
private:
	void createPlayerEntity(Netcode::CompID playerCompID, Netcode::CompID candleCompID, Netcode::CompID gunCompID, const glm::vec3& translation);
	void destroyEntity(Netcode::CompID entityID);
	void setEntityLocalPosition(Netcode::CompID id, const glm::vec3& translation);
	void setEntityLocalRotation(Netcode::CompID id, const glm::vec3& rotation);
	void setEntityLocalRotation(Netcode::CompID id, const glm::quat& rotation);
	void setEntityAnimation(Netcode::CompID id, unsigned int animationIndex, float animationTime);
	void setCandleHealth(Netcode::CompID candleId, float health);
	void extinguishCandle(Netcode::CompID candleId, Netcode::PlayerID shooterID);
	void playerJumped(Netcode::CompID id);
	void playerLanded(Netcode::CompID id);
	void projectileSpawned(glm::vec3& pos, glm::vec3 vel, Netcode::CompID projectileID, Netcode::CompID ownerID);
	void playerDied(Netcode::CompID id, Netcode::PlayerID shooterID);
	void playerDisconnect(Netcode::PlayerID playerID);
	void setCandleHeldState(Netcode::CompID id, bool isHeld);
	void hitBySprinkler(Netcode::CompID candleOwnerID);
	void enableSprinklers();
	void igniteCandle(Netcode::CompID candleID);

	Entity* findFromNetID(Netcode::CompID id) const;

	void shootStart(glm::vec3& gunPos, glm::vec3& gunVel, Netcode::CompID id);
	void shootLoop(glm::vec3& gunPos, glm::vec3& gunVel, Netcode::CompID id);
	void shootEnd(glm::vec3& gunPos, glm::vec3& gunVel, Netcode::CompID id);

	virtual void waterHitPlayer(Netcode::CompID id, Netcode::PlayerID SenderId) = 0;
	virtual void endMatch() = 0;                   // Start end timer for host
	virtual void endMatchAfterTimer(float dt) = 0; // Made for the host to quit the game after a set time
	virtual void mergeHostsStats() = 0;            // Host adds its data to global statistics before waiting for clients
	virtual void prepareEndScreen(int bf, float dw, int jm, Netcode::PlayerID id) = 0;

	void runningMetalStart(Netcode::CompID id);
	void runningTileStart(Netcode::CompID id);
	void runningStopSound(Netcode::CompID id);
	bool onEvent(const Event& event) override;

};
