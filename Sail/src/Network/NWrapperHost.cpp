#include "pch.h"
#include "NWrapperHost.h"
#include "Network/NetworkModule.hpp"

#include "Sail/events/EventDispatcher.h"

#include "../../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "../../SPLASH/src/game/events/NetworkDisconnectEvent.h"
#include "../../SPLASH/src/game/events/NetworkChatEvent.h"
#include "../../SPLASH/src/game/events/NetworkWelcomeEvent.h"
#include "../../SPLASH/src/game/events/NetworkNameEvent.h"
#include "../../SPLASH/src/game/events/NetworkSerializedPackageEvent.h"
#include "../../SPLASH/src/game/states/LobbyState.h"

bool NWrapperHost::host(int port) {
	bool result = m_network->host(port);
	return result;
}

bool NWrapperHost::connectToIP(char*) {
	// Do nothing. clients connect to hosts, not the other way around.
	return false;
}

void NWrapperHost::setLobbyName(std::string name) {
	m_lobbyName = name;
	updateServerDescription();
}

void NWrapperHost::sendChatMsg(std::string msg) {
	msg = std::string("mHost: ") + msg;
	m_network->send(msg.c_str(), msg.length() + 1, -1);
	msg.erase(0, 1);
	msg = msg + std::string("\n");
}

void NWrapperHost::updateServerDescription() {
	m_serverDescription = m_lobbyName + " (" + std::to_string(NWrapperSingleton::getInstance().getPlayers().size()) + "/12)";
	m_network->setServerMetaDescription(m_serverDescription.c_str(), m_serverDescription.length() + 1);
}

void NWrapperHost::playerJoined(TCP_CONNECTION_ID id) {
	// Generate an ID for the client that joined and send that information.
	unsigned char test = m_IdDistribution;
	m_IdDistribution++;
	unsigned char newId = m_IdDistribution;
	m_connectionsMap.insert(std::pair<TCP_CONNECTION_ID, unsigned char>(id, newId));

	// Request a name from the client, which upon retrieval will be sent to all clients.
	char msgRequest[64];
	msgRequest[0] = '?';
	msgRequest[1] = newId;
	msgRequest[2] = '\0';

	unsigned char c = msgRequest[1];

	m_network->send(msgRequest, sizeof(msgRequest), id);

	EventDispatcher::Instance().emit(NetworkJoinedEvent(Player{ newId, "NoName" }));

	updateServerDescription();
}

void NWrapperHost::playerDisconnected(TCP_CONNECTION_ID id) {
	unsigned char convertedId = m_connectionsMap.at(id);
	char compressedMessage[64] = { 0 };

	this->compressDCMessage(convertedId, compressedMessage);

	// Send to all clients that someone disconnected and which id.
	m_network->send(compressedMessage, sizeof(compressedMessage), -1);

	// Send id to menu / game state
	EventDispatcher::Instance().emit(NetworkDisconnectEvent(convertedId));

	updateServerDescription();
}

void NWrapperHost::playerReconnected(TCP_CONNECTION_ID id) {
	// Nothing implemented so far.
}

void NWrapperHost::decodeMessage(NetworkEvent nEvent) {
	// These will be assigned in the switch case.
	std::string message;
	char charAsInt[4] = { 0 };
	std::list<Player> playerList;	// Only used in 'w'-case but needs to be initialized up here
	int charCounter = 0;			//
	std::string id_string = "";				//
	std::string remnants = "";				//
	unsigned int id_number = 0;			//
	std::string id = "";			// used in 'm'
	std::string remnants_m = "";
	Message processedMessage;
	std::string dataString;

	switch (nEvent.data->Message.rawMsg[0])
	{
	case 'm':
		// The host has received a message from a player...

		// Send out the already formatted message to clients so that they can process the message.
		sendMsgAllClients(nEvent.data->Message.rawMsg);	

		// Process the chat message
		processedMessage = processChatMessage((std::string)nEvent.data->Message.rawMsg);

		// Dispatch to lobby
		EventDispatcher::Instance().emit(NetworkChatEvent(processedMessage));

		break;

	case 'd':
		// Only clients will get this message. Host handles this in playerDisconnected()
		break;

	case 'j':
		// Only clients will get this message. Host handles this in playerJoined()
		break;

	case '?':
		// The host has received an answer to a name request...

		// TODO: move "Parse the ID and name from the message" from Host::onNameRequest to here
		EventDispatcher::Instance().emit(NetworkNameEvent{ nEvent.data->Message.rawMsg });

		break;

	case 'w':
		// Only clients receive welcome packages.
		break;
	case 's': // Serialized data, remove first character and send the rest to be deserialized
		dataString = std::string(nEvent.data->Message.rawMsg, nEvent.data->Message.sizeOfMsg);
		dataString.erase(0, 1); // remove the s

		// Send the serialized stringData as an event to the networkSystem which parses it.
		EventDispatcher::Instance().emit(NetworkSerializedPackageEvent(dataString));
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

