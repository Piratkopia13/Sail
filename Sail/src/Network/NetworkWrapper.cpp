#include "pch.h"
#include "Network/NetworkModule.hpp"
#include "NetworkWrapper.h"
#include "Sail.h"

#include "../../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "../../SPLASH/src/game/events/NetworkDisconnectEvent.h"
#include "../../SPLASH/src/game/events/NetworkChatEvent.h"
#include "../../SPLASH/src/game/events/NetworkWelcomeEvent.h"
#include "../../SPLASH/src/game/events/NetworkNameEvent.h"
#include "../../SPLASH/SPLASH/src/game/states/LobbyState.h"

void NetworkWrapper::Initialize() {
	m_network = new Network();
}

NetworkWrapper::~NetworkWrapper() {
	m_network->shutdown();
	delete m_network;
}

bool NetworkWrapper::host(int port) {
	if (!m_network->isServer())
	{
		//return m_network->setupHost(port);
	}
	return m_network->setupHost(port);
}

bool NetworkWrapper::connectToIP(char* adress) {

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

	return m_network->setupClient(IP, port);
}

void NetworkWrapper::sendMsg(std::string msg) {
	m_network->send(msg.c_str(), msg.length());
}

void NetworkWrapper::sendChatMsg(std::string msg) {
	
	
	if (m_network->isServer())
	{
		msg = std::string("mHost: ") + msg;
		m_network->send(msg.c_str(), msg.length(), -1);
		msg.erase(0, 1);
		msg = msg + std::string("\n");
		printf(msg.c_str());
	}
	else
	{
		msg = std::string("m") + msg;
		m_network->send(msg.c_str(), msg.length());
	}
}

void NetworkWrapper::sendMsgAllClients(std::string msg) {
	m_network->send(msg.c_str(), msg.length(), -1);
}

void NetworkWrapper::sendChatAllClients(std::string msg) {
	msg = std::string("m") + msg;
	m_network->send(msg.c_str(), msg.length(), -1);
}

void NetworkWrapper::checkForPackages() {
	m_network->checkForPackages(*this);
}

bool NetworkWrapper::isInitialized() {
	return m_network->isInitialized();
}

bool NetworkWrapper::isHost() {
	return m_network->isServer();
}

void NetworkWrapper::decodeMessage(NetworkEvent nEvent) {

	// These will be assigned in the switch case.
	unsigned int userID;
	std::string message;
	char charAsInt[4] = { 0 };
	std::list<Player> playerList;	// Only used in 'w'-case but needs to be initialized up here
	Player currentPlayer{ -1, "" };	// 
	int charCounter = 0;			//
	string id_string;				//
	string remnants;				//
	unsigned int id_number;			//

	switch (nEvent.data->msg[0])
	{
	case 'm':
		// handle chat message.

		// The host sends this message to all clients.
		if (m_network->isServer())
		{
			std::string tempMessage = std::string(nEvent.data->msg);
			tempMessage.erase(0, 1);
			message = std::string("m")/* + std::to_string(nEvent.clientID) + 
				std::string(": ")*/ + tempMessage;

			sendMsgAllClients(message);

			// Print to screen
			message = message + std::string("\n");
			message.erase(0, 1);
			printf(message.c_str());
		}
		else
		{
			// Print to screen
			message = nEvent.data->msg + std::string("\n");
			message.erase(0, 1);
			printf(message.c_str());
		}



		Application::getInstance()->dispatchEvent(NetworkChatEvent(Message{
			to_string(nEvent.clientID),
			message
		}));

		break;

	case 'd':
		// Only clients will get this message. Host handles this in playerDisconnected()
		
		// Get the user ID from the event data.
		for (int i = 0; i < 4; i++) {
			charAsInt[i] = nEvent.data->msg[1 + i];
		}
		userID = reinterpret_cast<int&>(charAsInt);

		// TODO:
		// Remove the user with this ID from the lobby and print out that it disconnected.
		printf((std::to_string(userID) + " disconnected. \n").c_str());
		Application::getInstance()->dispatchEvent(NetworkDisconnectEvent(userID));
		break;

	case 'j':
		// Only clients will get this message. Host handles this in playerJoined()

		// Get the user ID from the event data.
		for (int i = 0; i < 4; i++) {
			charAsInt[i] = nEvent.data->msg[1 + i];
		}
		userID = reinterpret_cast<int&>(charAsInt);

		// TODO:
		// Add this user id to the list of players in the lobby.
		// Print out that this ID joined the lobby.
		printf((std::to_string(userID) + " joined. \n").c_str());
		Application::getInstance()->dispatchEvent(NetworkJoinedEvent(Player{ userID, "who?" }));
		break;

	case '?':

		// This client has recieved a request for its selected name
		if (!m_network->isServer()) {
			string ultratemp = nEvent.data->msg;
			ConnectionID id = parseID(ultratemp);
			Application::getInstance()->dispatchEvent(NetworkNameEvent(to_string(id)));
		}
		else {
			Application::getInstance()->dispatchEvent(NetworkNameEvent{nEvent.data->msg});
		}

	case 'w':

		// Parse the welcome-package.
		remnants = nEvent.data->msg;

		while (remnants != "") {
			currentPlayer.id = this->parseID(remnants);
			currentPlayer.name = this->parseName(remnants);

			// Only add players with full names.
			if (currentPlayer.name != "") { 
				playerList.push_back(currentPlayer);
			}
		}

		if (playerList.size() > 0) {
			Application::getInstance()->dispatchEvent(NetworkWelcomeEvent(playerList));
		}
		
		break;

	default:
		printf((std::string("Error: Packet message with key: ") + 
			nEvent.data->msg[0] + "can't be handled. \n").c_str());
		break;
	}

}

void NetworkWrapper::playerDisconnected(ConnectionID id) {
	/*
		Send disconnect message to all clients if host.
	*/
	if (m_network->isServer())
	{
		char msg[64];
		int intid = (int)id;
		char* int_asChar = reinterpret_cast<char*>(&intid);

		msg[0] = 'd';
		for (int i = 0; i < 4; i++) {
			msg[i + 1] = int_asChar[i];
		}

		// Send to all clients that soneone disconnected and which id.
		m_network->send(msg, sizeof(msg), -1);

		printf((std::to_string(intid) + " disconnected. \n").c_str());

		// Send id to menu / game state
		Application::getInstance()->dispatchEvent(NetworkDisconnectEvent(intid));
	}
	else
	{
		printf("Host disconnected. \n");
	}
}

void NetworkWrapper::playerReconnected(ConnectionID id) {
	/*
		This remains unimplemented.
	*/
}

void NetworkWrapper::playerJoined(ConnectionID id) {
	if (m_network->isServer())
	{
		char msg[64] = { 0 };
		unsigned int intid = id;
		char* int_asChar = reinterpret_cast<char*>(&intid);

		msg[0] = 'j';
		for (int i = 0; i < 4; i++) {
			msg[i + 1] = int_asChar[i];
		}

		// Request a name from the client, which upon recieval will be sent to all clients.
		char msgRequest[64];
		msgRequest[0] = '?';
		msgRequest[1] = *to_string(id).c_str();//*reinterpret_cast<char*>(&id);
		msgRequest[2] = '\0';

		m_network->send(msgRequest, sizeof(msgRequest), id);
		printf("Greeted ID: %d. Requesting Name... \n", id);

		// Send to all clients that someone joined and which id.
	//	m_network->send(msg, sizeof(msg), -1);

		// Add this user id to the list of players in the lobby.
		// Print out that this ID joined the lobby.
		printf((std::to_string(intid) + " joined. \n").c_str());

		Application::getInstance()->dispatchEvent(NetworkJoinedEvent(Player{ intid, "" }));
	}
}

void NetworkWrapper::handleNetworkEvents(NetworkEvent nEvent) {
	switch (nEvent.eventType)
	{
	case NETWORK_EVENT_TYPE::NETWORK_ERROR:
		break;
	case NETWORK_EVENT_TYPE::CLIENT_JOINED:
		playerJoined(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::CLIENT_DISCONNECTED:
		playerDisconnected(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::CLIENT_RECONNECTED:
		playerReconnected(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::MSG_RECEIVED:
		decodeMessage(nEvent);
		break;
	default:
		break;
	}
}

ConnectionID NetworkWrapper::parseID(std::string& data) {
	if (data.size() < 1) {
		return 0;
	}
	else {
		// Remove opening ':' / '?' marker.
		data.erase(0, 1);

		std::string id_string = "";
		int lastIndex;
		for (lastIndex = 0; lastIndex < MAX_PACKAGE_SIZE; lastIndex++) {
			if (data[lastIndex] == '\0') {
				break;
			}
			else if (data[lastIndex] != ':') {
				id_string += data[lastIndex];
			}
			else {
				break;
			}
		}

		data.erase(0, lastIndex);
		if (id_string != "") {
			return stoll(id_string);
		}
		else {
			return 0;
		}
		
	}
}

std::string NetworkWrapper::parseName(std::string& data) {
	if (data.size() < 1) {
		return data;
	}
	else {
		// Remove first ':' marker
		data.erase(0, 1);

		int lastIndex;
		std::string parsedName = "";
		for (lastIndex = 0; lastIndex < MAX_PACKAGE_SIZE; lastIndex++) {
			if (data[lastIndex] == ':') { // Does parseID also remove the last ':'? no?
				break;
			}
			else {
				parsedName += data[lastIndex];
			}
		}

		data.erase(0, lastIndex);
		return parsedName;
	}
}
