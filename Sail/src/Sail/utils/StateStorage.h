#pragma once

#include <string>
#include <list>

struct MenuToLobbyData {
	std::string name;
};

struct LobbyToGameData {
	int botCount = 0;
	bool enterAsSpectator = false;

	LobbyToGameData() {}
	LobbyToGameData(int nrOfBots, bool spectator) : botCount(nrOfBots), enterAsSpectator(spectator) {}
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