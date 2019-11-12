#pragma once

#include <string>
#include <list>

struct MenuToLobbyData {
	std::string name;
};

struct LobbyToGameData {
	int botCount = 0;
	int team = 1; // team 0 = spectator

	LobbyToGameData() {}
	LobbyToGameData(int nrOfBots, int _team) : botCount(nrOfBots), team(_team) {}
};

class StateStorage {
public:
	StateStorage() {
		// Initialize private data to 0 //
		m_menuLobbyData.name = "";
	}
	~StateStorage() { }

	void setMenuToLobbyData(MenuToLobbyData& data) { m_menuLobbyData = data; }
	const MenuToLobbyData* getMenuToLobbyData() { return &m_menuLobbyData; }

	void setLobbyToGameData(LobbyToGameData& data) { m_lobyToGameData = data; }
	const LobbyToGameData* getLobbyToGameData() { return &m_lobyToGameData; }


private:
	MenuToLobbyData m_menuLobbyData;
	LobbyToGameData m_lobyToGameData;

};