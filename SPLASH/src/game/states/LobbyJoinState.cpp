#include "LobbyJoinState.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkNameEvent.h"
#include "Network/NetworkWrapper.h"

LobbyJoinState::LobbyJoinState(StateStack& stack)
	: LobbyState(stack)
{
}

LobbyJoinState::~LobbyJoinState() {
}

bool LobbyJoinState::onEvent(Event& event) {
	EventHandler::dispatch<TextInputEvent>(event, SAIL_BIND_EVENT(&LobbyJoinState::onMyTextInput));
	EventHandler::dispatch<NetworkChatEvent>(event, SAIL_BIND_EVENT(&LobbyJoinState::onRecievedText));
	EventHandler::dispatch<NetworkJoinedEvent>(event, SAIL_BIND_EVENT(&LobbyJoinState::onPlayerJoined));
	EventHandler::dispatch<NetworkDisconnectEvent>(event, SAIL_BIND_EVENT(&LobbyJoinState::onPlayerDisconnected));
	EventHandler::dispatch<NetworkWelcomeEvent>(event, SAIL_BIND_EVENT(&LobbyJoinState::onPlayerWelcomed));
	EventHandler::dispatch<NetworkNameEvent>(event, SAIL_BIND_EVENT(&LobbyJoinState::onNameRequest));
	// something
	return true;
}

bool LobbyJoinState::onMyTextInput(TextInputEvent& event) {
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

bool LobbyJoinState::onRecievedText(NetworkChatEvent& event) {
	// Only add the recieved message to the chat
	this->addTextToChat(&event.getMessage());

	return false;
}

bool LobbyJoinState::onPlayerJoined(NetworkJoinedEvent& event) {
	// Add the player to the player list
	this->playerJoined(event.getPlayer());
	return false;
}

bool LobbyJoinState::onPlayerDisconnected(NetworkDisconnectEvent& event) {
	// Remove the player from the player list
	this->playerLeft(event.getPlayerID());

	return false;
}

bool LobbyJoinState::onPlayerWelcomed(NetworkWelcomeEvent& event) {
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

bool LobbyJoinState::onNameRequest(NetworkNameEvent& event) {
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
