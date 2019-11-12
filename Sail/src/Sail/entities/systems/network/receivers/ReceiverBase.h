#pragma once

#include "../../BaseComponentSystem.h"
#include "Sail/netcode/NetworkedStructs.h"
#include "Sail/events/EventReceiver.h"

class GameState;
class NetworkSenderSystem;
class GameDataTracker;

class ReceiverBase : public BaseComponentSystem, public EventReceiver {
public: // Functions
	ReceiverBase();
	virtual ~ReceiverBase();

	void init(Netcode::PlayerID playerID, NetworkSenderSystem* netSendSysPtr); // needed?
	void setPlayer(Entity* player);
	void setGameState(GameState* gameState);
	const std::vector<Entity*>& getEntities() const;

	void update(float dt);


	virtual void handleIncomingData(std::string data) = 0;

protected: // Variables
	NetworkSenderSystem* m_netSendSysPtr;
	GameDataTracker* m_gameDataTracker; // needed?

	// FIFO container of serialized data-strings to decode
	std::queue<std::string> m_incomingDataBuffer;
	std::mutex m_bufferLock;

	// The player entity is used to prevent creation of receiver components for entities controlled by the player
	Netcode::PlayerID m_playerID; // TODO? move to NRS
	Entity* m_playerEntity;

	GameState* m_gameStatePtr;

protected: // Input parameters
	struct PlayerComponentInfo {
		Netcode::CompID playerCompID;
		Netcode::CompID candleID;
		Netcode::CompID gunID;
	};

	struct ProjectileInfo {
		Netcode::CompID projectileID;
		Netcode::CompID ownerID;
		glm::vec3 position;
		glm::vec3 velocity;
	};

	struct AnimationInfo {
		unsigned int index;
		float time;
	};

	struct ShotFiredInfo {
		glm::vec3 gunPosition;
		glm::vec3 gunVelocity;
	};

protected: // Functions
	void pushDataToBuffer(const std::string& data);

	virtual void createPlayer    (const PlayerComponentInfo& info, const glm::vec3& pos)             = 0;
	virtual void destroyEntity   (const Netcode::CompID entityID)                                    = 0;
	virtual void enableSprinklers()                                                                  = 0;
	virtual void extinguishCandle(const Netcode::CompID candleID, const Netcode::PlayerID shooterID) = 0;
	virtual void hitBySprinkler  (const Netcode::CompID candleOwnerID)                               = 0;
	virtual void igniteCandle    (const Netcode::CompID candleID)                                    = 0;
	virtual void playerDied      (const Netcode::CompID id, const Netcode::PlayerID shooterID)       = 0;
	virtual void playerJumped    (const Netcode::CompID id)                                          = 0;
	virtual void playerLanded    (const Netcode::CompID id)                                          = 0;
	virtual void setAnimation    (const Netcode::CompID id, const AnimationInfo& info)               = 0;
	virtual void setCandleHealth (const Netcode::CompID candleID, const float health)                = 0;
	virtual void setCandleState  (const Netcode::CompID id, const bool isHeld)                       = 0;
	virtual void setLocalPosition(const Netcode::CompID id, const glm::vec3& pos)                    = 0;
	virtual void setLocalRotation(const Netcode::CompID id, const glm::vec3& rot)                    = 0;
	virtual void setLocalRotation(const Netcode::CompID id, const glm::quat& rot)                    = 0;
	virtual void spawnProjectile (const ProjectileInfo& info)                                        = 0;
	virtual void waterHitPlayer  (const Netcode::CompID id, const Netcode::PlayerID SenderId)        = 0;

	// AUDIO
	virtual void shootStart       (const Netcode::CompID id, const ShotFiredInfo& info) = 0;
	virtual void shootLoop        (const Netcode::CompID id, const ShotFiredInfo& info) = 0;
	virtual void shootEnd         (const Netcode::CompID id, const ShotFiredInfo& info) = 0;
	virtual void runningMetalStart(const Netcode::CompID id)  = 0;
	virtual void runningTileStart (const Netcode::CompID id)  = 0;
	virtual void runningStopSound (const Netcode::CompID id)  = 0;

	// HOST ONLY
	virtual void endMatch()                   = 0; // Start end timer for host
	virtual void endMatchAfterTimer(float dt) = 0; // Made for the host to quit the game after a set time
	virtual void mergeHostsStats()            = 0; // Host adds its data to global statistics before waiting for clients
	virtual void prepareEndScreen(int bf, float dw, int jm, Netcode::PlayerID id) = 0;

	//bool onEvent(const Event& event) override;


	// NOT FROM SERIALIZED MESSAGES
	virtual void playerDisconnect(const Netcode::PlayerID playerID) = 0;
	
	// Helper function
	virtual Entity* findFromNetID(Netcode::CompID id) const = 0;
};
