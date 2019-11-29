#pragma once

#include <string>
#include <map>
#include <vector>
#include "Sail/netcode/NetcodeTypes.h"
#include "Sail/events/EventReceiver.h"
#include "../SPLASH/src/game/events/NetworkDisconnectEvent.h"

class NWrapperSingleton;
class Application;
struct InduvidualStats {
	int bulletsFired;
	float distanceWalked;
	int jumpsMade;
};

struct GlobalTopStats {
	int bulletsFired;
	float distanceWalked;
	int jumpsMade;

	// IDs for the host to fill for the endscreen
	Netcode::PlayerID bulletsFiredID;
	Netcode::PlayerID distanceWalkedID;
	Netcode::PlayerID jumpsMadeID;
};

struct HostStatsPerPlayer {
	int nKills = 0;
	int nDeaths = 0;
	int placement = 0;
	int damage = 0;
	int damageTaken = 0;
	std::string playerName;	//Stored here to handle disconnects.
};

class GameDataTracker : public EventReceiver {
public:
	//
	~GameDataTracker();

	// Implemented in...
	void logWeaponFired();						// ...ehfy::update()
	void logEnemyKilled(Netcode::PlayerID killer);// CandleSystem::update
	void logDeath(Netcode::PlayerID victim);
	void logDamageDone(Netcode::PlayerID playerID, const int dmg);
	void logDamageTaken(Netcode::PlayerID playerID, const int dmg);
	void logJump();								// ...GameInputSystem::update()
	void logDistanceWalked(glm::vec3 vector);	// ...PhysicsSystem::update()
	void logPlacement(Netcode::PlayerID playerID);// CandleSystem::update
	void logMessage(const std::string& message);// CandleSystem::update

	void init();								// Gamestate::Gamestate()

	void resetData();							// EndGameState::onReturnToLobby() / renderImGui()
	// Filled with data at endgame to present to endscreen
	GlobalTopStats& getStatisticsGlobal();		// NetworkSenderSystem::writeEventToArchive()
	// Used to track the local player
	InduvidualStats& getStatisticsLocal();

	const std::vector<std::string>& getKillFeed();
	const std::map<Netcode::PlayerID, HostStatsPerPlayer> getPlayerDataMap(); // NetworkSenderSystem

	// Used in end game when recieving player data stats
	void setStatsForPlayer(Netcode::PlayerID id, int nKills, int placement, int nDeaths, int damage, int damageTaken); // NetworkRecieverSystem::Update()
	void setStatsForOtherData(Netcode::PlayerID bfID, int bf,
		Netcode::PlayerID dwID, float dw, Netcode::PlayerID jmID, int jm);
	// nr of player from the start of the match
	int getPlayerCount();	// Nowhere atm
	void turnOffLocalDataTracking();

	// Implemented in...
	void renderImgui();							// ...EndState::renderImGui()
	
	const int getTorchesLeft();
	void reduceTorchesLeft();

	const int getPlayersLeft();
	void setPlayersLeft(int playersLeft);

	void renderPlacement();
	void renderPersonalStats();
	void renderFunStats();
	void renderWinners();

#ifdef DEVELOPMENT
	void addDebugData();
#endif

private:
	Application* m_app;
	NWrapperSingleton* m_network;

	InduvidualStats m_loggedData;
	GlobalTopStats m_loggedDataGlobal;

	bool m_trackLocalStats;

	// Map of each player in current game containing data such as kills and deaths
	std::map<Netcode::PlayerID, HostStatsPerPlayer> m_hostPlayerTracker;

	int m_placement; // add 1 after ever time a player is placed to give next player a placement
	int m_nPlayersCurrentSession;
	std::vector<std::string> m_killFeed;

	int m_torchesLeft = 3;
	int m_playersLeft = 0;
	// -+-+-+-+-+- Singleton requirements below -+-+-+-+-+-
public:
	GameDataTracker(GameDataTracker const&) = delete;
	void operator=(GameDataTracker const&) = delete;
	static GameDataTracker& getInstance();
private:
	GameDataTracker();
	virtual bool onEvent(const Event& e);
	void playerDisconnected(const NetworkDisconnectEvent& e);


};