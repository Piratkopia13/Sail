#include "pch.h"
#include "NWrapperClient.h"
#include "Network/NetworkModule.hpp"


#include "../../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "../../SPLASH/src/game/events/NetworkDisconnectEvent.h"
#include "../../SPLASH/src/game/events/NetworkChatEvent.h"
#include "../../SPLASH/src/game/events/NetworkWelcomeEvent.h"
#include "../../SPLASH/src/game/events/NetworkNameEvent.h"
#include "../../SPLASH/src/game/events/NetworkDroppedEvent.h"
#include "../../SPLASH/src/game/events/NetworkStartGameEvent.h"
#include "../../SPLASH/src/game/events/NetworkSerializedPackageEvent.h"
#include "../../SPLASH/src/game/states/LobbyState.h"
#include "../../SPLASH/src/game/events/NetworkBackToLobby.h"

bool NWrapperClient::host(int port) {
	// A client does not host, do nothing.
	return false;
}

bool NWrapperClient::connectToIP(char* adress) {
	bool adressIsIP = true;
	char IP[15] = { 0 };
	char portChar[5] = { 0 };
	int port = 0;

	int ipCounter = 0;
	int charCounter = 0;

	for (size_t i = 0; i < 22; i++)
	{
		if (adress[i] == ':')
		{
			i++;
			adressIsIP = false;
		}
		if (adressIsIP)
		{
			IP[ipCounter] = adress[i];
			ipCounter++;
		}
		else
		{
			if (charCounter >= sizeof(portChar))
			{
				continue;
			}
			portChar[charCounter] = adress[i];
			charCounter++;
		}
	}

	port = std::atoi(portChar);

	return m_network->join(IP, port);
}

void NWrapperClient::sendChatMsg(std::string msg) {
	msg = std::string("m") + msg;
	m_network->send(msg.c_str(), msg.length() + 1);
}

void NWrapperClient::playerJoined(TCP_CONNECTION_ID id) {
	// Clients never have others connecting to them.
}

void NWrapperClient::playerDisconnected(TCP_CONNECTION_ID id_) {
	// My connection to host was lost :(
	
	m_app->dispatchEvent(NetworkDroppedEvent());
}

void NWrapperClient::playerReconnected(TCP_CONNECTION_ID id) {
	// Connection with host re-established, not implemented yet.
}

void NWrapperClient::decodeMessage(NetworkEvent nEvent) {
	// These will be assigned in the switch case.
	unsigned char userID;
	char charAsInt[4] = { 0 };
	std::list<Player> playerList;	// Only used in 'w'-case but needs to be initialized up here
	Player currentPlayer{};	// 
	int charCounter = 0;			//
	std::string id_string = "";				//
	std::string remnants = "";				//
	unsigned int id_number = 0;			//
	std::string id = "";			// used in 'm'
	std::string remnants_m = "";
	unsigned char id_question;
	Message processedMessage;
	std::string dataString;

	switch (nEvent.data->Message.rawMsg[0]) {
	case 'm':
		// Client received a chat message from the host...
		// Parse and process into Message struct
		processedMessage = processChatMessage((std::string)nEvent.data->Message.rawMsg);

		// Dispatch Message to lobby.
		Application::getInstance()->dispatchEvent(NetworkChatEvent(processedMessage));

		break;

	case 'd':
		// A player disconnected from the host...
		// Get the user ID from the event data.
		userID = this->decompressDCMessage(std::string(nEvent.data->Message.rawMsg));

		// Dispatch the ID to lobby
		Application::getInstance()->dispatchEvent(NetworkDisconnectEvent(userID));
		break;

	case 'j':
		// A player joined the host...
		// Get the user ID from the event data.
		for (int i = 0; i < 4; i++) {
			charAsInt[i] = nEvent.data->Message.rawMsg[1 + i];
		}
		userID = reinterpret_cast<int&>(charAsInt);

		// Dispatch ID to lobby where the ID will be used to join the player
		Application::getInstance()->dispatchEvent(NetworkJoinedEvent(Player{ userID, "who?" }));
		break;

	case '?':
		// The host is giving us an ID and asking what our name is...
		// Parse the ID from the message
		id_question = nEvent.data->Message.rawMsg[1];

		// Dispatch ID to lobby where my chosen name will be replied back.
		Application::getInstance()->dispatchEvent(NetworkNameEvent(std::to_string(id_question)));
		break;

	case 't':
	{
		char seed = nEvent.data->Message.rawMsg[1];
		NWrapperSingleton::getInstance().setSeed(seed);
		Application::getInstance()->dispatchEvent(NetworkStartGameEvent());
	}
	break;
	case 'w':
		// The host has sent us a welcome-package with a list of the players in the game...
		// Parse the welcome-package into a list of players
		remnants = nEvent.data->Message.rawMsg;
		while (remnants != "") {
			currentPlayer.id = this->parseID(remnants);
			currentPlayer.name = this->parseName(remnants);

			// Only add players with full names.
			if (currentPlayer.name != "") {
				playerList.push_back(currentPlayer);
			}
		}

		// Dispatch the list up to the lobby.
		if (playerList.size() > 0) {
			Application::getInstance()->dispatchEvent(NetworkWelcomeEvent(playerList));
		}

		break;
	case 's': // Serialized data, remove first character and send the rest to be deserialized
		dataString = std::string(nEvent.data->Message.rawMsg, nEvent.data->Message.sizeOfMsg);
		dataString.erase(0, 1); // remove the s

		// Send the serialized stringData as an event to the networkSystem which parses it.
		Application::getInstance()->dispatchEvent(NetworkSerializedPackageEvent(dataString));
		break;

	case 'z':
		Application::getInstance()->dispatchEvent(NetworkBackToLobby());
		break;

	default:
		break;
	}
}

unsigned int NWrapperClient::decompressDCMessage(std::string messageData) {
	unsigned char userID;
	char charAsInt[4] = { 0 };

	for (int i = 0; i < 4; i++) {
		charAsInt[i] = messageData[1 + i];
	}
	userID = reinterpret_cast<int&>(charAsInt);

	return userID;
}

