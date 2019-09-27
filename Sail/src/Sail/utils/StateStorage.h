#pragma once

#include <string>
#include <list>

class NWrapperHost;
class NWrapperClient;


//struct Player {
//	unsigned char id;
//	std::string name;
//
//	bool friend operator==(const Player& left, const Player& right) {
//		if (left.id == right.id &&
//			left.name == right.name) {
//			return true;
//		}
//		return false;
//	}
//};

//struct fuckyou {
//	std::list<Player> hehe;
//
//	fuckyou friend operator = (const Player& left, const Player& right) {
//		if (left.id == right.id &&
//			left.name == right.name) {
//			return true;
//		}
//		return false;
//	}
//};

struct MenuToLobbyData {
	std::string name;
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

	/*void setLobbyToGameData(fuckyou playerList) { m_playerList = playerList.hehe; }
	std::list<Player> getLobbyToGameData() { return m_playerList; }*/

private:
	MenuToLobbyData m_menuLobbyData; // //
	//std::list<Player> m_playerList;

};