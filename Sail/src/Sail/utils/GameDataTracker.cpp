#include "pch.h"
#include "GameDataTracker.h"
#include "Sail.h"
#include "../libraries/imgui/imgui.h"
#include <string>
#include "Network/NWrapperSingleton.h"

GameDataTracker::GameDataTracker() {
	m_loggedData = { 0 };
	m_loggedDataGlobal = { 0 };
	m_network = &NWrapperSingleton::getInstance();
	m_placement = 13;
	m_nPlayersCurrentSession = 0;
	m_trackLocalStats = true;

	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_DISCONNECT, this);
}

GameDataTracker::~GameDataTracker() {
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_DISCONNECT, this);
}

GameDataTracker& GameDataTracker::getInstance() {
	static GameDataTracker instance;

	return instance;
}

void GameDataTracker::init() {
	resetData();
}

void GameDataTracker::resetData() {
	m_trackLocalStats = true;
	m_loggedData = { 0 };
	m_loggedDataGlobal = { 0 };
	m_killFeed.clear();

	m_loggedDataGlobal.bulletsFiredID = NWrapperSingleton::getInstance().getMyPlayerID();
	m_loggedDataGlobal.distanceWalkedID = NWrapperSingleton::getInstance().getMyPlayerID();
	m_loggedDataGlobal.jumpsMadeID = NWrapperSingleton::getInstance().getMyPlayerID();
	m_hostPlayerTracker.clear();

	m_nPlayersCurrentSession = 0;
	for (auto player : m_network->getPlayers()) {
		m_hostPlayerTracker[player.id].nKills = 0;
		m_hostPlayerTracker[player.id].nDeaths = 0;
		m_hostPlayerTracker[player.id].placement = 99;
		m_hostPlayerTracker[player.id].playerName = player.name;
		m_nPlayersCurrentSession++;
	}
	m_placement = m_nPlayersCurrentSession + 1;
}

void GameDataTracker::logWeaponFired() {
	if (m_trackLocalStats) {
		m_loggedData.bulletsFired += 1;
	}
}

void GameDataTracker::logEnemyKilled(Netcode::PlayerID playerID) {
	m_hostPlayerTracker[playerID].nKills += 1;
}

void GameDataTracker::logJump() {
	if (m_trackLocalStats) {
		m_loggedData.jumpsMade += 1;
	}
}

void GameDataTracker::logDistanceWalked(glm::vec3 vector) {
	if (m_trackLocalStats) {
		float distanceOfVector = sqrt(pow(vector.x, 2) + pow(vector.y, 2) + pow(vector.z, 2));
		m_loggedData.distanceWalked += distanceOfVector;
	}
	
}

void GameDataTracker::logPlayerDeath(const std::string& killer, const std::string& killed, const std::string& deathType) {
	m_killFeed.emplace_back(killer + " " + deathType + " " + killed);
}

void GameDataTracker::logPlacement(Netcode::PlayerID playerID) {
	m_placement--;
	m_hostPlayerTracker[playerID].placement = m_placement;
}

void GameDataTracker::logMessage(const std::string& message) {
	m_killFeed.push_back(message);
}

GlobalTopStats& GameDataTracker::getStatisticsGlobal() {
	return m_loggedDataGlobal;
}

InduvidualStats& GameDataTracker::getStatisticsLocal() {
	return m_loggedData;
}

const std::vector<std::string>& GameDataTracker::getKillFeed() {
	return m_killFeed;
}

const std::map<Netcode::PlayerID, HostStatsPerPlayer> GameDataTracker::getPlayerDataMap() {
	return m_hostPlayerTracker;
}

void GameDataTracker::setStatsForPlayer(Netcode::PlayerID id, int nKills, int placement) {
	m_hostPlayerTracker[id].nKills = nKills;
	m_hostPlayerTracker[id].placement = placement;
}

void GameDataTracker::setStatsForOtherData(Netcode::PlayerID bfID, int bf, Netcode::PlayerID dwID, float dw, Netcode::PlayerID jmID, int jm) {
	m_loggedDataGlobal.bulletsFiredID = bfID;
	m_loggedDataGlobal.bulletsFired = bf;
	m_loggedDataGlobal.distanceWalkedID = dwID;
	m_loggedDataGlobal.distanceWalked = dw;
	m_loggedDataGlobal.jumpsMadeID = jmID;
	m_loggedDataGlobal.jumpsMade = jm;
}

int GameDataTracker::getPlayerCount() {
	return m_nPlayersCurrentSession;
}

void GameDataTracker::turnOffLocalDataTracking() {
	m_trackLocalStats = false;
}

void GameDataTracker::renderImgui() {

	ImGui::Begin("Game Statistics", NULL);
	ImGui::SetWindowPos({ 18,12 });
	ImGui::SetWindowSize({ 307,600 });
	struct mapLayout {
		std::string name;
		int nKills;
	};
	std::map<int, mapLayout> tempPlacementMap;


	ImGui::Text("\n Placement------");
	// Sort rankings
	for (auto player : m_hostPlayerTracker) {
		tempPlacementMap[player.second.placement].name = player.second.playerName;
		tempPlacementMap[player.second.placement].nKills = player.second.nKills;
	}

	if(ImGui::BeginChild("RESULTS",ImVec2(0, 300))) {
		ImGui::Columns(3, "PlacementColumns", true);
		ImGui::Separator();
		ImGui::Text("Placement"); ImGui::NextColumn();
		ImGui::Text("Player"); ImGui::NextColumn();
		ImGui::Text("Kills"); ImGui::NextColumn();
		ImGui::Separator();


		for (int i = 1; i < (int)tempPlacementMap.size() + 1; i++) {

			ImGui::Text(std::to_string(i).c_str()); ImGui::NextColumn();
			ImGui::Text(tempPlacementMap[i].name.c_str()); ImGui::NextColumn();
			ImGui::Text(std::to_string(tempPlacementMap[i].nKills).c_str()); ImGui::NextColumn();
		}

	}
	ImGui::EndChild();

	

	ImGui::Text("\n Misc stats------");
	
	std::string bdString = "Most bullets fires by " + 
		m_hostPlayerTracker[m_loggedDataGlobal.bulletsFiredID].playerName + ": " + std::to_string(m_loggedDataGlobal.bulletsFired);
	ImGui::Text(bdString.c_str());

	bdString = "Longest distance walked by " +
		m_hostPlayerTracker[m_loggedDataGlobal.distanceWalkedID].playerName + ": " + std::to_string((int)m_loggedDataGlobal.distanceWalked) + "m";
	ImGui::Text(bdString.c_str());

	bdString = "Most jumps made by " +
		m_hostPlayerTracker[m_loggedDataGlobal.jumpsMadeID].playerName + ": " + std::to_string(m_loggedDataGlobal.jumpsMade);
	ImGui::Text(bdString.c_str());
	
	ImGui::End();


	ImGui::Begin("WINNER", NULL);
	ImGui::SetWindowFontScale(5.0f);
	ImGui::SetWindowPos({331,12});
	ImGui::SetWindowSize({ 404,292 });
	ImGui::Text(tempPlacementMap[1].name.c_str());
	ImGui::End();

	ImGui::Begin("Personal Statistics", NULL);
	ImGui::SetWindowPos({ 331,310 });
	ImGui::SetWindowSize({ 307,293 });

	std::string localStatsString = "Bullets fired: " + std::to_string(m_loggedData.bulletsFired);
	ImGui::Text(localStatsString.c_str());

	localStatsString = "Distance walked: " + std::to_string((int)m_loggedData.distanceWalked) + "m";
	ImGui::Text(localStatsString.c_str());

	localStatsString = "Jumps made: " + std::to_string(m_loggedData.jumpsMade);
	ImGui::Text(localStatsString.c_str());

	ImGui::End();
}

bool GameDataTracker::onEvent(const Event& e) {

	switch (e.type) {
	case Event::Type::NETWORK_DISCONNECT:		playerDisconnected((const NetworkDisconnectEvent&)e); break;
	default:
		break;
	}

	return true;
}

void GameDataTracker::playerDisconnected(const NetworkDisconnectEvent& e) {
	if (m_trackLocalStats) {
		logPlacement(e.player.id);
	}
}
