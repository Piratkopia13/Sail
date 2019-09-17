#include "LobbyHostState.h"
#include "../SPLASH/src/game/events/NetworkNameEvent.h"

LobbyHostState::LobbyHostState(StateStack& stack)
	: LobbyState(stack)
{
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
		Message temp{ to_string(m_me.id), m_currentmessage };
		this->addTextToChat(&temp);

		// ... Send the message to other clients and reset message
		m_network->sendChatAllClients(this->fetchMessage());
	}

	return true;
}

bool LobbyHostState::onRecievedText(NetworkChatEvent& event) {
	// Add recieved text to chat log
	this->addTextToChat(&event.getMessage());

	// Send out the recieved text to joined players
	// --- Wrapper already does this

	return false;
}

bool LobbyHostState::onPlayerJoined(NetworkJoinedEvent& event) {
	// Add player to player list
	//this->playerJoined(event.getPlayer());

	// Send out 'playerjoined'
	// --- Wrapper already does this

	return true;
}

bool LobbyHostState::onPlayerDisconnected(NetworkDisconnectEvent& event) {
	// Remove player from player list
	this->playerLeft(event.getPlayerID());

	// Send out 'playerdisconnected'
	// --- Wrapper already does this
	
	return false;
}

bool LobbyHostState::onNameRequest(NetworkNameEvent& event) {
	// Parse the message | ?12:DANIEL
	string message = event.getRepliedName(); 
	string id_string = "";
	unsigned int id_int = 0;

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
	id_int = stoi(id_string);

	message.erase(0, id_string.size() + 2);	// Removes ?ID: ___
	message.erase(message.size() - 1);		// Removes ___ :

	// Add player
	this->playerJoined(Player{
		id_int,
		message	// Which at this point is only the name
	});

	printf("Got name: \"%s\" from %i\n", message.c_str(), id_int);

	// Send a welcome package to all players, letting them know who's joined the party
	std::string welcomePackage = "w";

	printf("Sending out welcome package...\n");
	for (auto currentPlayer : m_players) {
		welcomePackage.append(to_string(currentPlayer.id));
		welcomePackage.append(":");
		welcomePackage.append(currentPlayer.name);
		welcomePackage.append(":");
		printf("\t");
		printf(currentPlayer.name.c_str());
		printf("\n");
	}
	if (welcomePackage.size() > 63) {
		int asdf = 3;
	}
	m_network->sendMsgAllClients(welcomePackage);
	printf("Done sending welcome package\n");
	return false;
}
