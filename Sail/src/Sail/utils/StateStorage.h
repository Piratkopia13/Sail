#pragma once

#include <string>

struct MenuToLobbyData {
	std::string name;
};

struct LobbyToGameStateData {
	int botCount = 0;
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

	void setLobbyToGameStateData(LobbyToGameStateData& data) { lobbyGameData = data; }
	const LobbyToGameStateData* getLobbyToGameStateData() { return &lobbyGameData; }

private:
	MenuToLobbyData menuLobbyData; // //
	LobbyToGameStateData lobbyGameData;

};