#include "LobbyClientState.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkNameEvent.h"
#include "../SPLASH/src/game/events/NetworkStartGameEvent.h"
#include "Network/NWrapper.h"
#include "Network/NWrapperClient.h"
#include "Network/NWrapperSingleton.h"

LobbyClientState::LobbyClientState(StateStack& stack)
	: LobbyState(stack),
	m_wasDropped(false)
{
}

LobbyClientState::~LobbyClientState() {
	if (m_wasDropped) {
		NWrapperSingleton::getInstance().resetNetwork();
		NWrapperSingleton::getInstance().resetWrapper();
	}


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
		std::string mesgWithId = "";
		mesgWithId += std::to_string(NWrapperSingleton::getInstance().getMyPlayerID()) + ':';
		mesgWithId += m_currentmessage;
		std::string msg = this->fetchMessage();
		m_network->sendChatMsg(mesgWithId);
	}
	
	return false;
}

bool LobbyClientState::onRecievedText(NetworkChatEvent& event) {
	// Only add the received message to the chat
	this->addTextToChat(&event.getMessage());

	return false;
}

bool LobbyClientState::onPlayerJoined(NetworkJoinedEvent& event) {
	// Add the player to the player list
	NWrapperSingleton::getInstance().playerJoined(event.getPlayer());
	return false;
}

bool LobbyClientState::onPlayerDisconnected(NetworkDisconnectEvent& event) {
	// Remove the player from the player list
	unsigned char id = event.getPlayerID();
	NWrapperSingleton::getInstance().playerLeft(id);

	return false;
}

bool LobbyClientState::onPlayerWelcomed(NetworkWelcomeEvent& event) {
	// Update local list of players.
	std::list<Player> &list = event.getListOfPlayers();
	if (list.size() >= 2) {
		// Clean local list of players.
		NWrapperSingleton::getInstance().resetPlayerList();

		printf("Received welcome package...\n");
		for (auto currentPlayer : list) {
			// TODO: Maybe addPlayerFunction?
			NWrapperSingleton::getInstance().playerJoined(currentPlayer);


			//m_players.push_back(currentPlayer);
			printf("\t");
			printf(currentPlayer.name.c_str());
			printf("\t");
			printf(std::to_string(currentPlayer.id).c_str());
			printf("\n");
		}
	}
	

	return false;
}

bool LobbyClientState::onNameRequest(NetworkNameEvent& event) {
	// Save the ID which the host has blessed us with
	std::string temp = event.getRepliedName();	// And replace our current HOSTID
	int newId = std::stoi(temp);					//
	NWrapperSingleton::getInstance().getMyPlayer().id = newId;
	NWrapperSingleton::getInstance().playerJoined(Player{ 0, NWrapperSingleton::getInstance().getMyPlayerName().c_str() });
	
	// Append :NAME onto ?ID --> ?ID:NAME and answer the host
	std::string message = "?";
	message += event.getRepliedName();
	message += ":";
	message += NWrapperSingleton::getInstance().getMyPlayerName().c_str();
	message += ":";
	m_network->sendMsg(message);
	return false;
}

bool LobbyClientState::onDropped(NetworkDroppedEvent& event) {
	// Queue changes to statestack
	this->requestStackPop();
	this->requestStackPush(States::MainMenu);

	// Reset network so that user can choose host/client again.
	m_wasDropped = true;


	return false;
}

bool LobbyClientState::onStartGame(NetworkStartGameEvent& event) {
	// Queue changes to the stack while maintaining the connection

	m_app->getStateStorage().setLobbyToGameData(LobbyToGameData(*m_settingBotCount));
	this->requestStackPop();
	this->requestStackPush(States::Game);

	return true;
}
