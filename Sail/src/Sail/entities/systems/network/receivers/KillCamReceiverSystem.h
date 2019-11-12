#pragma once

#include "ReceiverBase.h"
#include "../../BaseComponentSystem.h"
#include "Sail/netcode/NetworkedStructs.h"
#include "Sail/events/EventReceiver.h"


class GameState;
class NetworkSenderSystem;
class GameDataTracker;

class KillCamReceiverSystem : public ReceiverBase {
public:
	KillCamReceiverSystem();
	virtual ~KillCamReceiverSystem();

	void handleIncomingData(const std::string& data) override;

#ifdef DEVELOPMENT
	void imguiPrint(Entity** selectedEntity = nullptr) {

		ImGui::Text(std::string("ID: " + std::to_string((int)m_playerID)).c_str());
	}
#endif


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
	void endMatch()                         override; // Start end timer for host
	void endMatchAfterTimer(const float dt) override; // Made for the host to quit the game after a set time
	void mergeHostsStats()                  override; // Host adds its data to global statistics before waiting for clients
	void prepareEndScreen(const Netcode::PlayerID sender, const EndScreenInfo& info) override;

	// NOT FROM SERIALIZED MESSAGES
	void playerDisconnect(const Netcode::PlayerID playerID) override;
	
	// Helper function
	Entity* findFromNetID(const Netcode::CompID id) const override;


	bool onEvent(const Event& event) override;
};
