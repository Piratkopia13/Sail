#pragma once

#include <string>
using namespace std;

struct MenuToLobbyData {
	string name;
};

//struct LobbyToGameData {
// Add when we need it
//};


class StateStorage {
public:
	StateStorage() {
		// Initialize private data to 0
		menuLobbyData.name = "";
	}
	~StateStorage() { }

	void setMenuToLobbyData(MenuToLobbyData data) { menuLobbyData = data; }
	MenuToLobbyData getMenuToLobbyData() { return menuLobbyData; }

private:
	MenuToLobbyData menuLobbyData;

};