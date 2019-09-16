#include "LobbyJoinState.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"
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

	return true;
}

bool LobbyJoinState::onMyTextInput(TextInputEvent& event) {
	// Add input to current message, If 'enter', send message to host, do not input to chat.
	if (this->inputToChatLog(event.getMSG())) {
		m_network->sendChatMsg(this->fetchMessage());
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
	this->playerJoined(
		std::to_string(event.getPlayerID()),
		event.getPlayerID()
	);
	return false;
}

bool LobbyJoinState::onPlayerDisconnected(NetworkDisconnectEvent& event) {
	// Remove the player from the player list
	this->playerLeft(event.getPlayerID());

	return false;
}

bool LobbyJoinState::onPlayerWelcomed(NetworkWelcomeEvent& event) {
	// Clean local list of players.
	m_players.clear();

	// Update local list of players.
	std::list<player>* list = &event.getListOfPlayers();
	for (auto currentName : *list) {
		m_players.push_back(currentName)
	}

	return false;
}
