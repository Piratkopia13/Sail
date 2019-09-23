#include "LobbyClientState.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkNameEvent.h"
#include "../SPLASH/src/game/events/NetworkStartGameEvent.h"
#include "Network/NWrapper.h"
#include "Network/NWrapperClient.h"
#include "Network/NWrapperSingleton.h"

using namespace std;

LobbyClientState::LobbyClientState(StateStack& stack)
	: LobbyState(stack)
{
}

LobbyClientState::~LobbyClientState() {
}

bool LobbyClientState::onEvent(Event& event) {
	EventHandler::dispatch<TextInputEvent>(event, SAIL_BIND_EVENT(&LobbyClientState::onMyTextInput));
	EventHandler::dispatch<NetworkChatEvent>(event, SAIL_BIND_EVENT(&LobbyClientState::onRecievedText));
	EventHandler::dispatch<NetworkJoinedEvent>(event, SAIL_BIND_EVENT(&LobbyClientState::onPlayerJoined));
	EventHandler::dispatch<NetworkDisconnectEvent>(event, SAIL_BIND_EVENT(&LobbyClientState::onPlayerDisconnected));
	EventHandler::dispatch<NetworkWelcomeEvent>(event, SAIL_BIND_EVENT(&LobbyClientState::onPlayerWelcomed));
	EventHandler::dispatch<NetworkNameEvent>(event, SAIL_BIND_EVENT(&LobbyClientState::onNameRequest));
	EventHandler::dispatch<NetworkDroppedEvent>(event, SAIL_BIND_EVENT(&LobbyClientState::onDropped));
	EventHandler::dispatch<NetworkStartGameEvent>(event, SAIL_BIND_EVENT(&LobbyClientState::onStartGame));
	return true;
}

bool LobbyClientState::onMyTextInput(TextInputEvent& event) {
	// Add input to current message, If 'enter', send message to host, do not input to chat.
	if (this->inputToChatLog(event.getMSG())) {
		string mesgWithId = "";
		mesgWithId += to_string(m_me.id) + ':';
		mesgWithId += m_currentmessage;
		this->fetchMessage();
		m_network->sendChatMsg(mesgWithId);
	}
	
	return false;
}

bool LobbyClientState::onRecievedText(NetworkChatEvent& event) {
	// Only add the recieved message to the chat
	this->addTextToChat(&event.getMessage());

	return false;
}

bool LobbyClientState::onPlayerJoined(NetworkJoinedEvent& event) {
	// Add the player to the player list
	this->playerJoined(event.getPlayer());
	return false;
}

bool LobbyClientState::onPlayerDisconnected(NetworkDisconnectEvent& event) {
	// Remove the player from the player list
	unsigned char id = event.getPlayerID();
	this->playerLeft(id);

	return false;
}

bool LobbyClientState::onPlayerWelcomed(NetworkWelcomeEvent& event) {
	// Update local list of players.
	std::list<Player> &list = event.getListOfPlayers();
	if (list.size() >= 2) {
		// Clean local list of players.
		this->resetPlayerList();

		printf("Recieved welcome package...\n");
		for (auto currentPlayer : list) {
			// TODO: Maybe addPlayerFunction?
			this->playerJoined(currentPlayer);

			//m_players.push_back(currentPlayer);
			printf("\t");
			printf(currentPlayer.name.c_str());
			printf("\t");
			printf(to_string(currentPlayer.id).c_str());
			printf("\n");
		}
	}
	

	return false;
}

bool LobbyClientState::onNameRequest(NetworkNameEvent& event) {
	// Save the ID which the host has blessed us with
	string temp = event.getRepliedName();	// And replace our current HOSTID
	int newId = stoi(temp);					//
	m_me.id = newId;
	this->playerJoined(Player{ 0, m_me.name });
	
	// Append :NAME onto ?ID --> ?ID:NAME and answer the host
	string message = "?";
	message += event.getRepliedName();
	message += ":" + m_me.name + ":";
	m_network->sendMsg(message);
	return false;
}

bool LobbyClientState::onDropped(NetworkDroppedEvent& event) {
	this->requestStackPop();
	this->requestStackPush(States::MainMenu);
	NWrapperSingleton::getInstance().resetNetwork();

	return false;
}

bool LobbyClientState::onStartGame(NetworkStartGameEvent& event) {
	
	this->requestStackPop();
	this->requestStackPush(States::Game);

	return true;
}
