#include "pch.h"
#include "NWrapperHost.h"
#include "Network/NetworkModule.hpp"

#include "../../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "../../SPLASH/src/game/events/NetworkDisconnectEvent.h"
#include "../../SPLASH/src/game/events/NetworkChatEvent.h"
#include "../../SPLASH/src/game/events/NetworkWelcomeEvent.h"
#include "../../SPLASH/src/game/events/NetworkNameEvent.h"
#include "../../SPLASH/src/game/states/LobbyState.h"

bool NWrapperHost::host(int port) {
	return m_network->host(port);
}

bool NWrapperHost::connectToIP(char*) {
	// Do nothing. clients connect to hosts, not the other way around.
	return false;
}

void NWrapperHost::sendChatMsg(std::string msg) {
	msg = std::string("mHost: ") + msg;
	m_network->send(msg.c_str(), msg.length() + 1, -1);
	msg.erase(0, 1);
	msg = msg + std::string("\n");
}

void NWrapperHost::playerJoined(TCP_CONNECTION_ID id) {
	// Generate an ID for the client that joined and send that information.
	unsigned char test = m_IdDistribution;
	m_IdDistribution++;
	unsigned char newId = m_IdDistribution;
	m_connectionsMap.insert(std::pair<TCP_CONNECTION_ID, unsigned char>(id, newId));

	// Request a name from the client, which upon recieval will be sent to all clients.
	char msgRequest[64];
	msgRequest[0] = '?';
	msgRequest[1] = newId;
	msgRequest[2] = '\0';

	unsigned char c = msgRequest[1];

	m_network->send(msgRequest, sizeof(msgRequest), id);

	Application::getInstance()->dispatchEvent(NetworkJoinedEvent(Player{ newId, "" }));
}

void NWrapperHost::playerDisconnected(TCP_CONNECTION_ID id) {
	unsigned char convertedId = m_connectionsMap.at(id);
	char compressedMessage[64] = { 0 };

	this->compressDCMessage(convertedId, compressedMessage);

	// Send to all clients that soneone disconnected and which id.
	m_network->send(compressedMessage, sizeof(compressedMessage), -1);

	// Send id to menu / game state
	Application::getInstance()->dispatchEvent(NetworkDisconnectEvent(convertedId));
}

void NWrapperHost::playerReconnected(TCP_CONNECTION_ID id) {
	// Nothing implemented so far.
}

void NWrapperHost::decodeMessage(NetworkEvent nEvent) {
	// These will be assigned in the switch case.
	unsigned int userID;
	std::string message;
	char charAsInt[4] = { 0 };
	std::list<Player> playerList;	// Only used in 'w'-case but needs to be initialized up here
	Player currentPlayer{ -1, "" };	// 
	int charCounter = 0;			//
	std::string id_string = "";				//
	std::string remnants = "";				//
	unsigned int id_number = 0;			//
	std::string id = "";			// used in 'm'
	std::string remnants_m = "";
	unsigned int id_m;
	unsigned char id_question;
	Message processedMessage;

	switch (nEvent.data->rawMsg[0])
	{
	case 'm':
		// The host has recieved a message from a player...

		// Send out the already formatted message to clients so that they can process the message.
		sendMsgAllClients(nEvent.data->rawMsg);	

		// Process the chat message
		processedMessage = processChatMessage((std::string)nEvent.data->rawMsg);

		// Dispatch to lobby
		Application::getInstance()->dispatchEvent(NetworkChatEvent(processedMessage));

		break;

	case 'd':
		// Only clients will get this message. Host handles this in playerDisconnected()
		break;

	case 'j':
		// Only clients will get this message. Host handles this in playerJoined()
		break;

	case '?':
		// The host has recieved an answer to a name request...

		// TODO: move "Parse the ID and name from the message" from Host::onNameRequest to here
		Application::getInstance()->dispatchEvent(NetworkNameEvent{ nEvent.data->rawMsg });

		break;

	case 'w':
		// Only clients recieve welcome packages.
		break;

	default:
		break;
	}

	
}


void NWrapperHost::compressDCMessage(unsigned char& convertedId, char pDestination[64]) {
	char* int_asChar = reinterpret_cast<char*>(&convertedId);

	pDestination[0] = 'd';
	for (int i = 0; i < 4; i++) {
		pDestination[i + 1] = int_asChar[i];
	}
}

