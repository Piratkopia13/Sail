#pragma once

#include "ReceiverBase.h"
#include "../../BaseComponentSystem.h"
#include "Sail/netcode/NetworkedStructs.h"
#include "Sail/events/EventReceiver.h"

class GameState;
class NetworkSenderSystem;
class GameDataTracker;

class NetworkReceiverSystem : public ReceiverBase {
public:
	NetworkReceiverSystem();
	virtual ~NetworkReceiverSystem();

	//void init(Netcode::PlayerID playerID, NetworkSenderSystem* netSendSysPtr);
	//void setPlayer(Entity* player);
	//void setGameState(GameState* gameState);

	//const std::vector<Entity*>& getEntities() const;

#ifdef DEVELOPMENT
	void imguiPrint(Entity** selectedEntity = nullptr) {

		ImGui::Text(std::string("ID: " + std::to_string((int)m_playerID)).c_str());
	}
#endif

protected:
	//NetworkSenderSystem* m_netSendSysPtr;
	//GameDataTracker* m_gameDataTracker;

	//// FIFO container of serialized data-strings to decode
	//std::queue<std::string> m_incomingDataBuffer;
	//std::mutex m_bufferLock;

	//// The player entity is used to prevent creation of receiver components for entities controlled by the player
	//Netcode::PlayerID m_playerID;
	//Entity* m_playerEntity;

	//GameState* m_gameStatePtr;
private:
	void createPlayer    (const PlayerComponentInfo& info, const glm::vec3& pos)             override;
	void destroyEntity   (const Netcode::CompID entityID)                                    override;
	void enableSprinklers()                                                                  override;
	void extinguishCandle(const Netcode::CompID candleID, const Netcode::PlayerID shooterID) override;
	void hitBySprinkler  (const Netcode::CompID candleOwnerID)                               override;
	void igniteCandle    (const Netcode::CompID candleID)                                    override;
	void playerDied      (const Netcode::CompID id, const Netcode::PlayerID shooterID)       override;
	void setAnimation    (const Netcode::CompID id, const AnimationInfo& info)               override;
	void setCandleHealth (const Netcode::CompID candleID, const float health)                override;
	void setCandleState  (const Netcode::CompID id, const bool isHeld)                       override;
	void setLocalPosition(const Netcode::CompID id, const glm::vec3& pos)                    override;
	void setLocalRotation(const Netcode::CompID id, const glm::vec3& rot)                    override;
	void setLocalRotation(const Netcode::CompID id, const glm::quat& rot)                    override;
	void spawnProjectile (const ProjectileInfo& info)                                        override;
	void waterHitPlayer  (const Netcode::CompID id, const Netcode::PlayerID SenderId)        override;

	// AUDIO
	void playerJumped     (const Netcode::CompID id) override;
	void playerLanded     (const Netcode::CompID id) override;
	void shootStart       (const Netcode::CompID id, const ShotFiredInfo& info) override;
	void shootLoop        (const Netcode::CompID id, const ShotFiredInfo& info) override;
	void shootEnd         (const Netcode::CompID id, const ShotFiredInfo& info) override;
	void runningMetalStart(const Netcode::CompID id) override;
	void runningTileStart (const Netcode::CompID id) override;
	void runningStopSound (const Netcode::CompID id) override;

	// HOST ONLY
	//void endMatch()                   override; // Start end timer for host
	//void endMatchAfterTimer(float dt) override; // Made for the host to quit the game after a set time
	//void mergeHostsStats()            override; // Host adds its data to global statistics before waiting for clients
	//void prepareEndScreen(int bf, float dw, int jm, Netcode::PlayerID id) override;

	//bool onEvent(const Event& event) override;


	// NOT FROM SERIALIZED MESSAGES
	void playerDisconnect(const Netcode::PlayerID playerID) override;
	
	// Helper function
	Entity* findFromNetID(Netcode::CompID id) const override;

	bool onEvent(const Event& event) override;

};
