#pragma once

#include <string>
#include <list>



// Really should be somewhere else
struct Player {
	unsigned char id;
	std::string name;

	bool friend operator==(const Player& left, const Player& right) {
		if (left.id == right.id &&
			left.name == right.name) {
			return true;
		}
		return false;
	}
};

struct MenuToLobbyData {
	std::string name;
};


struct LobbyToGameData {
	Player myPlayer;
	std::list<Player> playerList;
	int botCount = 0;

	LobbyToGameData() {}
	LobbyToGameData(Player& me, std::list<Player>& players, int nrOfBots = 0) : 
		myPlayer(me), playerList(players), botCount(nrOfBots) {}
};

class StateStorage {
public:
	StateStorage() {
		// Initialize private data to 0
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