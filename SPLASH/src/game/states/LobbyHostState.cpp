#include "LobbyHostState.h"
#include "../SPLASH/src/game/events/NetworkNameEvent.h"
#include "Network/NWrapper.h"
#include "Network/NWrapperHost.h"

LobbyHostState::LobbyHostState(StateStack& stack)
	: LobbyState(stack) {
	// Reserved for host, all other will get 1,2,3,...,n
	NWrapperSingleton::getInstance().setPlayerID(HOST_ID); 

	NWrapperSingleton::getInstance().playerJoined(NWrapperSingleton::getInstance().getMyPlayer());
}

LobbyHostState::~LobbyHostState() {
}

bool LobbyHostState::onEvent(Event& event)
{
	EventHandler::dispatch<TextInputEvent>(event, SAIL_BIND_EVENT(&LobbyHostState::onMyTextInput));
	EventHandler::dispatch<NetworkChatEvent>(event, SAIL_BIND_EVENT(&LobbyHostState::onRecievedText));
	EventHandler::dispatch<NetworkJoinedEvent>(event, SAIL_BIND_EVENT(&LobbyHostState::onPlayerJoined));
	EventHandler::dispatch<NetworkDisconnectEvent>(event, SAIL_BIND_EVENT(&LobbyHostState::onPlayerDisconnected));
	EventHandler::dispatch<NetworkNameEvent>(event, SAIL_BIND_EVENT(&LobbyHostState::onNameRequest));

	return false;
}

bool LobbyHostState::onMyTextInput(TextInputEvent& event) {
	// Add to current message, If 'enter' ...
	if (this->inputToChatLog(event.getMSG())) {
		// ... Add current message to chat log
		Message temp{ std::to_string(NWrapperSingleton::getInstance().getMyPlayer().id), m_currentmessage };
		this->addTextToChat(&temp);

		// ... Append my ID to it.
		std::string mesgWithId = "";
		mesgWithId += std::to_string(NWrapperSingleton::getInstance().getMyPlayer().id) + ':';
		mesgWithId += m_currentmessage;
		std::string msg = this->fetchMessage();

		// ... Send the message to other clients and reset message
		m_network->sendChatAllClients(mesgWithId);
	}

	return true;
}

bool LobbyHostState::onRecievedText(NetworkChatEvent& event) {
	// Add received text to chat log
	this->addTextToChat(&event.getMessage());

	return false;
}

bool LobbyHostState::onPlayerJoined(NetworkJoinedEvent& event) {
	// Add player to player list
	//this->playerJoined(event.getPlayer());

	return true;
}

bool LobbyHostState::onPlayerDisconnected(NetworkDisconnectEvent& event) {
	// Remove player from player list
	unsigned char id = event.getPlayerID();
	NWrapperSingleton::getInstance().playerLeft(id);
	
	return false;
}

bool LobbyHostState::onNameRequest(NetworkNameEvent& event) {
	// Parse the message | ?12:DANIEL
	std::string message = event.getRepliedName(); 
	std::string id_string = "";
	unsigned char id_int = 0;

	// Get ID...
	for (int i = 1; i < 64; i++) {
		// ... as a string
		if (message[i] != ':') {
			id_string += message[i];
		}
		else {
			break;
		}
	}
	// ... as a number
	id_int = std::stoi(id_string);

	message.erase(0, id_string.size() + 2);	// Removes ?ID: ___
	message.erase(message.size() - 1);		// Removes ___ :

	// Add player
	NWrapperSingleton::getInstance().playerJoined(Player{
			id_int,
			message	// Which at this point is only the name
	});


	printf("Got name: \"%s\" from %i\n", message.c_str(), id_int);

	// Send a welcome package to all players, letting them know who's joined the party
	std::string welcomePackage = "w";

	printf("Sending out welcome package...\n");
	for (auto currentPlayer : NWrapperSingleton::getInstance().getPlayers()) {
		welcomePackage.append(std::to_string(currentPlayer.id));
		welcomePackage.append(":");
		welcomePackage.append(currentPlayer.name);
		welcomePackage.append(":");
		printf("\t");
		printf(currentPlayer.name.c_str());
		printf("\n");
	}
	m_network->sendMsgAllClients(welcomePackage);
	printf("Done sending welcome package\n");
	return false;
}
