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

	// something
	void setMenuToLobbyData(MenuToLobbyData& data) { menuLobbyData = data; }
	const MenuToLobbyData* getMenuToLobbyData() { return &menuLobbyData; }

private:
	MenuToLobbyData menuLobbyData;

};