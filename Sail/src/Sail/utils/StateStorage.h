#pragma once

#include <string>

struct MenuToLobbyData {
	std::string name;
};

class StateStorage {
public:
	StateStorage() {
		// Initialize private data to 0
		menuLobbyData.name = "";
	}
	~StateStorage() { }

	void setMenuToLobbyData(MenuToLobbyData data) { menuLobbyData = data; }
	MenuToLobbyData* getMenuToLobbyData() { return &menuLobbyData; }

private:
	MenuToLobbyData menuLobbyData;

};