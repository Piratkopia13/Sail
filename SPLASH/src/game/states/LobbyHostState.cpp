#include "LobbyHostState.h"
#include "../SPLASH/src/game/events/NetworkNameEvent.h"
#include "Network/NWrapper.h"
#include "Network/NWrapperHost.h"

LobbyHostState::LobbyHostState(StateStack& stack)
	: LobbyState(stack) {
	// Reserved for host, all other will get 1,2,3,...,n
	NWrapperSingleton::getInstance().setPlayerID(HOST_ID); 
	NWrapperSingleton::getInstance().startUDP();
	
	//NWrapperHost* wrapper = static_cast<NWrapperHost*>(NWrapperSingleton::getInstance().getNetworkWrapper());
	//wrapper->setLobbyName(NWrapperSingleton::getInstance().getMyPlayer().name.c_str());
}

LobbyHostState::~LobbyHostState() {
}

bool LobbyHostState::onEvent(const Event& event) {
	switch (event.type) {
	case Event::Type::TEXTINPUT:			onMyTextInput((const TextInputEvent&)event); break;
	case Event::Type::NETWORK_CHAT:			onRecievedText((const NetworkChatEvent&)event); break;
	case Event::Type::NETWORK_JOINED:		onPlayerJoined((const NetworkJoinedEvent&)event); break;
	case Event::Type::NETWORK_DISCONNECT:	onPlayerDisconnected((const NetworkDisconnectEvent&)event); break;
	case Event::Type::NETWORK_NAME:			onNameRequest((const NetworkNameEvent&)event); break;
	default: break;
	}

	return false;
}

bool LobbyHostState::onMyTextInput(const TextInputEvent& event) {
	// Add to current message, If 'enter' ...
	if (this->inputToChatLog(event.msg)) {
		// ... Add current message to chat log
		Message temp{ std::to_string(NWrapperSingleton::getInstance().getMyPlayer().id), m_currentmessage };
		this->addTextToChat(temp);

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

bool LobbyHostState::onRecievedText(const NetworkChatEvent& event) {
	// Add received text to chat log
	this->addTextToChat(event.chatMessage);

	return false;
}

bool LobbyHostState::onPlayerJoined(const NetworkJoinedEvent& event) {
	// Add player to player list
	NWrapperSingleton::getInstance().playerJoined(event.player);

	return true;
}

bool LobbyHostState::onPlayerDisconnected(const NetworkDisconnectEvent& event) {
	// Remove player from player list.
	unsigned char id = event.player_id;
	NWrapperSingleton::getInstance().playerLeft(id);

	
	return false;
}

bool LobbyHostState::onNameRequest(const NetworkNameEvent& event) {
	// Parse the message | ?12:DANIEL
	std::string message = event.repliedName;
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
	//NWrapperSingleton::getInstance().playerJoined(Player{
	//		id_int,
	//		message	// Which at this point is only the name
	//});
	NWrapperSingleton::getInstance().getPlayer(id_int)->name = message;

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
